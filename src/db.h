#ifndef DB_H
#define DB_H

#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <string_view>

// Raised for connection failures and SQL errors.
class DBException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Raised when the database cannot be reached at all.
class DBUnavailableException : public DBException {
 public:
  using DBException::DBException;
};

struct DBConfig {
  std::string dbName = "kiosk";
  std::string host = "localhost";
  std::string port = "5432";
  std::string username = "postgres";
  std::string password = "postgres";

  // Loads settings from a JSON file (path from $KIOSK_CONFIG, otherwise
  // config/config.json), then applies DB_HOST, DB_PORT, DB_NAME, DB_USER and
  // DB_PASSWORD environment overrides. Missing values keep their defaults.
  static DBConfig load();
  static DBConfig loadFromFile(const std::string& path);
  void applyEnvironment();

  std::string connectionString() const;
};

// A single PostgreSQL connection shared by all callers. Every query runs in
// its own transaction under a mutex, and the connection is (re)opened lazily,
// so the service recovers if the database restarts.
class DB {
 public:
  explicit DB(DBConfig config = DBConfig::load());
  ~DB();

  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  // Runs `sql` with positional parameters ($1, $2, ...) and commits.
  pqxx::result exec(std::string_view sql, const pqxx::params& params = {});

  // Returns true if the database answers a trivial query.
  bool ping();

  bool isConnected() const;
  void connect();
  void disconnect();
  void reconnect();

  const std::string& getDbName() const {
    return config.dbName;
  }
  const std::string& getHost() const {
    return config.host;
  }

 private:
  void connectLocked();
  void disconnectLocked();

  DBConfig config;
  std::unique_ptr<pqxx::connection> conn;
  mutable std::mutex dbMutex;
};

#endif  // DB_H
