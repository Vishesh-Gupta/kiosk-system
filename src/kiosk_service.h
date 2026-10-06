#ifndef KIOSK_SERVICE_H
#define KIOSK_SERVICE_H

#include <grpcpp/grpcpp.h>

#include "db.h"
#include "kiosk/kiosk.grpc.pb.h"

// gRPC implementation of kiosk.Kiosk backed by PostgreSQL.
//
// Successful calls return Status::OK with `success = true`. Failures are
// reported through the gRPC status code (INVALID_ARGUMENT, NOT_FOUND,
// UNAVAILABLE, INTERNAL) with a human-readable message; as with any gRPC
// call, the response body is not delivered to the client in that case.
class KioskServiceImpl final : public kiosk::Kiosk::Service {
 public:
  static constexpr int kDefaultPageSize = 100;
  static constexpr int kMaxPageSize = 1000;

  explicit KioskServiceImpl(DB& db);

  grpc::Status GetMovie(grpc::ServerContext* context, const kiosk::GetMovieRequest* request,
                        kiosk::GetMovieResponse* response) override;

  grpc::Status ListMovies(grpc::ServerContext* context, const kiosk::ListMoviesRequest* request,
                          kiosk::ListMoviesResponse* response) override;

  grpc::Status CreateMovie(grpc::ServerContext* context, const kiosk::CreateMovieRequest* request,
                           kiosk::CreateMovieResponse* response) override;

  grpc::Status UpdateMovie(grpc::ServerContext* context, const kiosk::UpdateMovieRequest* request,
                           kiosk::UpdateMovieResponse* response) override;

  grpc::Status DeleteMovie(grpc::ServerContext* context, const kiosk::DeleteMovieRequest* request,
                           kiosk::DeleteMovieResponse* response) override;

  grpc::Status HealthCheck(grpc::ServerContext* context, const kiosk::HealthCheckRequest* request,
                           kiosk::HealthCheckResponse* response) override;

 private:
  DB& db;
};

#endif  // KIOSK_SERVICE_H
