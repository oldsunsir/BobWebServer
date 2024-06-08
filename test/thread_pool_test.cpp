// threadpool_test.cpp
#include <gtest/gtest.h>
#include "../code/pool/threadpool.h"
#include "../code/log/log.h"

void threadLogTask(int i, int cnt){
    for (int j = 0; j < 10000; ++j){
        LOG_INFO("thread %d PID:[%04d]======== %05d ======== ", i, gettid(), cnt++);
    }
}
TEST(threadPoolTest, initWithAdd){
    thread_pool test_pool;
    for (int i = 0; i < 16; ++i){
        test_pool.addTask(std::bind(threadLogTask, i, i*10000));
    }
    getchar();
}
int main(int argc, char **argv){
    log::getInstance()->init("./log", ".thread_pool_test_log", 1024);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}