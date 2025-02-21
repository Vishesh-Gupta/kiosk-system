#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "../proto/gen/kiosk/kiosk.grpc.pb.h"
#include "db.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using kiosk::DeleteDataRequest;
using kiosk::DeleteDataResponse;
using kiosk::GetDataRequest;
using kiosk::GetDataResponse;
using kiosk::Kiosk;
using kiosk::ListDataRequest;
using kiosk::ListDataResponse;
using kiosk::PostDataRequest;
using kiosk::PostDataResponse;
using kiosk::UpdateDataRequest;
using kiosk::UpdateDataResponse;

class KioskImpl final : public Kiosk::Service {
 private:
  DB db;

 public:
  Status GetData(ServerContext* context, const GetDataRequest* request,
                 GetDataResponse* response) override {
    std::string name = request->name();
    response->set_name(name);
    return Status::OK;
  }

  Status ListData(ServerContext* context, const ListDataRequest* request,
                  ListDataResponse* response) override {
    std::string message = request->message();
    response->set_message(message);
    return Status::OK;
  }

  Status PostData(ServerContext* context, const PostDataRequest* request,
                  PostDataResponse* response) override {
    std::string message = request->message();
    response->set_message(message);
    return Status::OK;
  }

  Status DeleteData(ServerContext* context, const DeleteDataRequest* request,
                    DeleteDataResponse* response) override {
    std::string message = request->message();
    response->set_message(message);
    return Status::OK;
  }

  Status UpdateData(ServerContext* context, const UpdateDataRequest* request,
                    UpdateDataResponse* response) override {
    std::string message = request->message();
    response->set_message(message);
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  KioskImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();
  return 0;
}
