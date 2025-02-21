#include "db.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

DB::DB() {
  // Try to load config from common locations
  std::vector<std::string> configPaths = {
      "config.json", "/etc/kiosk/config.json",
      std::string(getenv("HOME") ? getenv("HOME") : "") + "/.config/kiosk/config.json"};

  bool loaded = false;
  for (const auto& path : configPaths) {
    if (std::filesystem::exists(path)) {
      try {
        std::ifstream configFile(path);
        nlohmann::json config;
        configFile >> config;

        dbName = config.value("database", "postgres");
        host = config.value("host", "localhost");
        port = config.value("port", "5432");
        username = config.value("username", "postgres");
        password = config.value("password", "password");

        loaded = true;
        break;
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

void DB::connect() {
  if (conn) {
    return;
  }

  std::stringstream connStr;
  connStr << "postgresql://" << username << ":" << password << "@" << host << ":" << port << "/"
          << dbName;

  conn = std::make_unique<pqxx::connection>(connStr.str());
}

void DB::disconnect() {
  conn.reset();
}

bool DB::isConnected() const {
  return conn && conn->is_open();
}

void DB::query(const std::string& sql) {
  if (!isConnected()) {
    connect();
  }

  pqxx::work txn{*conn};
  txn.exec(sql);
  txn.commit();
}

pqxx::result DB::exec(const std::string& sql) {
  if (!isConnected()) {
    connect();
  }

  pqxx::work txn{*conn};
  pqxx::result r = txn.exec(sql);
  txn.commit();
  return r;
}
