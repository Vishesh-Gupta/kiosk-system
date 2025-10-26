#include "db.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <algorithm>
#include <vector>

DB::DB() {
  // Try to load config from common locations
  std::vector<std::string> configPaths = {
      "config.json", 
      "src/config.json",
      "/etc/kiosk/config.json",
      getenv("HOME") ? std::string(getenv("HOME")) + "/.config/kiosk/config.json" : ""
  };

  bool loaded = false;
  for (const auto& path : configPaths) {
    if (!path.empty() && std::filesystem::exists(path)) {
      try {
        std::ifstream configFile(path);
        if (configFile.is_open()) {
          nlohmann::json config;
          configFile >> config;

          dbName = config.value("database", "postgres");
          host = config.value("host", "localhost");
          port = config.value("port", "5432");
          username = config.value("username", "postgres");
          password = config.value("password", "password");

          loaded = true;
          std::cout << "Loaded configuration from: " << path << std::endl;
          break;
        }
      } catch (const std::exception& e) {
        std::cerr << "Error loading config from " << path << ": " << e.what() << std::endl;
      }
    }
  }

  if (!loaded) {
    // Fallback to defaults
    dbName = "postgres";
    host = "localhost";
    port = "5432";
    username = "postgres";
    password = "password";

    std::cerr << "Warning: Using default database configuration" << std::endl;
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

  std::stringstream connStr;
  connStr << "postgresql://" << username << ":" << password << "@" << host << ":" << port << "/"
          << dbName;

  try {
    conn = std::make_unique<pqxx::connection>(connStr.str());
    std::cout << "Successfully connected to database: " << dbName << std::endl;
  } catch (const std::exception& e) {
    throw DBException("Failed to connect to database: " + std::string(e.what()));
  }
}

void DB::reconnect() {
  std::lock_guard<std::mutex> lock(dbMutex);
  disconnect();
  connect();
}

void DB::disconnect() {
  if (conn) {
    conn->close();
    conn.reset();
  }
}

bool DB::isConnected() const {
  std::lock_guard<std::mutex> lock(dbMutex);
  return conn && conn->is_open();
}

void DB::query(const std::string& sql) {
  if (!isConnected()) {
    connect();
  }

  try {
    std::lock_guard<std::mutex> lock(dbMutex);
    pqxx::work txn{*conn};
    txn.exec(sql);
    txn.commit();
  } catch (const std::exception& e) {
    throw DBException("Query execution failed: " + std::string(e.what()));
  }
}

pqxx::result DB::exec(const std::string& sql) {
  if (!isConnected()) {
    connect();
  }

  try {
    std::lock_guard<std::mutex> lock(dbMutex);
    pqxx::work txn{*conn};
    pqxx::result r = txn.exec(sql);
    txn.commit();
    return r;
  } catch (const std::exception& e) {
    throw DBException("Query execution failed: " + std::string(e.what()));
  }
}

pqxx::result DB::exec_params(const std::string& sql, const std::vector<std::string>& params) {
  if (!isConnected()) {
    connect();
  }

  try {
    std::lock_guard<std::mutex> lock(dbMutex);
    pqxx::work txn{*conn};
    
    // Simple implementation - execute with raw parameters
    // Note: For production use, use pqxx::params or static query building
    if (params.empty()) {
      pqxx::result r = txn.exec(sql);
      txn.commit();
      return r;
    } else {
      // Create parameter array for simple cases
      throw DBException("Complex parameterized queries not yet implemented");
    }
  } catch (const std::exception& e) {
    throw DBException("Parameterized query execution failed: " + std::string(e.what()));
  }
}

std::string DB::escape(const std::string& str) const {
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
