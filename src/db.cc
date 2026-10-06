#include "db.h"

#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

// Quotes a libpq connection-string value so spaces, quotes and backslashes in
// passwords etc. are passed through intact.
std::string quoteConnValue(const std::string& value) {
  std::string quoted = "'";
  for (char c : value) {
    if (c == '\'' || c == '\\') {
      quoted += '\\';
    }
    quoted += c;
  }
  quoted += '\'';
  return quoted;
}

void overrideFromEnv(const char* name, std::string& target) {
  if (const char* value = std::getenv(name); value != nullptr && *value != '\0') {
    target = value;
  }
}

// Accepts "5432" or 5432 for convenience.
std::string jsonString(const json& config, const char* key, const std::string& fallback) {
  auto it = config.find(key);
  if (it == config.end() || it->is_null()) {
    return fallback;
  }
  if (it->is_string()) {
    return it->get<std::string>();
  }
  return it->dump();
}

}  // namespace

DBConfig DBConfig::load() {
  const char* path = std::getenv("KIOSK_CONFIG");
  DBConfig config = loadFromFile(path != nullptr && *path != '\0' ? path : "config/config.json");
  config.applyEnvironment();
  return config;
}

DBConfig DBConfig::loadFromFile(const std::string& path) {
  DBConfig config;
  std::ifstream configFile(path);
  if (!configFile.is_open()) {
    return config;
  }

  json parsed;
  try {
    configFile >> parsed;
  } catch (const json::exception& e) {
    throw DBException("Invalid database config " + path + ": " + e.what());
  }

  config.dbName = jsonString(parsed, "database", config.dbName);
  config.host = jsonString(parsed, "host", config.host);
  config.port = jsonString(parsed, "port", config.port);
  config.username = jsonString(parsed, "username", config.username);
  config.password = jsonString(parsed, "password", config.password);
  return config;
}

void DBConfig::applyEnvironment() {
  overrideFromEnv("DB_NAME", dbName);
  overrideFromEnv("DB_HOST", host);
  overrideFromEnv("DB_PORT", port);
  overrideFromEnv("DB_USER", username);
  overrideFromEnv("DB_PASSWORD", password);
}

std::string DBConfig::connectionString() const {
  return "host=" + quoteConnValue(host) + " port=" + quoteConnValue(port) +
         " dbname=" + quoteConnValue(dbName) + " user=" + quoteConnValue(username) +
         " password=" + quoteConnValue(password) + " connect_timeout=5";
}

DB::DB(DBConfig config) : config(std::move(config)) {}

DB::~DB() {
  disconnect();
}

void DB::connect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  connectLocked();
}

void DB::disconnect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  disconnectLocked();
}

void DB::reconnect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  disconnectLocked();
  connectLocked();
}

bool DB::isConnected() const {
  std::lock_guard<std::mutex> lock(dbMutex);
  return conn != nullptr && conn->is_open();
}

bool DB::ping() {
  try {
    exec("SELECT 1");
    return true;
  } catch (const DBException&) {
    return false;
  }
}

void DB::connectLocked() {
  if (conn && conn->is_open()) {
    return;
  }
  try {
    conn = std::make_unique<pqxx::connection>(config.connectionString());
  } catch (const std::exception& e) {
    conn.reset();
    throw DBUnavailableException("Database connection error: " + std::string(e.what()));
  }
}

void DB::disconnectLocked() {
  if (conn && conn->is_open()) {
    conn->close();
  }
  conn.reset();
}

pqxx::result DB::exec(std::string_view sql, const pqxx::params& params) {
  std::lock_guard<std::mutex> lock(dbMutex);

  // A broken connection is detected before the transaction commits, so it is
  // safe to reconnect and retry exactly once.
  for (int attempt = 0;; ++attempt) {
    connectLocked();
    try {
      pqxx::work txn(*conn);
      pqxx::result result = txn.exec(sql, params);
      txn.commit();
      return result;
    } catch (const pqxx::broken_connection& e) {
      disconnectLocked();
      if (attempt > 0) {
        throw DBUnavailableException("Database connection lost: " + std::string(e.what()));
      }
    } catch (const std::exception& e) {
      throw DBException("SQL execution error: " + std::string(e.what()));
    }
  }
}
