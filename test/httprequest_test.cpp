#include <gtest/gtest.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "../code/buffer/buffer.h"
#include "../code/pool/sqlconnpool.h"
#include "../code/http/httprequest.h"

http_request test_http;
buffer test_buffer;
int fd = open("../http_request.txt", O_RDONLY);

// 测试解析
TEST(HttpRequestTest, ParseTest) {
    LOG_INFO("test_buffer size is :%d", test_buffer.readableBytes())
    ASSERT_TRUE(test_http.parse(test_buffer));
    // 验证解析结果
    ASSERT_EQ(test_http.method(), "POST");
    ASSERT_EQ(test_http.path(), "/welcome.html");
    ASSERT_EQ(test_http.version(), "1.1");

    ASSERT_EQ(test_http.getPost("username"), "admin");
    ASSERT_EQ(test_http.getPost("password"), "123456");
}

int main(int argc, char **argv) {
    log::getInstance()->init("./log", ".http_request_test_log", 0);
    sql_pool::instance()->init("localhost", 3306, "test_user", "test_password", "test_db", 5);
    if (fd == -1){
        std::cerr << "Failed to open file"  << std::endl;
        return 1;
    }
    int errno_value;
    test_buffer.readFd(fd, &errno_value);
    ::testing::InitGoogleTest(&argc, argv);
    close(fd);
    return RUN_ALL_TESTS();
}