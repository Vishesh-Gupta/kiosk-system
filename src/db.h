#ifndef DB_H
#define DB_H

#include <exception>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <string>
#include <vector>

// Custom exception for database operations
class DBException : public std::exception {
private:
  std::string message;
public:
  explicit DBException(const std::string& msg) : message(msg) {}
  const char* what() const noexcept override { return message.c_str(); }
};

class DB {
private:
  std::string dbName;
  std::string host;
  std::string port;
  std::string password; 
  std::string username;
  std::unique_ptr<pqxx::connection> conn;
  mutable std::mutex dbMutex; // For thread-safety

public:
  DB();
  ~DB();
  
  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  // Move constructor
  DB(DB&& other) noexcept
      : dbName(std::move(other.dbName)),
        host(std::move(other.host)),
        port(std::move(other.port)),
        password(std::move(other.password)),
        username(std::move(other.username)),
        conn(std::move(other.conn)) {}

  // Move assignment operator
  DB& operator=(DB&& other) noexcept {
    if (this != &other) {
      std::lock(dbMutex, other.dbMutex);
      std::lock_guard<std::mutex> lhs_lock(dbMutex, std::adopt_lock);
      std::lock_guard<std::mutex> rhs_lock(other.dbMutex, std::adopt_lock);

      dbName = std::move(other.dbName);
      host = std::move(other.host);
      port = std::move(other.port);
      password = std::move(other.password);
      username = std::move(other.username);
      conn = std::move(other.conn);
    }
    return *this;
  }
  
  pqxx::result exec(const std::string& sql);

  pqxx::result exec_params(const std::string& sql, const std::vector<std::string>& params);

  std::string escape(const std::string& str) const;

  
  bool isConnected() const;
  void connect();
  void disconnect();
  void reconnect();

  std::string getDbName() const { return dbName; }
  std::string getHost() const { return host; }
};

#endif // DB_H
