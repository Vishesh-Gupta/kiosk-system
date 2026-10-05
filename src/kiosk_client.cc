#include "kiosk_client.h"

#include <utility>

KioskClient::KioskClient(std::shared_ptr<grpc::Channel> channel, std::string target,
                         std::chrono::milliseconds timeout)
    : stub_(kiosk::Kiosk::NewStub(std::move(channel))),
      target_(std::move(target)),
      timeout_(timeout) {}

std::unique_ptr<KioskClient> KioskClient::Connect(const std::string& address,
                                                  std::chrono::milliseconds timeout) {
  // gRPC's default reconnect backoff grows to two minutes, during which calls
  // fail immediately. An interactive client should notice a restarted server
  // within a couple of seconds instead.
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 500);
  args.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, 500);
  args.SetInt(GRPC_ARG_MAX_RECONNECT_BACKOFF_MS, 2000);
  return std::make_unique<KioskClient>(
      grpc::CreateCustomChannel(address, grpc::InsecureChannelCredentials(), args), address,
      timeout);
}

void KioskClient::prepare(grpc::ClientContext& context) const {
  context.set_deadline(std::chrono::system_clock::now() + timeout_);
}

RpcResult<kiosk::ListMoviesResponse> KioskClient::ListMovies(const std::string& searchQuery,
                                                             int32_t limit, int32_t offset) const {
  kiosk::ListMoviesRequest request;
  request.set_search_query(searchQuery);
  request.set_limit(limit);
  request.set_offset(offset);
  RpcResult<kiosk::ListMoviesResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->ListMovies(&context, request, &result.response);
  return result;
}

RpcResult<kiosk::GetMovieResponse> KioskClient::GetMovie(int32_t id) const {
  kiosk::GetMovieRequest request;
  request.set_id(id);
  RpcResult<kiosk::GetMovieResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->GetMovie(&context, request, &result.response);
  return result;
}

RpcResult<kiosk::CreateMovieResponse> KioskClient::CreateMovie(
    const kiosk::CreateMovieRequest& request) const {
  RpcResult<kiosk::CreateMovieResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->CreateMovie(&context, request, &result.response);
  return result;
}

RpcResult<kiosk::UpdateMovieResponse> KioskClient::UpdateMovie(
    const kiosk::UpdateMovieRequest& request) const {
  RpcResult<kiosk::UpdateMovieResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->UpdateMovie(&context, request, &result.response);
  return result;
}

RpcResult<kiosk::DeleteMovieResponse> KioskClient::DeleteMovie(int32_t id) const {
  kiosk::DeleteMovieRequest request;
  request.set_id(id);
  RpcResult<kiosk::DeleteMovieResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->DeleteMovie(&context, request, &result.response);
  return result;
}

RpcResult<kiosk::HealthCheckResponse> KioskClient::HealthCheck() const {
  RpcResult<kiosk::HealthCheckResponse> result;
  grpc::ClientContext context;
  prepare(context);
  result.status = stub_->HealthCheck(&context, kiosk::HealthCheckRequest(), &result.response);
  return result;
}

std::string describeStatus(const grpc::Status& status) {
  switch (status.error_code()) {
    case grpc::StatusCode::OK:
      return "OK";
    case grpc::StatusCode::UNAVAILABLE:
      return status.error_message() == "Database unavailable" ? "Database unavailable"
                                                              : "Server unreachable";
    case grpc::StatusCode::DEADLINE_EXCEEDED:
      return "Server did not respond in time";
    case grpc::StatusCode::INVALID_ARGUMENT:
    case grpc::StatusCode::NOT_FOUND:
      return status.error_message();
    default:
      return "Server error: " + status.error_message();
  }
}
