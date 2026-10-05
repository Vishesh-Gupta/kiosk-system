#ifndef KIOSK_TUI_H
#define KIOSK_TUI_H

#include <condition_variable>
#include <deque>
#include <ftxui/component/component.hpp>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "kiosk_client.h"

// Decides where KioskApp work runs. RPCs go to background(); their results
// come back through ui(), which must run on the thread that owns the UI.
class Dispatcher {
 public:
  virtual ~Dispatcher() = default;
  virtual void background(std::function<void()> task) = 0;
  virtual void ui(std::function<void()> task) = 0;
};

// Runs everything immediately on the calling thread (used by tests).
class InlineDispatcher : public Dispatcher {
 public:
  void background(std::function<void()> task) override {
    task();
  }
  void ui(std::function<void()> task) override {
    task();
  }
};

// Runs background tasks in order on one worker thread and hands results to
// the UI thread through `post`.
class WorkerDispatcher : public Dispatcher {
 public:
  explicit WorkerDispatcher(std::function<void(std::function<void()>)> post);
  ~WorkerDispatcher() override;

  WorkerDispatcher(const WorkerDispatcher&) = delete;
  WorkerDispatcher& operator=(const WorkerDispatcher&) = delete;

  void background(std::function<void()> task) override;
  void ui(std::function<void()> task) override;

  // Finishes the running task, drops queued ones and joins the worker. Call
  // before destroying anything the queued tasks reference.
  void stop();

 private:
  void run();

  std::function<void(std::function<void()>)> post_;
  std::mutex mutex_;
  std::condition_variable wake_;
  std::deque<std::function<void()>> queue_;
  bool stopping_ = false;
  std::thread worker_;
};

// Interactive terminal UI for browsing and editing movies.
//
// All state is owned by the UI thread: RPCs are dispatched to the background
// and their results are applied in ui() callbacks.
class KioskApp {
 public:
  static constexpr int kPageSize = 20;

  KioskApp(const KioskClient& client, Dispatcher& dispatcher, std::function<void()> onQuit);

  ftxui::Component component() const {
    return root_;
  }

  // Loads the first page and checks server health.
  void start();
  void refreshHealth();

  // Read-only views of the state, for tests.
  const std::vector<kiosk::Movie>& movies() const {
    return movies_;
  }
  const std::string& statusMessage() const {
    return status_;
  }
  bool statusIsError() const {
    return statusIsError_;
  }
  bool formOpen() const {
    return formOpen_;
  }
  bool confirmOpen() const {
    return confirmOpen_;
  }
  int page() const {
    return page_;
  }

 private:
  enum class FormMode { Create, Edit };

  void reload(int32_t selectId = 0);
  void openCreateForm();
  void openEditForm();
  void submitForm();
  void closeForm();
  void openDeleteConfirm();
  void confirmDelete();
  void changePage(int delta);
  void setStatus(std::string message, bool isError);
  const kiosk::Movie* selectedMovie() const;
  int pageCount() const;

  bool handleEvent(const ftxui::Event& event);

  ftxui::Element renderMain();
  ftxui::Element renderRow(const ftxui::EntryState& state) const;
  ftxui::Element renderDetails() const;
  ftxui::Element renderForm();
  ftxui::Element renderConfirm();

  const KioskClient& client_;
  Dispatcher& dispatcher_;
  std::function<void()> onQuit_;

  // List state.
  std::vector<kiosk::Movie> movies_;
  std::vector<std::string> entries_;  // one label per movie, backs the Menu
  int selected_ = 0;
  int32_t totalCount_ = 0;
  int page_ = 0;
  std::string query_;
  int listGeneration_ = 0;
  int pendingRequests_ = 0;

  // Server state.
  bool healthKnown_ = false;
  bool healthy_ = false;
  std::string healthMessage_;

  std::string status_;
  bool statusIsError_ = false;

  // Create/edit form.
  bool formOpen_ = false;
  FormMode formMode_ = FormMode::Create;
  int32_t formId_ = 0;
  std::string formName_;
  std::string formYear_;
  std::string formDuration_;
  std::string formRating_;
  std::string formDescription_;
  std::string formError_;
  bool formSubmitting_ = false;

  // Delete confirmation.
  bool confirmOpen_ = false;
  int32_t confirmId_ = 0;
  std::string confirmName_;

  ftxui::Component searchInput_;
  ftxui::Component list_;
  std::vector<ftxui::Component> formInputs_;  // name, year, duration, rating, description
  ftxui::Component formButtons_;
  ftxui::Component confirmButtons_;
  ftxui::Component root_;
};

#endif  // KIOSK_TUI_H
