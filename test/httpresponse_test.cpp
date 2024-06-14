#include "../code/buffer/buffer.h"
#include "../code/http/httpresponse.h"
#include "../code/log/log.h"
#include <gtest/gtest.h>

buffer test_buff;
http_response test_response;
int write_fd;
int m_errno;
TEST(HttpResponseTest, RightResponseTest){
    test_response.init("../../resources", "/welcome.html");
    LOG_DEBUG("test1 init success!")
    test_response.makeResponse(test_buff);
    test_buff.writeFd(write_fd, &m_errno);
}

TEST(HttpResponseTest, BadResponseTest){
    test_buff.retrieveAll();
    test_response.init("../../resources", "/welcome666.html");
    LOG_DEBUG("test2 init success!")
    test_response.addErrorContent(test_buff, "bad request");
    test_buff.writeFd(write_fd, &m_errno);
}
int main(int argc, char** argv){
    log::getInstance()->init("./log", "http_response_test.", 0);
    write_fd = open("test_http_response.txt", O_RDWR);
    if (write_fd < 0){
        LOG_ERROR("open write_fd failed");
        return 1;
    }

        
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    close(write_fd);
    return 0;
}