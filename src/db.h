#ifndef DB_H
#define DB_H

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <mutex>
#include <exception>
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
  
  // Executes a query without returning results
  void query(const std::string& sql);
  
  // Executes a query and returns the results
  pqxx::result exec(const std::string& sql);

  pqxx::result exec_params(const std::string& sql, const std::vector<std::string>& params);

  std::string escape(const std::string& str) const;

  // Connection management
  bool isConnected() const;
  void connect();
  void disconnect();
  void reconnect();

  // Getters/setters for connection params
  void setDbName(const std::string& name) { dbName = name; }
  void setHost(const std::string& h) { host = h; }
  void setPort(const std::string& p) { port = p; }
  void setUsername(const std::string& user) { username = user; }
  void setPassword(const std::string& pass) { password = pass; }

  // Getters
  std::string getDbName() const { return dbName; }
  std::string getHost() const { return host; }
};

#endif // DB_H
