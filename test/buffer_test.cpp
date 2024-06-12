#include <gtest/gtest.h>
#include "../code/buffer/buffer.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

class BufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试用例前的初始化代码
        test_buffer = new buffer(1024); // 初始化一个大小为1024的buffer
    }

    void TearDown() override {
        // 清理测试用例后的代码
        delete test_buffer;
    }

    buffer* test_buffer;
};

TEST_F(BufferTest, Initialization) {
    EXPECT_EQ(test_buffer->readableBytes(), 0);
    EXPECT_EQ(test_buffer->writablBytes(), 1024);
    EXPECT_EQ(test_buffer->prependableBytes(), 0);
}

TEST_F(BufferTest, AppendAndRetrieve) {
    const std::string data = "Hello, World!";
    test_buffer->append(data);
    EXPECT_EQ(test_buffer->readableBytes(), data.size());
    EXPECT_EQ(test_buffer->retrieveAllToStr(), data);
    EXPECT_EQ(test_buffer->readableBytes(), 0);
}

TEST_F(BufferTest, EnsureWritable) {
    const std::string data1 = "Hello, ";
    const std::string data2 = "World!";
    test_buffer->append(data1);
    test_buffer->append(data2);
    EXPECT_EQ(test_buffer->retrieveAllToStr(), "Hello, World!");
}

TEST_F(BufferTest, Retrieve) {
    const std::string data = "Hello, World!";
    test_buffer->append(data);
    test_buffer->retrieve(7); // retrieve "Hello, "
    EXPECT_EQ(test_buffer->readableBytes(), data.size() - 7);
    EXPECT_EQ(std::string(test_buffer->peek(), test_buffer->readableBytes()), "World!");
}

TEST_F(BufferTest, ReadFd) {
    int fds[2];
    ASSERT_EQ(pipe(fds), 0); // 创建管道
    const std::string data = "Hello, World!";
    write(fds[1], data.c_str(), data.size());
    close(fds[1]);

    int saveErrno;
    test_buffer->readFd(fds[0], &saveErrno);
    close(fds[0]);

    EXPECT_EQ(test_buffer->retrieveAllToStr(), data);
}

TEST_F(BufferTest, WriteFd) {
    int fds[2];
    ASSERT_EQ(pipe(fds), 0); // 创建管道
    const std::string data = "Hello, World!";
    test_buffer->append(data);

    int saveErrno;
    test_buffer->writeFd(fds[1], &saveErrno);
    close(fds[1]);

    char readBuff[1024];
    ssize_t n = read(fds[0], readBuff, sizeof(readBuff));
    close(fds[0]);

    EXPECT_EQ(std::string(readBuff, n), data);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
