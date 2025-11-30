#include "db.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DB::DB() {
  std::ifstream configFile("config/config.json");
  if (configFile.is_open()) {
    json config;
    configFile >> config;
    dbName = config.value("database", "kiosk");
    host = config.value("host", "localhost");
    port = config.value("port", "5432");
    username = config.value("username", "postgres");
    password = config.value("password", "password");
  } else {
    dbName = "kiosk";
    host = "localhost";
    port = "5432";
    username = "postgres";
    password = "password";
  }
}

DB::~DB() {
  disconnect();
}

void DB::connect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  
  if (conn && conn->is_open()) {
    return;
  }

  try {
    std::string connectionString = "host=" + host + 
                                   " port=" + port + 
                                   " dbname=" + dbName + 
                                   " user=" + username + 
                                   " password=" + password;
    conn = std::make_unique<pqxx::connection>(connectionString);
    
    if (!conn->is_open()) {
      throw DBException("Failed to open database connection");
    }
  } catch (const std::exception& e) {
    throw DBException("Database connection error: " + std::string(e.what()));
  }
}

void DB::disconnect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  if (conn && conn->is_open()) {
    conn->close();
  }
  conn.reset();
}

void DB::reconnect() {
  disconnect();
  connect();
}

bool DB::isConnected() const {
  std::lock_guard<std::mutex> lock(dbMutex);
  return conn != nullptr && conn->is_open();
}

pqxx::result DB::exec(const std::string& sql) {
  std::lock_guard<std::mutex> lock(dbMutex);
  
  if (!conn || !conn->is_open()) {
    throw DBException("Database not connected");
  }

  try {
    pqxx::work txn(*conn);
    pqxx::result result = txn.exec(sql);
    txn.commit();
    return result;
  } catch (const std::exception& e) {
    throw DBException("SQL execution error: " + std::string(e.what()));
  }
}

pqxx::result DB::exec_params(const std::string& sql, const std::vector<std::string>& params) {
  std::lock_guard<std::mutex> lock(dbMutex);
  
  if (!conn || !conn->is_open()) {
    throw DBException("Database not connected");
  }

  try {
    pqxx::work txn(*conn);
    
    // Convert vector<string> to vector<const char*>
    std::vector<const char*> c_params;
    for (const auto& param : params) {
      c_params.push_back(param.c_str());
    }
    
    pqxx::result result = txn.exec_params(sql, c_params);
    txn.commit();
    return result;
  } catch (const std::exception& e) {
    throw DBException("SQL execution error: " + std::string(e.what()));
  }
}

std::string DB::escape(const std::string& str) const {
  std::lock_guard<std::mutex> lock(dbMutex);
  
  if (!conn || !conn->is_open()) {
    // If not connected, do basic escaping
    std::string escaped;
    escaped.reserve(str.length() * 2);
    for (char c : str) {
      if (c == '\'') {
        escaped += "''";
      } else if (c == '\\') {
        escaped += "\\\\";
      } else {
        escaped += c;
      }
    }
    return escaped;
  }

  try {
    pqxx::work txn(*conn);
    return txn.esc(str);
  } catch (const std::exception& e) {
    // Fallback to basic escaping
    std::string escaped;
    escaped.reserve(str.length() * 2);
    for (char c : str) {
      if (c == '\'') {
        escaped += "''";
      } else if (c == '\\') {
        escaped += "\\\\";
      } else {
        escaped += c;
      }
    }
    return escaped;
  }
}