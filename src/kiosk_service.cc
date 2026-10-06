#include "kiosk_service.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using kiosk::CreateMovieRequest;
using kiosk::CreateMovieResponse;
using kiosk::DeleteMovieRequest;
using kiosk::DeleteMovieResponse;
using kiosk::GetMovieRequest;
using kiosk::GetMovieResponse;
using kiosk::HealthCheckRequest;
using kiosk::HealthCheckResponse;
using kiosk::ListMoviesRequest;
using kiosk::ListMoviesResponse;
using kiosk::Movie;
using kiosk::UpdateMovieRequest;
using kiosk::UpdateMovieResponse;

namespace {

// Column order must match rowToMovie().
constexpr const char* kMovieColumns =
    "id, name, release_year, description, duration, rating, "
    "to_char(created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS'), "
    "to_char(updated_at, 'YYYY-MM-DD\"T\"HH24:MI:SS')";

constexpr const char* kSearchFilter =
    " WHERE name ILIKE '%' || $1 || '%' OR description ILIKE '%' || $1 || '%'";

Movie rowToMovie(const pqxx::row& row) {
  Movie movie;
  movie.set_id(row[0].as<int32_t>());
  movie.set_name(row[1].as<std::string>());
  movie.set_release_year(row[2].as<int32_t>());
  movie.set_description(row[3].as<std::string>(""));
  movie.set_duration(row[4].as<int32_t>());
  movie.set_rating(row[5].as<std::string>(""));
  movie.set_created_at(row[6].as<std::string>(""));
  movie.set_updated_at(row[7].as<std::string>(""));
  return movie;
}

// Escapes LIKE wildcards so a search for "100%" matches literally.
std::string escapeLike(const std::string& input) {
  std::string escaped;
  escaped.reserve(input.size());
  for (char c : input) {
    if (c == '%' || c == '_' || c == '\\') {
      escaped += '\\';
    }
    escaped += c;
  }
  return escaped;
}

std::optional<std::string> nonEmpty(const std::string& value) {
  if (value.empty()) {
    return std::nullopt;
  }
  return value;
}

std::optional<int32_t> positive(int32_t value) {
  if (value <= 0) {
    return std::nullopt;
  }
  return value;
}

Status validateMovieFields(int32_t release_year, int32_t duration) {
  if (release_year < 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "release_year must not be negative");
  }
  if (duration < 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "duration must not be negative");
  }
  return Status::OK;
}

// Maps database failures onto gRPC status codes and logs them server-side.
Status dbError(const char* rpc, const std::exception& e) {
  std::cerr << rpc << " failed: " << e.what() << "\n";
  if (dynamic_cast<const DBUnavailableException*>(&e) != nullptr) {
    return Status(StatusCode::UNAVAILABLE, "Database unavailable");
  }
  return Status(StatusCode::INTERNAL, "Database error");
}

}  // namespace

KioskServiceImpl::KioskServiceImpl(DB& db) : db(db) {}

Status KioskServiceImpl::GetMovie(ServerContext* /*context*/, const GetMovieRequest* request,
                                  GetMovieResponse* response) {
  if (request->id() <= 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "id must be positive");
  }

  try {
    pqxx::result result =
        db.exec(std::string("SELECT ") + kMovieColumns + " FROM movie WHERE id = $1",
                pqxx::params{request->id()});
    if (result.empty()) {
      return Status(StatusCode::NOT_FOUND, "Movie not found");
    }

    *response->mutable_movie() = rowToMovie(result[0]);
    response->set_success(true);
    response->set_message("Movie retrieved successfully");
    return Status::OK;
  } catch (const std::exception& e) {
    return dbError("GetMovie", e);
  }
}

Status KioskServiceImpl::ListMovies(ServerContext* /*context*/, const ListMoviesRequest* request,
                                    ListMoviesResponse* response) {
  if (request->limit() < 0 || request->offset() < 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "limit and offset must not be negative");
  }
  const int32_t limit =
      request->limit() == 0 ? kDefaultPageSize : std::min(request->limit(), kMaxPageSize);
  const int32_t offset = request->offset();
  const bool hasSearch = !request->search_query().empty();
  const std::string pattern = escapeLike(request->search_query());

  try {
    std::string sql = std::string("SELECT ") + kMovieColumns + " FROM movie";
    std::string countSql = "SELECT COUNT(*) FROM movie";
    pqxx::params params;
    pqxx::params countParams;
    if (hasSearch) {
      sql += kSearchFilter;
      countSql += kSearchFilter;
      params.append(pattern);
      countParams.append(pattern);
    }
    const int nextParam = hasSearch ? 2 : 1;
    sql += " ORDER BY id DESC LIMIT $" + std::to_string(nextParam) + " OFFSET $" +
           std::to_string(nextParam + 1);
    params.append(limit);
    params.append(offset);

    pqxx::result rows = db.exec(sql, params);
    pqxx::result count = db.exec(countSql, countParams);

    for (const auto& row : rows) {
      *response->add_movies() = rowToMovie(row);
    }
    response->set_total_count(count[0][0].as<int32_t>());
    response->set_success(true);
    response->set_message("Movies retrieved successfully");
    return Status::OK;
  } catch (const std::exception& e) {
    return dbError("ListMovies", e);
  }
}

Status KioskServiceImpl::CreateMovie(ServerContext* /*context*/, const CreateMovieRequest* request,
                                     CreateMovieResponse* response) {
  if (request->name().empty()) {
    return Status(StatusCode::INVALID_ARGUMENT, "Movie name is required");
  }
  if (Status status = validateMovieFields(request->release_year(), request->duration());
      !status.ok()) {
    return status;
  }

  try {
    pqxx::result result = db.exec(
        std::string("INSERT INTO movie (name, release_year, description, duration, rating) "
                    "VALUES ($1, $2, $3, $4, $5) RETURNING ") +
            kMovieColumns,
        pqxx::params{request->name(), request->release_year(), nonEmpty(request->description()),
                     request->duration(), nonEmpty(request->rating())});

    *response->mutable_movie() = rowToMovie(result.at(0));
    response->set_success(true);
    response->set_message("Movie created successfully");
    return Status::OK;
  } catch (const std::exception& e) {
    return dbError("CreateMovie", e);
  }
}

Status KioskServiceImpl::UpdateMovie(ServerContext* /*context*/, const UpdateMovieRequest* request,
                                     UpdateMovieResponse* response) {
  // proto3 scalars have no presence, so empty strings and zero numbers mean
  // "leave this field unchanged".
  if (request->id() <= 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "id must be positive");
  }
  if (Status status = validateMovieFields(request->release_year(), request->duration());
      !status.ok()) {
    return status;
  }
  if (request->name().empty() && request->release_year() == 0 && request->description().empty() &&
      request->duration() == 0 && request->rating().empty()) {
    return Status(StatusCode::INVALID_ARGUMENT, "No fields to update");
  }

  try {
    pqxx::result result =
        db.exec(std::string("UPDATE movie SET "
                            "name = COALESCE($2, name), "
                            "release_year = COALESCE($3, release_year), "
                            "description = COALESCE($4, description), "
                            "duration = COALESCE($5, duration), "
                            "rating = COALESCE($6, rating), "
                            "updated_at = CURRENT_TIMESTAMP "
                            "WHERE id = $1 RETURNING ") +
                    kMovieColumns,
                pqxx::params{request->id(), nonEmpty(request->name()),
                             positive(request->release_year()), nonEmpty(request->description()),
                             positive(request->duration()), nonEmpty(request->rating())});
    if (result.empty()) {
      return Status(StatusCode::NOT_FOUND, "Movie not found");
    }

    *response->mutable_movie() = rowToMovie(result[0]);
    response->set_success(true);
    response->set_message("Movie updated successfully");
    return Status::OK;
  } catch (const std::exception& e) {
    return dbError("UpdateMovie", e);
  }
}

Status KioskServiceImpl::DeleteMovie(ServerContext* /*context*/, const DeleteMovieRequest* request,
                                     DeleteMovieResponse* response) {
  if (request->id() <= 0) {
    return Status(StatusCode::INVALID_ARGUMENT, "id must be positive");
  }

  try {
    pqxx::result result =
        db.exec("DELETE FROM movie WHERE id = $1 RETURNING id", pqxx::params{request->id()});
    if (result.empty()) {
      return Status(StatusCode::NOT_FOUND, "Movie not found");
    }

    response->set_success(true);
    response->set_message("Movie deleted successfully");
    return Status::OK;
  } catch (const std::exception& e) {
    return dbError("DeleteMovie", e);
  }
}

Status KioskServiceImpl::HealthCheck(ServerContext* /*context*/,
                                     const HealthCheckRequest* /*request*/,
                                     HealthCheckResponse* response) {
  const bool healthy = db.ping();
  response->set_healthy(healthy);
  response->set_message(healthy ? "Service is healthy" : "Database unreachable");
  return Status::OK;
}
