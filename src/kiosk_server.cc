#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <ctime>

#include "db.h"

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "../api/gen/kiosk.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerAsyncReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using kiosk::Kiosk;

class KioskImpl final : public Kiosk::Service {
  public:
  Status GetKiosk();
  Status ListKiosk();
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  KioskImpl kiosk;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&kiosk);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}


int main(int argc, char* argv[]) {
  RunServer();
  return 0;
}
