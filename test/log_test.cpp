#include <gtest/gtest.h>
#include <fstream>
#include "../code/log/log.h"

log* test_log;
// 测试初始化日志模块
TEST(LogTest, InitLog) {
    ASSERT_TRUE(test_log->isOpen());
}

//测试日志写入
TEST(LogTest, WriteLog) {
    LOG_INFO("This is an info log");

    LOG_DEBUG("This is a debug log");

    LOG_WARN("This is a warning log");

    LOG_ERROR("This is an error log");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    /*NOTE:今天是六一！！！ */
    std::ifstream log_file("./log/2024_06_01.testlog");
    ASSERT_TRUE(log_file.is_open());

    std::string line;
    std::vector<std::string> log_lines;
    while (std::getline(log_file, line)) {
        log_lines.push_back(line);
    }
    log_file.close();

    ASSERT_GE(log_lines.size(), 4); // 至少有四条日志
    EXPECT_NE(log_lines[0].find("[info]:"), std::string::npos);
    EXPECT_NE(log_lines[1].find("[debug]:"), std::string::npos);
    EXPECT_NE(log_lines[2].find("[warn]:"), std::string::npos);
    EXPECT_NE(log_lines[3].find("[error]:"), std::string::npos);
}

// //测试日志文件切割
// TEST(LogTest, RotateLog) {
//     for (int i = 0; i < 30; ++i) {
//         LOG_INFO("Log line %d", i);
//     }
//     sleep(1);
//     std::ifstream log_file1("./log/2024_06_01.testlog");
//     ASSERT_TRUE(log_file1.is_open());
//     log_file1.close();

//     std::ifstream log_file2("./log/2024_06_01-1.testlog");
//     ASSERT_TRUE(log_file2.is_open());
//     log_file2.close();

//     std::ifstream log_file3("./log/2024_06_01-2.testlog");
//     ASSERT_TRUE(log_file3.is_open());
//     log_file3.close();
// }

// 测试异步写入
// TEST(LogTest, AsyncWriteLog) {
//     // 模拟多线程写日志
//     std::thread t1([] {
//         for (int i = 0; i < 5; ++i) {
//             LOG_INFO("Thread 1, log %d", i);
//             std::this_thread::sleep_for(std::chrono::seconds(2));
//         }
//     });

//     std::thread t2([] {
//         for (int i = 0; i < 5; ++i) {
//             LOG_DEBUG("Thread 2, log %d", i);
//             std::this_thread::sleep_for(std::chrono::seconds(2));
//         }
//     });

//     t1.join();
//     t2.join();
//     std::this_thread::sleep_for(std::chrono::seconds(1));

//     std::ifstream log_file("./log/2024_06_01.testlog");
//     ASSERT_TRUE(log_file.is_open());

//     std::string line;
//     std::vector<std::string> log_lines;
//     while (std::getline(log_file, line)) {
//         log_lines.push_back(line);
//     }
//     log_file.close();

//     ASSERT_GE(log_lines.size(), 10); // 至少有200条日志
// }



int main(int argc, char **argv) {
    test_log = log::getInstance();
    log::getInstance()->init("./log", ".testlog", 0);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();


}
