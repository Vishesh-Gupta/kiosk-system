#include <gtest/gtest.h>
#include "../src/db.h"

class DBTest : public ::testing::Test {
protected:
    void SetUp() override {
        // DB configuration is controlled by config/config.json or defaults in DB ctor
        // If you want to ensure test DB usage, make sure config/config.json points to "kiosk_test"
        // Or change DB instantiation here as needed, but DB API (per db.h) has no set* methods
    }

    DB db;
};

TEST_F(DBTest, ConnectionTest) {
    EXPECT_NO_THROW(db.connect());
    EXPECT_TRUE(db.isConnected());
    db.disconnect();
    EXPECT_FALSE(db.isConnected());
}

TEST_F(DBTest, ExecTest) {
    db.connect();
    // Expect no exception on valid SQL
    EXPECT_NO_THROW({
        auto res = db.exec("SELECT 1");
        ASSERT_EQ(res.size(), 1);
        ASSERT_EQ(res[0][0].as<int>(), 1);
    });
    db.disconnect();
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 