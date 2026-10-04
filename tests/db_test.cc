#include "db.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "test_support.h"

TEST(DBConfigTest, MissingFileUsesDefaults) {
  DBConfig config = DBConfig::loadFromFile("does/not/exist.json");
  EXPECT_EQ(config.dbName, "kiosk");
  EXPECT_EQ(config.host, "localhost");
  EXPECT_EQ(config.port, "5432");
}

TEST(DBConfigTest, ReadsFileAndAcceptsNumericPort) {
  const std::string path = ::testing::TempDir() + "kiosk_db_config.json";
  {
    std::ofstream out(path);
    out << R"({"database": "films", "host": "db.internal", "port": 6543, "username": "u"})";
  }
  DBConfig config = DBConfig::loadFromFile(path);
  std::remove(path.c_str());

  EXPECT_EQ(config.dbName, "films");
  EXPECT_EQ(config.host, "db.internal");
  EXPECT_EQ(config.port, "6543");
  EXPECT_EQ(config.username, "u");
  EXPECT_EQ(config.password, "postgres");
}

TEST(DBConfigTest, InvalidJsonThrows) {
  const std::string path = ::testing::TempDir() + "kiosk_db_bad_config.json";
  {
    std::ofstream out(path);
    out << "{not json";
  }
  EXPECT_THROW(DBConfig::loadFromFile(path), DBException);
  std::remove(path.c_str());
}

TEST(DBConfigTest, EnvironmentOverridesFile) {
  DBConfig config;
  setenv("DB_HOST", "override-host", 1);
  setenv("DB_PORT", "15432", 1);
  config.applyEnvironment();
  unsetenv("DB_HOST");
  unsetenv("DB_PORT");

  EXPECT_EQ(config.host, "override-host");
  EXPECT_EQ(config.port, "15432");
  EXPECT_EQ(config.dbName, "kiosk");
}

TEST(DBConfigTest, ConnectionStringQuotesValues) {
  DBConfig config;
  config.password = "it's a \\secret";
  EXPECT_NE(config.connectionString().find(R"(password='it\'s a \\secret')"), std::string::npos);
}

TEST(DBTest, UnreachableDatabaseThrowsUnavailable) {
  DBConfig config;
  config.host = "127.0.0.1";
  config.port = "1";  // nothing listens here
  DB db(config);
  EXPECT_THROW(db.connect(), DBUnavailableException);
  EXPECT_THROW(db.exec("SELECT 1"), DBUnavailableException);
  EXPECT_FALSE(db.ping());
  EXPECT_FALSE(db.isConnected());
}

class DBIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    KIOSK_SKIP_WITHOUT_DB(db);
  }

  DB db;
};

TEST_F(DBIntegrationTest, ConnectAndDisconnect) {
  EXPECT_NO_THROW(db.connect());
  EXPECT_TRUE(db.isConnected());
  db.disconnect();
  EXPECT_FALSE(db.isConnected());
}

TEST_F(DBIntegrationTest, ExecReturnsRows) {
  pqxx::result result = db.exec("SELECT 1");
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0][0].as<int>(), 1);
}

TEST_F(DBIntegrationTest, ExecBindsParametersSafely) {
  const std::string hostile = "'; DROP TABLE movie; --";
  pqxx::result result = db.exec("SELECT $1::text, $2::int + 1", pqxx::params{hostile, 41});
  EXPECT_EQ(result[0][0].as<std::string>(), hostile);
  EXPECT_EQ(result[0][1].as<int>(), 42);
}

TEST_F(DBIntegrationTest, ExecReconnectsAfterDisconnect) {
  db.disconnect();
  EXPECT_EQ(db.exec("SELECT 2")[0][0].as<int>(), 2);
  EXPECT_TRUE(db.isConnected());
}

TEST_F(DBIntegrationTest, SqlErrorThrowsDBException) {
  EXPECT_THROW(db.exec("SELECT * FROM no_such_table"), DBException);
  // The connection is still usable afterwards.
  EXPECT_TRUE(db.ping());
}
