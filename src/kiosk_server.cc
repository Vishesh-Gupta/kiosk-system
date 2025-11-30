#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include "kiosk/kiosk.grpc.pb.h"
#include "db.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using kiosk::Kiosk;
using kiosk::GetMovieRequest;
using kiosk::GetMovieResponse;
using kiosk::ListMoviesRequest;
using kiosk::ListMoviesResponse;
using kiosk::CreateMovieRequest;
using kiosk::CreateMovieResponse;
using kiosk::UpdateMovieRequest;
using kiosk::UpdateMovieResponse;
using kiosk::DeleteMovieRequest;
using kiosk::DeleteMovieResponse;
using kiosk::Movie;
using kiosk::HealthCheckRequest;
using kiosk::HealthCheckResponse;

// Helper function to format timestamp
static std::string currentTimestamp() {
  auto now = std::time(nullptr);
  std::stringstream sstream;
  sstream << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
  return sstream.str();
}

// Helper function to convert pqxx result to Movie
Movie resultToMovie(const pqxx::row& row) {
  Movie movie;
  movie.set_id(row[0].as<int>());
  movie.set_name(row[1].as<std::string>());
  movie.set_release_year(row[2].as<int>());
  movie.set_description(row[3].as<std::string>());
  movie.set_duration(row[4].as<int>());
  movie.set_rating(row[5].as<std::string>());
  movie.set_created_at(row[6].as<std::string>());
  movie.set_updated_at(row[7].as<std::string>());
  return movie;
}

class KioskImpl final : public Kiosk::Service {
 private:
  DB db;

 public:
  KioskImpl() {
    try {
      db.connect();
    } catch (const std::exception& e) {
      std::cerr << "Failed to connect to database: " << e.what() << "\n";
    }
  }

  Status GetMovie(ServerContext* context, const GetMovieRequest* request,
                   GetMovieResponse* response) override {
    try {
      int32_t movieId = request->id();
      std::stringstream sql;
      sql << "SELECT id, name, release_year, description, duration, rating, created_at, updated_at "
          << "FROM movie WHERE id = " << movieId;

      pqxx::result result = db.exec(sql.str());

      if (result.empty()) {
        response->set_success(false);
        response->set_message("Movie not found");
        return Status::OK;
      }

      Movie movie = resultToMovie(result[0]);
      response->set_success(true);
      response->set_message("Movie retrieved successfully");
      *response->mutable_movie() = movie;

      return Status::OK;
    } catch (const std::exception& e) {
      response->set_success(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
  }

  Status ListMovies(ServerContext* context, const ListMoviesRequest* request,
                    ListMoviesResponse* response) override {
    try {
      int limit = request->limit() > 0 ? request->limit() : 100;
      int offset = request->offset() >= 0 ? request->offset() : 0;

      std::stringstream sql;
      sql << "SELECT id, name, release_year, description, duration, rating, created_at, updated_at "
          << "FROM movie ";

      if (!request->search_query().empty()) {
        sql << "WHERE name ILIKE '%" << db.escape(request->search_query()) << "%' "
            << "OR description ILIKE '%" << db.escape(request->search_query()) << "%' ";
      }

      sql << "ORDER BY id DESC "
          << "LIMIT " << limit << " OFFSET " << offset;

      pqxx::result result = db.exec(sql.str());

      for (const auto& row : result) {
        *response->add_movies() = resultToMovie(row);
      }

      // Get total count
      std::stringstream countSql;
      countSql << "SELECT COUNT(*) FROM movie";
      if (!request->search_query().empty()) {
        countSql << " WHERE name ILIKE '%" << db.escape(request->search_query()) << "%' "
                 << "OR description ILIKE '%" << db.escape(request->search_query()) << "%'";
      }

      pqxx::result countResult = db.exec(countSql.str());
      response->set_total_count(countResult[0][0].as<int>());
      response->set_success(true);
      response->set_message("Movies retrieved successfully");

      return Status::OK;
    } catch (const std::exception& e) {
      response->set_success(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
  }

  Status CreateMovie(ServerContext* context, const CreateMovieRequest* request,
                     CreateMovieResponse* response) override {
    try {
      if (request->name().empty()) {
        response->set_success(false);
        response->set_message("Movie name is required");
        return Status::OK;
      }

      std::string timestamp = currentTimestamp();
      std::stringstream sql;
      sql << "INSERT INTO movie (name, release_year, description, duration, rating, created_at, updated_at) "
          << "VALUES ('" << db.escape(request->name()) << "', "
          << request->release_year() << ", '"
          << db.escape(request->description()) << "', "
          << request->duration() << ", '"
          << db.escape(request->rating()) << "', '"
          << timestamp << "', '"
          << timestamp << "') RETURNING id, name, release_year, description, duration, rating, created_at, updated_at";

      pqxx::result result = db.exec(sql.str());

      if (!result.empty()) {
        Movie movie = resultToMovie(result[0]);
        *response->mutable_movie() = movie;
        response->set_success(true);
        response->set_message("Movie created successfully");
      } else {
        response->set_success(false);
        response->set_message("Failed to create movie");
      }

      return Status::OK;
    } catch (const std::exception& e) {
      response->set_success(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
  }

  Status UpdateMovie(ServerContext* context, const UpdateMovieRequest* request,
                     UpdateMovieResponse* response) override {
    try {
      std::string timestamp = currentTimestamp();
      std::stringstream sql;
      sql << "UPDATE movie SET ";

      bool first = true;
      if (!request->name().empty()) {
        sql << "name = '" << db.escape(request->name()) << "'";
        first = false;
      }
      if (request->release_year() > 0) {
        if (!first) { sql << ", "; }
        sql << "release_year = " << request->release_year();
        first = false;
      }
      if (!request->description().empty()) {
        if (!first) { sql << ", "; }
        sql << "description = '" << db.escape(request->description()) << "'";
        first = false;
      }
      if (request->duration() > 0) {
        if (!first) { sql << ", "; }
        sql << "duration = " << request->duration();
        first = false;
      }
      if (!request->rating().empty()) {
        if (!first) { sql << ", "; }
        sql << "rating = '" << db.escape(request->rating()) << "'";
      }

      sql << ", updated_at = '" << timestamp << "' "
          << "WHERE id = " << request->id()
          << " RETURNING id, name, release_year, description, duration, rating, created_at, updated_at";

      pqxx::result result = db.exec(sql.str());

      if (result.empty()) {
        response->set_success(false);
        response->set_message("Movie not found");
        return Status::OK;
      }

      Movie movie = resultToMovie(result[0]);
      *response->mutable_movie() = movie;
      response->set_success(true);
      response->set_message("Movie updated successfully");

      return Status::OK;
    } catch (const std::exception& e) {
      response->set_success(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
  }

  Status DeleteMovie(ServerContext* context, const DeleteMovieRequest* request,
                     DeleteMovieResponse* response) override {
    try {
      std::stringstream sql;
      sql << "DELETE FROM movie WHERE id = " << request->id();

      db.exec(sql.str());

      response->set_success(true);
      response->set_message("Movie deleted successfully");

      return Status::OK;
    } catch (const std::exception& e) {
      response->set_success(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
  }

  Status HealthCheck(ServerContext* context, const HealthCheckRequest* request,
                     HealthCheckResponse* response) override {
    try {
      bool healthy = db.isConnected();
      response->set_healthy(healthy);
      response->set_message(healthy ? "Service is healthy" : "Service is unhealthy");
      return Status::OK;
    } catch (const std::exception& e) {
      response->set_healthy(false);
      response->set_message("Error: " + std::string(e.what()));
      return Status::OK;
    }
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  KioskImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << "\n";
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
