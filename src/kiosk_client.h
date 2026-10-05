#ifndef KIOSK_CLIENT_H
#define KIOSK_CLIENT_H

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <memory>
#include <string>

#include "kiosk/kiosk.grpc.pb.h"

template <typename Response>
struct RpcResult {
  grpc::Status status;
  Response response;

  bool ok() const {
    return status.ok();
  }
};

// Synchronous, thread-safe client for the kiosk.Kiosk service. Every call
// carries a deadline so an unreachable server fails fast instead of hanging.
class KioskClient {
 public:
  static constexpr std::chrono::milliseconds kDefaultTimeout{5000};

  explicit KioskClient(std::shared_ptr<grpc::Channel> channel, std::string target = "",
                       std::chrono::milliseconds timeout = kDefaultTimeout);

  // Opens an insecure channel to `address` (host:port).
  static std::unique_ptr<KioskClient> Connect(const std::string& address,
                                              std::chrono::milliseconds timeout = kDefaultTimeout);

  RpcResult<kiosk::ListMoviesResponse> ListMovies(const std::string& searchQuery, int32_t limit,
                                                  int32_t offset) const;
  RpcResult<kiosk::GetMovieResponse> GetMovie(int32_t id) const;
  RpcResult<kiosk::CreateMovieResponse> CreateMovie(const kiosk::CreateMovieRequest& request) const;
  RpcResult<kiosk::UpdateMovieResponse> UpdateMovie(const kiosk::UpdateMovieRequest& request) const;
  RpcResult<kiosk::DeleteMovieResponse> DeleteMovie(int32_t id) const;
  RpcResult<kiosk::HealthCheckResponse> HealthCheck() const;

  const std::string& target() const {
    return target_;
  }

 private:
  void prepare(grpc::ClientContext& context) const;

  std::unique_ptr<kiosk::Kiosk::Stub> stub_;
  std::string target_;
  std::chrono::milliseconds timeout_;
};

// Turns a failed status into a short message suitable for showing to a user.
std::string describeStatus(const grpc::Status& status);

#endif  // KIOSK_CLIENT_H
