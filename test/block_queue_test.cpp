#include "../code/log/blockqueue.h"
#include <gtest/gtest.h>
#include <thread>


class BlockQueueTest : public ::testing::Test{
protected:
    void SetUp() override{
        queue = new block_queue<int>(10);
    }
    void TearDown() override{
        delete queue;
    }
public:
    block_queue<int>* queue;
};

// 测试队列的初始化状态
TEST_F(BlockQueueTest, Initialization) {
    EXPECT_TRUE(queue->empty());
    EXPECT_FALSE(queue->full());
    EXPECT_EQ(queue->size(), 0);
    EXPECT_EQ(queue->capacity(), 10);
}

// 测试队列的push和pop操作
TEST_F(BlockQueueTest, PushPop) {
    int item;
    EXPECT_TRUE(queue->push(1));
    EXPECT_FALSE(queue->empty());
    EXPECT_TRUE(queue->pop(item));
    EXPECT_EQ(item, 1);
    EXPECT_TRUE(queue->empty());
}

// 测试队列的容量限制
TEST_F(BlockQueueTest, CapacityLimit) {
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(queue->push(i));
    }
    EXPECT_TRUE(queue->full());
    // EXPECT_FALSE(queue->push(11));  // 队列已满，插入失败
}

// 测试超时pop操作
TEST_F(BlockQueueTest, PopTimeout) {
    int item;
    EXPECT_FALSE(queue->pop(item, 1));  // 队列为空，等待1秒后超时
}

// 测试关闭队列后的操作
TEST_F(BlockQueueTest, CloseQueue) {
    queue->close();
    EXPECT_FALSE(queue->push(1));
    int item;
    EXPECT_FALSE(queue->pop(item));
}
//多进程测试
TEST_F(BlockQueueTest, MultiplePushPop) {
    std::thread producer([this]() {
        for (int i = 0; i < 10; i += 2) {
            EXPECT_TRUE(queue->push(i));
            sleep(1);
        }
    });

    std::thread producer2([this](){
        for (int i = 1; i < 11; i += 2){
            EXPECT_TRUE(queue->push(i));
            sleep(1);
        }
    });
    std::thread consumer([this]() {
        int item;
        for (int i = 0; i < 10; ++i) {
            EXPECT_TRUE(queue->pop(item));
            EXPECT_EQ(item, i);
        }
    });

    producer.join();
    producer2.join();
    consumer.join();
}
int main(int argc, char* argv[]){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}