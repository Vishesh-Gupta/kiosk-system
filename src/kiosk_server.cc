#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "db.h"
#include "kiosk_service.h"

namespace {

std::string listenAddress() {
  if (const char* address = std::getenv("KIOSK_ADDRESS"); address != nullptr && *address != '\0') {
    return address;
  }
  return "0.0.0.0:50051";
}

}  // namespace

int main() {
  // Block SIGINT/SIGTERM in every thread (gRPC's included) so a dedicated
  // thread can wait for them and shut the server down cleanly.
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &signals, nullptr);

  DB db;
  try {
    db.connect();
    std::cout << "Connected to database " << db.getDbName() << " at " << db.getHost() << "\n";
  } catch (const std::exception& e) {
    // Not fatal: the connection is retried on every request.
    std::cerr << "Database not reachable yet: " << e.what() << "\n";
  }

  KioskServiceImpl service(db);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  const std::string address = listenAddress();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "Failed to start gRPC server on " << address << "\n";
    return EXIT_FAILURE;
  }
  std::cout << "Server listening on " << address << std::endl;

  std::thread signalWaiter([&server, &signals] {
    int signal = 0;
    sigwait(&signals, &signal);
    std::cout << "Received signal " << signal << ", shutting down" << std::endl;
    server->Shutdown();
  });

  server->Wait();
  signalWaiter.join();
  return EXIT_SUCCESS;
}
