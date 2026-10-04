#include "kiosk_service.h"

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "test_support.h"

// Runs KioskServiceImpl on a local port and talks to it through a real gRPC
// stub, so these tests cover serialization, status codes and SQL together.
class KioskServiceTest : public ::testing::Test {
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
    ASSERT_GT(port, 0);

    stub = kiosk::Kiosk::NewStub(grpc::CreateChannel("127.0.0.1:" + std::to_string(port),
                                                     grpc::InsecureChannelCredentials()));
    prefix = "kiosk-test-" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-";
  }

  void TearDown() override {
    if (!prefix.empty()) {
      db.exec("DELETE FROM movie WHERE name LIKE $1 || '%'", pqxx::params{prefix});
    }
    if (server) {
      server->Shutdown();
    }
  }

  kiosk::Movie create(const std::string& name, int32_t year = 2000) {
    kiosk::CreateMovieRequest request;
    request.set_name(prefix + name);
    request.set_release_year(year);
    request.set_description("A test movie about " + name);
    request.set_duration(120);
    request.set_rating("PG");
    kiosk::CreateMovieResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub->CreateMovie(&context, request, &response);
    EXPECT_TRUE(status.ok()) << status.error_message();
    EXPECT_TRUE(response.success());
    return response.movie();
  }

  grpc::Status get(int32_t id, kiosk::GetMovieResponse* response) {
    kiosk::GetMovieRequest request;
    request.set_id(id);
    grpc::ClientContext context;
    return stub->GetMovie(&context, request, response);
  }

  DB db;
  std::unique_ptr<KioskServiceImpl> service;
  std::unique_ptr<grpc::Server> server;
  std::unique_ptr<kiosk::Kiosk::Stub> stub;
  std::string prefix;
};

TEST_F(KioskServiceTest, HealthCheckReportsHealthy) {
  kiosk::HealthCheckResponse response;
  grpc::ClientContext context;
  ASSERT_TRUE(stub->HealthCheck(&context, kiosk::HealthCheckRequest(), &response).ok());
  EXPECT_TRUE(response.healthy());
}

TEST_F(KioskServiceTest, CreateThenGet) {
  kiosk::Movie created = create("Alien", 1979);
  ASSERT_GT(created.id(), 0);
  EXPECT_EQ(created.name(), prefix + "Alien");
  EXPECT_EQ(created.release_year(), 1979);
  EXPECT_EQ(created.duration(), 120);
  EXPECT_EQ(created.rating(), "PG");
  EXPECT_FALSE(created.created_at().empty());
  EXPECT_EQ(created.created_at(), created.updated_at());

  kiosk::GetMovieResponse response;
  ASSERT_TRUE(get(created.id(), &response).ok());
  EXPECT_TRUE(response.success());
  EXPECT_EQ(response.movie().SerializeAsString(), created.SerializeAsString());
}

TEST_F(KioskServiceTest, CreateStoresHostileInputVerbatim) {
  kiosk::Movie created = create("O'Brien's \\ \"Movie\"; DROP TABLE movie; --");
  kiosk::GetMovieResponse response;
  ASSERT_TRUE(get(created.id(), &response).ok());
  EXPECT_EQ(response.movie().name(), prefix + "O'Brien's \\ \"Movie\"; DROP TABLE movie; --");
}

TEST_F(KioskServiceTest, CreateAllowsMissingOptionalFields) {
  kiosk::CreateMovieRequest request;
  request.set_name(prefix + "Untitled");
  request.set_release_year(2020);
  kiosk::CreateMovieResponse response;
  grpc::ClientContext context;
  ASSERT_TRUE(stub->CreateMovie(&context, request, &response).ok());
  EXPECT_EQ(response.movie().description(), "");
  EXPECT_EQ(response.movie().rating(), "");
}

TEST_F(KioskServiceTest, CreateRejectsInvalidInput) {
  kiosk::CreateMovieResponse response;
  {
    grpc::ClientContext context;
    EXPECT_EQ(stub->CreateMovie(&context, kiosk::CreateMovieRequest(), &response).error_code(),
              grpc::StatusCode::INVALID_ARGUMENT);
  }
  {
    kiosk::CreateMovieRequest request;
    request.set_name(prefix + "Negative");
    request.set_duration(-5);
    grpc::ClientContext context;
    EXPECT_EQ(stub->CreateMovie(&context, request, &response).error_code(),
              grpc::StatusCode::INVALID_ARGUMENT);
  }
}

TEST_F(KioskServiceTest, GetMissingMovieIsNotFound) {
  kiosk::GetMovieResponse response;
  EXPECT_EQ(get(2147483647, &response).error_code(), grpc::StatusCode::NOT_FOUND);
  EXPECT_EQ(get(0, &response).error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(KioskServiceTest, UpdateChangesOnlyProvidedFields) {
  kiosk::Movie created = create("Before", 1990);

  kiosk::UpdateMovieRequest request;
  request.set_id(created.id());
  request.set_name(prefix + "After");
  kiosk::UpdateMovieResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub->UpdateMovie(&context, request, &response);
  ASSERT_TRUE(status.ok()) << status.error_message();

  EXPECT_EQ(response.movie().name(), prefix + "After");
  EXPECT_EQ(response.movie().release_year(), 1990);
  EXPECT_EQ(response.movie().description(), created.description());
  EXPECT_EQ(response.movie().duration(), created.duration());
  EXPECT_EQ(response.movie().rating(), created.rating());
  EXPECT_EQ(response.movie().created_at(), created.created_at());
}

TEST_F(KioskServiceTest, UpdateRejectsEmptyAndMissing) {
  kiosk::Movie created = create("Unchanged");
  kiosk::UpdateMovieResponse response;
  {
    kiosk::UpdateMovieRequest request;
    request.set_id(created.id());
    grpc::ClientContext context;
    EXPECT_EQ(stub->UpdateMovie(&context, request, &response).error_code(),
              grpc::StatusCode::INVALID_ARGUMENT);
  }
  {
    kiosk::UpdateMovieRequest request;
    request.set_id(2147483647);
    request.set_name(prefix + "Ghost");
    grpc::ClientContext context;
    EXPECT_EQ(stub->UpdateMovie(&context, request, &response).error_code(),
              grpc::StatusCode::NOT_FOUND);
  }
}

TEST_F(KioskServiceTest, DeleteRemovesMovieOnce) {
  kiosk::Movie created = create("Doomed");

  kiosk::DeleteMovieRequest request;
  request.set_id(created.id());
  kiosk::DeleteMovieResponse response;
  {
    grpc::ClientContext context;
    ASSERT_TRUE(stub->DeleteMovie(&context, request, &response).ok());
    EXPECT_TRUE(response.success());
  }
  {
    grpc::ClientContext context;
    EXPECT_EQ(stub->DeleteMovie(&context, request, &response).error_code(),
              grpc::StatusCode::NOT_FOUND);
  }

  kiosk::GetMovieResponse getResponse;
  EXPECT_EQ(get(created.id(), &getResponse).error_code(), grpc::StatusCode::NOT_FOUND);
}

TEST_F(KioskServiceTest, ListSearchesAndPaginates) {
  std::vector<int32_t> ids;
  for (const char* name : {"Search A", "Search B", "Search C"}) {
    ids.push_back(create(name).id());
  }

  kiosk::ListMoviesRequest request;
  request.set_search_query(prefix + "Search");
  request.set_limit(2);
  kiosk::ListMoviesResponse response;
  {
    grpc::ClientContext context;
    ASSERT_TRUE(stub->ListMovies(&context, request, &response).ok());
  }
  EXPECT_EQ(response.total_count(), 3);
  ASSERT_EQ(response.movies_size(), 2);
  // Newest first.
  EXPECT_EQ(response.movies(0).id(), ids[2]);
  EXPECT_EQ(response.movies(1).id(), ids[1]);

  request.set_offset(2);
  response.Clear();
  {
    grpc::ClientContext context;
    ASSERT_TRUE(stub->ListMovies(&context, request, &response).ok());
  }
  EXPECT_EQ(response.total_count(), 3);
  ASSERT_EQ(response.movies_size(), 1);
  EXPECT_EQ(response.movies(0).id(), ids[0]);
}

TEST_F(KioskServiceTest, ListTreatsWildcardsLiterally) {
  create("100% Real");
  create("100X Real");

  kiosk::ListMoviesRequest request;
  request.set_search_query(prefix + "100%");
  kiosk::ListMoviesResponse response;
  grpc::ClientContext context;
  ASSERT_TRUE(stub->ListMovies(&context, request, &response).ok());
  ASSERT_EQ(response.movies_size(), 1);
  EXPECT_EQ(response.movies(0).name(), prefix + "100% Real");
}

TEST_F(KioskServiceTest, ListRejectsNegativePaging) {
  kiosk::ListMoviesRequest request;
  request.set_offset(-1);
  kiosk::ListMoviesResponse response;
  grpc::ClientContext context;
  EXPECT_EQ(stub->ListMovies(&context, request, &response).error_code(),
            grpc::StatusCode::INVALID_ARGUMENT);
}
