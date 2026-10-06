#include "kiosk_tui.h"

#include <algorithm>
#include <charconv>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <utility>

using namespace ftxui;

// --- WorkerDispatcher --------------------------------------------------------

WorkerDispatcher::WorkerDispatcher(std::function<void(std::function<void()>)> post)
    : post_(std::move(post)), worker_([this] { run(); }) {}

WorkerDispatcher::~WorkerDispatcher() {
  stop();
}

void WorkerDispatcher::background(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopping_) {
      return;
    }
    queue_.push_back(std::move(task));
  }
  wake_.notify_one();
}

void WorkerDispatcher::ui(std::function<void()> task) {
  post_(std::move(task));
}

void WorkerDispatcher::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopping_ = true;
    queue_.clear();
  }
  wake_.notify_one();
  if (worker_.joinable()) {
    worker_.join();
  }
}

void WorkerDispatcher::run() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      wake_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
      if (stopping_) {
        return;
      }
      task = std::move(queue_.front());
      queue_.pop_front();
    }
    task();
  }
}

// --- Helpers -----------------------------------------------------------------

namespace {

const Color kAccent = Color::CyanLight;

// Parses an optional non-negative number; empty input means 0.
bool parseNumber(const std::string& input, int32_t& out) {
  if (input.empty()) {
    out = 0;
    return true;
  }
  const char* end = input.data() + input.size();
  auto [ptr, ec] = std::from_chars(input.data(), end, out);
  return ec == std::errc() && ptr == end && out >= 0;
}

std::string numberOrEmpty(int32_t value) {
  return value > 0 ? std::to_string(value) : "";
}

// Surrounds a modal with a cleared one-cell margin so its border does not
// join up with the box-drawing characters underneath it.
Element modalFrame(Element content) {
  return vbox({text(""), hbox({text(" "), std::move(content), text(" ")}), text("")}) | clear_under;
}

Element cell(const std::string& value, int width) {
  return text(value) | size(WIDTH, EQUAL, width);
}

Element keyHint(const std::string& key, const std::string& action) {
  return hbox({text(" " + key + " ") | inverted, text(" " + action + "  ") | dim});
}

InputOption singleLine(std::function<void()> onEnter) {
  InputOption option = InputOption::Default();
  option.multiline = false;
  option.on_enter = std::move(onEnter);
  return option;
}

}  // namespace

// --- KioskApp ----------------------------------------------------------------

KioskApp::KioskApp(const KioskClient& client, Dispatcher& dispatcher, std::function<void()> onQuit)
    : client_(client), dispatcher_(dispatcher), onQuit_(std::move(onQuit)) {
  InputOption searchOption = singleLine([this] { list_->TakeFocus(); });
  searchOption.on_change = [this] {
    pinnedId_ = 0;
    page_ = 0;
    reload();
  };
  searchInput_ = Input(&query_, "type to filter by title or description", searchOption);

  MenuOption listOption = MenuOption::Vertical();
  listOption.entries_option.transform = [this](const EntryState& state) {
    return renderRow(state);
  };
  listOption.on_enter = [this] { openEditForm(); };
  list_ = Menu(&entries_, &selected_, listOption);

  auto main = Renderer(Container::Vertical({searchInput_, list_}), [this] { return renderMain(); });

  auto submit = [this] { submitForm(); };
  formInputs_ = {
      Input(&formName_, "required", singleLine(submit)),
      Input(&formYear_, "e.g. 1999", singleLine(submit)),
      Input(&formDuration_, "minutes", singleLine(submit)),
      Input(&formRating_, "e.g. PG-13", singleLine(submit)),
      Input(&formDescription_, "short synopsis", singleLine(submit)),
  };
  formButtons_ = Container::Horizontal({
      Button(" Save ", submit, ButtonOption::Ascii()),
      Button(
          " Cancel ", [this] { closeForm(); }, ButtonOption::Ascii()),
  });
  Components formChildren = formInputs_;
  formChildren.push_back(formButtons_);
  auto form = Renderer(Container::Vertical(formChildren), [this] { return renderForm(); });

  confirmButtons_ = Container::Horizontal({
      Button(
          " Delete ", [this] { confirmDelete(); }, ButtonOption::Ascii()),
      Button(
          " Cancel ", [this] { confirmOpen_ = false; }, ButtonOption::Ascii()),
  });
  auto confirm = Renderer(confirmButtons_, [this] { return renderConfirm(); });

  gotoInputComponent_ = Input(&gotoInput_, "movie id", singleLine([this] { submitGoto(); }));
  auto gotoPrompt = Renderer(gotoInputComponent_, [this] { return renderGoto(); });

  root_ = main | Modal(form, &formOpen_) | Modal(confirm, &confirmOpen_) |
          Modal(gotoPrompt, &gotoOpen_) |
          CatchEvent([this](const Event& event) { return handleEvent(event); });
  list_->TakeFocus();
}

void KioskApp::start() {
  refreshHealth();
  reload();
}

void KioskApp::refreshHealth() {
  dispatcher_.background([this] {
    auto result = client_.HealthCheck();
    dispatcher_.ui([this, result] {
      healthKnown_ = true;
      if (!result.ok()) {
        healthy_ = false;
        healthMessage_ = describeStatus(result.status);
      } else {
        healthy_ = result.response.healthy();
        healthMessage_ = result.response.message();
      }
    });
  });
}

void KioskApp::reload(int32_t selectId) {
  const int generation = ++listGeneration_;
  ++pendingRequests_;
  if (pinnedId_ > 0) {
    loadPinned(generation);
    return;
  }

  const std::string query = query_;
  const int32_t offset = page_ * kPageSize;
  dispatcher_.background([this, generation, query, offset, selectId] {
    auto result = client_.ListMovies(query, kPageSize, offset);
    dispatcher_.ui([this, generation, selectId, result] {
      --pendingRequests_;
      if (generation != listGeneration_) {
        return;  // a newer request superseded this one
      }
      if (!result.ok()) {
        handleLoadError(result.status);
        return;
      }

      const auto& response = result.response;
      totalCount_ = response.total_count();
      if (response.movies().empty() && page_ > 0 && totalCount_ > 0) {
        // The page we were on no longer exists (e.g. after a delete).
        page_ = pageCount() - 1;
        reload(selectId);
        return;
      }
      applyMovies({response.movies().begin(), response.movies().end()}, totalCount_, selectId);
    });
  });
}

void KioskApp::loadPinned(int generation) {
  const int32_t id = pinnedId_;
  dispatcher_.background([this, generation, id] {
    auto result = client_.GetMovie(id);
    dispatcher_.ui([this, generation, id, result] {
      --pendingRequests_;
      if (generation != listGeneration_) {
        return;
      }
      if (result.status.error_code() == grpc::StatusCode::NOT_FOUND) {
        setStatus("Movie #" + std::to_string(id) + " not found", true);
        pinnedId_ = 0;
        reload();
        return;
      }
      if (!result.ok()) {
        handleLoadError(result.status);
        return;
      }
      applyMovies({result.response.movie()}, 1, id);
    });
  });
}

void KioskApp::applyMovies(std::vector<kiosk::Movie> movies, int32_t totalCount, int32_t selectId) {
  movies_ = std::move(movies);
  totalCount_ = totalCount;
  entries_.clear();
  for (const auto& movie : movies_) {
    entries_.push_back(movie.name());
  }
  if (selectId > 0) {
    auto it = std::find_if(movies_.begin(), movies_.end(),
                           [selectId](const kiosk::Movie& m) { return m.id() == selectId; });
    if (it != movies_.end()) {
      selected_ = static_cast<int>(it - movies_.begin());
    }
  }
  selected_ = std::clamp(selected_, 0, std::max(0, static_cast<int>(movies_.size()) - 1));
  if (statusIsError_ && status_.rfind("Could not load", 0) == 0) {
    setStatus("", false);
  }
}

void KioskApp::handleLoadError(const grpc::Status& status) {
  setStatus("Could not load movies: " + describeStatus(status), true);
  if (status.error_code() == grpc::StatusCode::UNAVAILABLE ||
      status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
    healthKnown_ = true;
    healthy_ = false;
    healthMessage_ = describeStatus(status);
  }
}

void KioskApp::openGoto() {
  gotoInput_.clear();
  gotoError_.clear();
  gotoOpen_ = true;
  gotoInputComponent_->TakeFocus();
}

void KioskApp::submitGoto() {
  int32_t id = 0;
  if (!parseNumber(gotoInput_, id) || id <= 0) {
    gotoError_ = "Enter a positive movie id";
    return;
  }
  gotoOpen_ = false;
  list_->TakeFocus();
  pinnedId_ = id;
  query_.clear();
  page_ = 0;
  selected_ = 0;
  reload();
}

void KioskApp::unpin() {
  pinnedId_ = 0;
  page_ = 0;
  reload();
}

void KioskApp::changePage(int delta) {
  const int target = std::clamp(page_ + delta, 0, std::max(0, pageCount() - 1));
  if (target != page_) {
    page_ = target;
    selected_ = 0;
    reload();
  }
}

int KioskApp::pageCount() const {
  return std::max(1, (totalCount_ + kPageSize - 1) / kPageSize);
}

const kiosk::Movie* KioskApp::selectedMovie() const {
  if (selected_ < 0 || selected_ >= static_cast<int>(movies_.size())) {
    return nullptr;
  }
  return &movies_[selected_];
}

void KioskApp::setStatus(std::string message, bool isError) {
  status_ = std::move(message);
  statusIsError_ = isError;
}

void KioskApp::openCreateForm() {
  formMode_ = FormMode::Create;
  formId_ = 0;
  formName_.clear();
  formYear_.clear();
  formDuration_.clear();
  formRating_.clear();
  formDescription_.clear();
  formError_.clear();
  formOpen_ = true;
  formInputs_[0]->TakeFocus();
}

void KioskApp::openEditForm() {
  const kiosk::Movie* movie = selectedMovie();
  if (movie == nullptr) {
    return;
  }
  formMode_ = FormMode::Edit;
  formId_ = movie->id();
  formName_ = movie->name();
  formYear_ = numberOrEmpty(movie->release_year());
  formDuration_ = numberOrEmpty(movie->duration());
  formRating_ = movie->rating();
  formDescription_ = movie->description();
  formError_.clear();
  formOpen_ = true;
  formInputs_[0]->TakeFocus();
}

void KioskApp::closeForm() {
  formOpen_ = false;
  formSubmitting_ = false;
  list_->TakeFocus();
}

void KioskApp::submitForm() {
  if (formSubmitting_) {
    return;
  }
  int32_t year = 0;
  int32_t duration = 0;
  if (formName_.empty()) {
    formError_ = "Title is required";
    return;
  }
  if (!parseNumber(formYear_, year)) {
    formError_ = "Year must be a whole number";
    return;
  }
  if (!parseNumber(formDuration_, duration)) {
    formError_ = "Duration must be a whole number of minutes";
    return;
  }

  formError_.clear();
  formSubmitting_ = true;
  const bool creating = formMode_ == FormMode::Create;
  const std::string name = formName_;

  if (creating) {
    kiosk::CreateMovieRequest request;
    request.set_name(formName_);
    request.set_release_year(year);
    request.set_duration(duration);
    request.set_rating(formRating_);
    request.set_description(formDescription_);
    dispatcher_.background([this, request, name] {
      auto result = client_.CreateMovie(request);
      dispatcher_.ui([this, result, name] {
        formSubmitting_ = false;
        if (!result.ok()) {
          formError_ = describeStatus(result.status);
          return;
        }
        closeForm();
        setStatus("Created \"" + name + "\"", false);
        page_ = 0;
        reload(result.response.movie().id());
      });
    });
  } else {
    kiosk::UpdateMovieRequest request;
    request.set_id(formId_);
    request.set_name(formName_);
    request.set_release_year(year);
    request.set_duration(duration);
    request.set_rating(formRating_);
    request.set_description(formDescription_);
    dispatcher_.background([this, request, name] {
      auto result = client_.UpdateMovie(request);
      dispatcher_.ui([this, result, name] {
        formSubmitting_ = false;
        if (!result.ok()) {
          formError_ = describeStatus(result.status);
          return;
        }
        closeForm();
        setStatus("Saved \"" + name + "\"", false);
        reload(result.response.movie().id());
      });
    });
  }
}

void KioskApp::openDeleteConfirm() {
  const kiosk::Movie* movie = selectedMovie();
  if (movie == nullptr) {
    return;
  }
  confirmId_ = movie->id();
  confirmName_ = movie->name();
  confirmOpen_ = true;
  confirmButtons_->TakeFocus();
}

void KioskApp::confirmDelete() {
  confirmOpen_ = false;
  list_->TakeFocus();
  const int32_t id = confirmId_;
  const std::string name = confirmName_;
  dispatcher_.background([this, id, name] {
    auto result = client_.DeleteMovie(id);
    dispatcher_.ui([this, result, id, name] {
      if (!result.ok()) {
        setStatus("Could not delete \"" + name + "\": " + describeStatus(result.status), true);
      } else {
        setStatus("Deleted \"" + name + "\"", false);
        if (pinnedId_ == id) {
          pinnedId_ = 0;  // nothing left to show; go back to the full list
        }
      }
      reload();
    });
  });
}

bool KioskApp::handleEvent(const Event& event) {
  if (formOpen_) {
    if (event == Event::Escape) {
      closeForm();
      return true;
    }
    return false;
  }

  if (confirmOpen_) {
    if (event == Event::Character('y') || event == Event::Character('Y')) {
      confirmDelete();
      return true;
    }
    if (event == Event::Escape || event == Event::Character('n') ||
        event == Event::Character('N')) {
      confirmOpen_ = false;
      list_->TakeFocus();
      return true;
    }
    return false;
  }

  if (gotoOpen_) {
    if (event == Event::Escape) {
      gotoOpen_ = false;
      list_->TakeFocus();
      return true;
    }
    return false;
  }

  if (searchInput_->Focused()) {
    if (event == Event::Escape || event == Event::ArrowDown || event == Event::Return) {
      list_->TakeFocus();
      return true;
    }
    return false;
  }

  if (event == Event::Character('q')) {
    onQuit_();
    return true;
  }
  if (event == Event::Character('/')) {
    searchInput_->TakeFocus();
    return true;
  }
  if (event == Event::Character('n')) {
    openCreateForm();
    return true;
  }
  if (event == Event::Character('g')) {
    openGoto();
    return true;
  }
  if (event == Event::Character('e') || event == Event::Return) {
    openEditForm();
    return true;
  }
  if (event == Event::Character('d') || event == Event::Delete) {
    openDeleteConfirm();
    return true;
  }
  if (event == Event::Character('r')) {
    const kiosk::Movie* movie = selectedMovie();
    reload(movie != nullptr ? movie->id() : 0);
    refreshHealth();
    return true;
  }
  if (event == Event::PageDown || event == Event::Character(']')) {
    changePage(1);
    return true;
  }
  if (event == Event::PageUp || event == Event::Character('[')) {
    changePage(-1);
    return true;
  }
  if (event == Event::Escape && pinnedId_ > 0) {
    unpin();
    return true;
  }
  if (event == Event::Escape && !query_.empty()) {
    query_.clear();
    page_ = 0;
    reload();
    return true;
  }
  return false;
}

// --- Rendering ---------------------------------------------------------------

Element KioskApp::renderRow(const EntryState& state) const {
  const kiosk::Movie& movie = movies_[state.index];
  Element row = hbox({
      cell(" " + std::to_string(movie.id()), 7),
      text(movie.name()) | flex,
      cell(numberOrEmpty(movie.release_year()), 6),
      cell(movie.duration() > 0 ? std::to_string(movie.duration()) + "m" : "", 6),
      cell(movie.rating(), 7),
  });
  if (state.focused) {
    row = row | inverted;
  } else if (state.active) {
    row = row | bold | color(kAccent);
  }
  return row;
}

Element KioskApp::renderDetails() const {
  const kiosk::Movie* movie = selectedMovie();
  if (movie == nullptr) {
    return text("No movie selected") | dim | center;
  }

  std::vector<std::string> facts;
  if (movie->release_year() > 0) {
    facts.push_back(std::to_string(movie->release_year()));
  }
  if (movie->duration() > 0) {
    facts.push_back(std::to_string(movie->duration()) + " min");
  }
  if (!movie->rating().empty()) {
    facts.push_back("Rated " + movie->rating());
  }
  std::string factLine;
  for (size_t i = 0; i < facts.size(); ++i) {
    factLine += (i > 0 ? "  ·  " : "") + facts[i];
  }

  return vbox({
             paragraph(movie->name()) | bold | color(kAccent),
             text(factLine) | dim,
             separator(),
             movie->description().empty() ? text("No description") | dim
                                          : paragraph(movie->description()),
             filler(),
             separator(),
             text("ID       " + std::to_string(movie->id())) | dim,
             text("Created  " + movie->created_at()) | dim,
             text("Updated  " + movie->updated_at()) | dim,
         }) |
         flex | xflex_shrink | borderEmpty;
}

Element KioskApp::renderMain() {
  Element health;
  if (!healthKnown_) {
    health = text("○ connecting…") | dim;
  } else if (healthy_) {
    health = text("● online") | color(Color::GreenLight);
  } else {
    health = text("● " + healthMessage_) | color(Color::RedLight);
  }

  std::string countText = std::to_string(totalCount_) + (totalCount_ == 1 ? " movie" : " movies");
  if (pinnedId_ > 0) {
    countText = "movie #" + std::to_string(pinnedId_) + "  ·  Esc shows all";
  } else if (totalCount_ > kPageSize) {
    countText += "  ·  page " + std::to_string(page_ + 1) + "/" + std::to_string(pageCount());
  }

  Element listBody;
  if (movies_.empty()) {
    std::string message = pendingRequests_ > 0 ? "Loading…"
                          : query_.empty()     ? "No movies yet. Press n to add one."
                                               : "No movies match \"" + query_ + "\"";
    listBody = text(message) | dim | center | flex;
  } else {
    listBody = list_->Render() | vscroll_indicator | frame | flex;
  }

  const Element tableHeader = hbox({
                                  cell(" ID", 7),
                                  text("Title") | flex,
                                  cell("Year", 6),
                                  cell("Len", 6),
                                  cell("Rating", 7),
                              }) |
                              bold | dim;

  Element status =
      status_.empty()
          ? text(" ")
          : text(" " + status_) | color(statusIsError_ ? Color::RedLight : Color::GreenLight);

  Element hints;
  if (searchInput_->Focused()) {
    hints = hbox({keyHint("Enter", "done"), keyHint("Esc", "back to list")});
  } else {
    hints = hbox({
        keyHint("↑↓", "select"),
        keyHint("/", "search"),
        keyHint("g", "go to id"),
        keyHint("n", "new"),
        keyHint("e", "edit"),
        keyHint("d", "delete"),
        keyHint("[ ]", "page"),
        keyHint("r", "refresh"),
        keyHint("q", "quit"),
    });
  }

  Element search = searchInput_->Render() | flex;
  if (searchInput_->Focused()) {
    search = search | color(kAccent);
  }

  return vbox({
             hbox({
                 text(" KIOSK ") | bold | inverted | color(kAccent),
                 text("  Movies  ") | bold,
                 text(client_.target()) | dim,
                 filler(),
                 pendingRequests_ > 0 ? text("loading…  ") | dim : text(""),
                 health,
                 text(" "),
             }),
             separator(),
             hbox({text(" Search ") | bold, search, text("  " + countText + " ") | dim}),
             separator(),
             hbox({
                 vbox({tableHeader, separator(), listBody}) | flex,
                 separator(),
                 renderDetails() | size(WIDTH, EQUAL, 44),
             }) | flex,
             separator(),
             status,
             hints,
         }) |
         borderRounded;
}

Element KioskApp::renderForm() {
  static const char* const kLabels[] = {"Title", "Year", "Duration", "Rating", "Description"};
  Elements rows;
  for (size_t i = 0; i < formInputs_.size(); ++i) {
    Element field = formInputs_[i]->Render() | size(WIDTH, GREATER_THAN, 40);
    if (formInputs_[i]->Focused()) {
      field = field | color(kAccent);
    }
    rows.push_back(hbox({text(kLabels[i]) | size(WIDTH, EQUAL, 13) | bold, field}));
  }

  const bool creating = formMode_ == FormMode::Create;
  return window(text(creating ? " New movie " : " Edit movie ") | bold,
                vbox({
                    vbox(std::move(rows)),
                    separator(),
                    formError_.empty() ? text(creating ? "" : "Empty fields keep their value") | dim
                                       : text(formError_) | color(Color::RedLight),
                    hbox({filler(), formSubmitting_ ? text("saving… ") | dim : text(""),
                          formButtons_->Render()}),
                    text("Tab next field · Enter save · Esc cancel") | dim | center,
                }) | size(WIDTH, GREATER_THAN, 60)) |
         modalFrame;
}

Element KioskApp::renderGoto() {
  return window(
             text(" Go to movie ") | bold,
             vbox({
                 hbox({text("ID  ") | bold, gotoInputComponent_->Render() | color(kAccent) | flex}),
                 gotoError_.empty() ? text("") : text(gotoError_) | color(Color::RedLight),
                 text("Enter open · Esc cancel") | dim | center,
             }) | size(WIDTH, GREATER_THAN, 32)) |
         modalFrame;
}

Element KioskApp::renderConfirm() {
  return window(text(" Delete movie ") | bold | color(Color::RedLight),
                vbox({
                    text("Delete \"" + confirmName_ + "\"?"),
                    text("This cannot be undone.") | dim,
                    separator(),
                    hbox({filler(), confirmButtons_->Render()}),
                    text("y delete · n / Esc cancel") | dim | center,
                }) | size(WIDTH, GREATER_THAN, 40)) |
         modalFrame;
}
