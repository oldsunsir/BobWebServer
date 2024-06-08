// sqlconnpool_test.cpp
#include <gtest/gtest.h>
#include "../code/pool/sqlconnpool.h"

// 测试套件
class SqlConnPoolTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // 初始化数据库连接池
        sql_pool::instance()->init("localhost", 3306, "test_user", "test_password", "test_db", 5);
    }

    static void TearDownTestSuite() {
        // 关闭数据库连接池
        sql_pool::instance()->closePool();
    }
};

// 测试连接池初始化
TEST_F(SqlConnPoolTest, Initialization) {
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 5);
}

// 测试获取连接
TEST_F(SqlConnPoolTest, GetConn) {
    MYSQL* conn = sql_pool::instance()->getConn();
    ASSERT_NE(conn, nullptr);
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 4);
    sql_pool::instance()->freeConn(conn);
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 5);
}


// 测试 RAII 封装
TEST_F(SqlConnPoolTest, RAII) {
    MYSQL* conn = nullptr;
    {
        conn_RAII raii(&conn, sql_pool::instance());
        ASSERT_NE(conn, nullptr);
        ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 4);
    }
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 5);
}

// 测试获取连接失败
TEST_F(SqlConnPoolTest, GetConnFail) {
    // 先拿出所有连接
    std::vector<MYSQL*> connections;
    for (int i = 0; i < 5; ++i) {
        connections.push_back(sql_pool::instance()->getConn());
    }
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 0);

    // 尝试再获取一个连接，应失败
    MYSQL* conn = sql_pool::instance()->getConn();
    ASSERT_EQ(conn, nullptr);

    // 释放所有连接
    for (auto conn : connections) {
        sql_pool::instance()->freeConn(conn);
    }
    ASSERT_EQ(sql_pool::instance()->getFreeConnCnt(), 5);
}

int main(int argc, char **argv) {
    log::getInstance()->init("./log", ".sql_pool_test_log", 1024);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
