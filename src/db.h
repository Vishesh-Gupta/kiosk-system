#include <pqxx/pqxx>

class DB {
  std::string dbName;
  std::string host;
  std::string port;
  std::string password;
  std::string username;

public:
  DB();
  void query(std::string sql);
  pqxx::result exec(std::string sql);
};
