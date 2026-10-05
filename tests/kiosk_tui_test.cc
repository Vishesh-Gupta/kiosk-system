#include "kiosk_tui.h"

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <memory>
#include <regex>
#include <string>

#include "kiosk_client.h"
#include "kiosk_service.h"
#include "test_support.h"

using ftxui::Event;

namespace {

// Renders the app to a fixed-size screen and returns the plain text.
std::string renderText(const ftxui::Component& component, int width = 140, int height = 40) {
  auto screen =
      ftxui::Screen::Create(ftxui::Dimension::Fixed(width), ftxui::Dimension::Fixed(height));
  ftxui::Render(screen, component->Render());
  static const std::regex kAnsi("\x1B\\[[0-9;?]*[A-Za-z]|\x1B\\][^\x07]*\x07");
  return std::regex_replace(screen.ToString(), kAnsi, "");
}

void typeText(const ftxui::Component& component, const std::string& text) {
  for (char c : text) {
    component->OnEvent(Event::Character(c));
  }
}

}  // namespace

TEST(DescribeStatusTest, ProducesFriendlyMessages) {
  EXPECT_EQ(describeStatus(grpc::Status(grpc::StatusCode::UNAVAILABLE, "failed to connect")),
            "Server unreachable");
  EXPECT_EQ(describeStatus(grpc::Status(grpc::StatusCode::UNAVAILABLE, "Database unavailable")),
            "Database unavailable");
  EXPECT_EQ(describeStatus(grpc::Status(grpc::StatusCode::NOT_FOUND, "Movie not found")),
            "Movie not found");
  EXPECT_EQ(describeStatus(grpc::Status(grpc::StatusCode::INTERNAL, "boom")), "Server error: boom");
}

TEST(KioskTuiOfflineTest, ShowsUnreachableServer) {
  auto client = KioskClient::Connect("127.0.0.1:1", std::chrono::milliseconds(500));
  InlineDispatcher dispatcher;
  KioskApp app(*client, dispatcher, [] {});
  app.start();

  EXPECT_TRUE(app.statusIsError());
  EXPECT_EQ(app.statusMessage(), "Could not load movies: Server unreachable");
  const std::string screen = renderText(app.component());
  EXPECT_NE(screen.find("Server unreachable"), std::string::npos) << screen;
}

// Drives the real component tree with key events against an in-process
// server backed by PostgreSQL.
class KioskTuiTest : public ::testing::Test {
 protected:
  void SetUp() override {
    KIOSK_SKIP_WITHOUT_DB(db);

    service = std::make_unique<KioskServiceImpl>(db);
    grpc::ServerBuilder builder;
    int port = 0;
    builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(service.get());
    server = builder.BuildAndStart();
    ASSERT_NE(server, nullptr);

    client = KioskClient::Connect("127.0.0.1:" + std::to_string(port));
    prefix = "tui-test-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-";
    app = std::make_unique<KioskApp>(*client, dispatcher, [this] { quit = true; });
    app->start();
  }

  void TearDown() override {
    if (!prefix.empty()) {
      db.exec("DELETE FROM movie WHERE name LIKE $1 || '%'", pqxx::params{prefix});
    }
    if (server) {
      server->Shutdown();
    }
  }

  ftxui::Component ui() {
    return app->component();
  }

  int32_t createMovie(const std::string& name, int32_t year = 2000) {
    kiosk::CreateMovieRequest request;
    request.set_name(prefix + name);
    request.set_release_year(year);
    request.set_duration(100);
    auto result = client->CreateMovie(request);
    EXPECT_TRUE(result.ok()) << result.status.error_message();
    return result.response.movie().id();
  }

  // Focuses the search box, filters to this test's movies and returns to the list.
  void searchOwnMovies() {
    ui()->OnEvent(Event::Character('/'));
    typeText(ui(), prefix);
    ui()->OnEvent(Event::Return);
  }

  bool hasMovie(const std::string& name) {
    auto result = client->ListMovies(prefix + name, 10, 0);
    return result.ok() && result.response.total_count() > 0;
  }

  DB db;
  std::unique_ptr<KioskServiceImpl> service;
  std::unique_ptr<grpc::Server> server;
  std::unique_ptr<KioskClient> client;
  InlineDispatcher dispatcher;
  std::unique_ptr<KioskApp> app;
  std::string prefix;
  bool quit = false;
};

TEST_F(KioskTuiTest, ListsAndFiltersMovies) {
  createMovie("Alpha", 1999);
  createMovie("Beta", 2004);
  searchOwnMovies();

  ASSERT_EQ(app->movies().size(), 2u);
  EXPECT_EQ(app->movies()[0].name(), prefix + "Beta");  // newest first

  const std::string screen = renderText(ui());
  EXPECT_NE(screen.find(prefix + "Alpha"), std::string::npos) << screen;
  EXPECT_NE(screen.find("2 movies"), std::string::npos) << screen;
  EXPECT_NE(screen.find("online"), std::string::npos) << screen;
}

TEST_F(KioskTuiTest, CreatesMovieThroughForm) {
  ui()->OnEvent(Event::Character('n'));
  ASSERT_TRUE(app->formOpen());

  typeText(ui(), prefix + "Gamma");
  ui()->OnEvent(Event::Tab);
  typeText(ui(), "1985");
  ui()->OnEvent(Event::Tab);
  typeText(ui(), "116");
  ui()->OnEvent(Event::Tab);
  typeText(ui(), "PG");
  ui()->OnEvent(Event::Tab);
  typeText(ui(), "Time travel in a DeLorean");
  ui()->OnEvent(Event::Return);

  EXPECT_FALSE(app->formOpen());
  EXPECT_FALSE(app->statusIsError());
  EXPECT_EQ(app->statusMessage(), "Created \"" + prefix + "Gamma\"");

  auto result = client->ListMovies(prefix + "Gamma", 10, 0);
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(result.response.movies_size(), 1);
  const auto& movie = result.response.movies(0);
  EXPECT_EQ(movie.release_year(), 1985);
  EXPECT_EQ(movie.duration(), 116);
  EXPECT_EQ(movie.rating(), "PG");
  EXPECT_EQ(movie.description(), "Time travel in a DeLorean");
}

TEST_F(KioskTuiTest, FormValidatesInput) {
  ui()->OnEvent(Event::Character('n'));
  ui()->OnEvent(Event::Return);
  EXPECT_TRUE(app->formOpen());
  EXPECT_NE(renderText(ui()).find("Title is required"), std::string::npos);

  typeText(ui(), prefix + "Delta");
  ui()->OnEvent(Event::Tab);
  typeText(ui(), "19x9");
  ui()->OnEvent(Event::Return);
  EXPECT_TRUE(app->formOpen());
  EXPECT_NE(renderText(ui()).find("Year must be a whole number"), std::string::npos);
  EXPECT_FALSE(hasMovie("Delta"));

  ui()->OnEvent(Event::Escape);
  EXPECT_FALSE(app->formOpen());
  EXPECT_FALSE(hasMovie("Delta"));
}

TEST_F(KioskTuiTest, EditsSelectedMovie) {
  createMovie("Epsilon", 1990);
  searchOwnMovies();
  ASSERT_EQ(app->movies().size(), 1u);

  ui()->OnEvent(Event::Character('e'));
  ASSERT_TRUE(app->formOpen());
  EXPECT_NE(renderText(ui()).find("Edit movie"), std::string::npos);

  ui()->OnEvent(Event::Tab);  // year field, prefilled with 1990
  ui()->OnEvent(Event::End);
  for (int i = 0; i < 4; ++i) {
    ui()->OnEvent(Event::Backspace);
  }
  typeText(ui(), "2001");
  ui()->OnEvent(Event::Return);

  EXPECT_FALSE(app->formOpen());
  EXPECT_EQ(app->statusMessage(), "Saved \"" + prefix + "Epsilon\"");
  ASSERT_EQ(app->movies().size(), 1u);
  EXPECT_EQ(app->movies()[0].release_year(), 2001);
  EXPECT_EQ(app->movies()[0].name(), prefix + "Epsilon");
}

TEST_F(KioskTuiTest, DeletesAfterConfirmation) {
  createMovie("Zeta");
  searchOwnMovies();
  ASSERT_EQ(app->movies().size(), 1u);

  ui()->OnEvent(Event::Character('d'));
  ASSERT_TRUE(app->confirmOpen());
  ui()->OnEvent(Event::Character('n'));  // cancel first
  EXPECT_FALSE(app->confirmOpen());
  EXPECT_TRUE(hasMovie("Zeta"));

  ui()->OnEvent(Event::Character('d'));
  ui()->OnEvent(Event::Character('y'));
  EXPECT_FALSE(app->confirmOpen());
  EXPECT_EQ(app->statusMessage(), "Deleted \"" + prefix + "Zeta\"");
  EXPECT_TRUE(app->movies().empty());
  EXPECT_FALSE(hasMovie("Zeta"));
  EXPECT_NE(renderText(ui()).find("No movies match"), std::string::npos);
}

TEST_F(KioskTuiTest, PagesThroughResults) {
  for (int i = 0; i < KioskApp::kPageSize + 5; ++i) {
    createMovie("Page " + std::to_string(i));
  }
  searchOwnMovies();
  EXPECT_EQ(app->movies().size(), static_cast<size_t>(KioskApp::kPageSize));
  EXPECT_NE(renderText(ui()).find("page 1/2"), std::string::npos);

  ui()->OnEvent(Event::Character(']'));
  EXPECT_EQ(app->page(), 1);
  EXPECT_EQ(app->movies().size(), 5u);

  ui()->OnEvent(Event::Character(']'));  // already on the last page
  EXPECT_EQ(app->page(), 1);

  ui()->OnEvent(Event::Character('['));
  EXPECT_EQ(app->page(), 0);
  EXPECT_EQ(app->movies().size(), static_cast<size_t>(KioskApp::kPageSize));
}

TEST_F(KioskTuiTest, QuitsOnQ) {
  ui()->OnEvent(Event::Character('q'));
  EXPECT_TRUE(quit);
}

TEST_F(KioskTuiTest, GoToIdShowsMovieFromGetMovie) {
  const int32_t id = createMovie("Eta", 1977);
  createMovie("Theta");  // newer, so Eta is not the first row of the full list

  ui()->OnEvent(Event::Character('g'));
  ASSERT_TRUE(app->gotoOpen());
  typeText(ui(), std::to_string(id));
  ui()->OnEvent(Event::Return);

  EXPECT_FALSE(app->gotoOpen());
  EXPECT_EQ(app->pinnedId(), id);
  ASSERT_EQ(app->movies().size(), 1u);
  EXPECT_EQ(app->movies()[0].name(), prefix + "Eta");
  EXPECT_NE(renderText(ui()).find("movie #" + std::to_string(id)), std::string::npos);

  // Editing keeps the pinned view and shows the updated movie.
  ui()->OnEvent(Event::Character('e'));
  ui()->OnEvent(Event::Tab);
  ui()->OnEvent(Event::End);
  for (int i = 0; i < 4; ++i) {
    ui()->OnEvent(Event::Backspace);
  }
  typeText(ui(), "1978");
  ui()->OnEvent(Event::Return);
  EXPECT_EQ(app->pinnedId(), id);
  ASSERT_EQ(app->movies().size(), 1u);
  EXPECT_EQ(app->movies()[0].release_year(), 1978);

  // Esc returns to the full list.
  ui()->OnEvent(Event::Escape);
  EXPECT_EQ(app->pinnedId(), 0);
  EXPECT_GT(app->movies().size(), 1u);
}

TEST_F(KioskTuiTest, GoToMissingIdReportsNotFound) {
  ui()->OnEvent(Event::Character('g'));
  typeText(ui(), "2147483647");
  ui()->OnEvent(Event::Return);

  EXPECT_EQ(app->pinnedId(), 0);
  EXPECT_TRUE(app->statusIsError());
  EXPECT_EQ(app->statusMessage(), "Movie #2147483647 not found");
}

TEST_F(KioskTuiTest, GoToRejectsInvalidId) {
  ui()->OnEvent(Event::Character('g'));
  typeText(ui(), "abc");
  ui()->OnEvent(Event::Return);
  EXPECT_TRUE(app->gotoOpen());
  EXPECT_NE(renderText(ui()).find("Enter a positive movie id"), std::string::npos);

  ui()->OnEvent(Event::Escape);
  EXPECT_FALSE(app->gotoOpen());
  EXPECT_EQ(app->pinnedId(), 0);
}

TEST_F(KioskTuiTest, DeletingPinnedMovieReturnsToList) {
  const int32_t id = createMovie("Iota");
  ui()->OnEvent(Event::Character('g'));
  typeText(ui(), std::to_string(id));
  ui()->OnEvent(Event::Return);
  ASSERT_EQ(app->pinnedId(), id);

  ui()->OnEvent(Event::Character('d'));
  ui()->OnEvent(Event::Character('y'));
  EXPECT_EQ(app->pinnedId(), 0);
  EXPECT_FALSE(app->statusIsError());
  EXPECT_EQ(app->statusMessage(), "Deleted \"" + prefix + "Iota\"");
  EXPECT_FALSE(hasMovie("Iota"));
}
