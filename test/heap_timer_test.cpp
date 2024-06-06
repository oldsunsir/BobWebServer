#include <gtest/gtest.h>
#include "../code/timer/heaptimer.h"
#include "../code/log/log.h"

log* test_log;
/*  BUG:    这里通过timer_heap test_heap;
            来声明不可以, 会被编译器认为是默认初始化    */
timer_heap test_heap(100);
std::vector<client_ptr> users;
time_out_cb test_cb = [](client_ptr user_data){
    LOG_INFO("user_data %d 进行回调函数", user_data->sockfd)
};

TEST(HeapTimerTest, InitHeap){
    ASSERT_TRUE(test_heap.empty());
    LOG_INFO("heap size is : %d", test_heap.cap())

}

TEST(HeapTimerTest, AddTimer){
    for (int i = 0; i < users.size(); ++i){
        timer_node_ptr timer = std::make_shared<timer_node>((i+1)%10 * 400);
        timer->cb_func = test_cb;
        timer->user_data = users[i];
        timer->user_data->timer = timer;
        test_heap.add_timer(timer);
        LOG_INFO("已添加第%d个timer_node", i+1);
    }
    EXPECT_EQ(test_heap.size(), 50);
    LOG_INFO("第一个timer的expires为:%d", log_out_expires(*(test_heap.top())));
}

TEST(HeapTimerTest, Tick){
    printf("当前为Tick测试\n");
    for(const auto& sp : users){
        printf("引用计数为:%ld\n", sp.use_count());
    }
    while (1){
        sleep(2);
        test_heap.tick();
        if (test_heap.empty()){
            LOG_INFO("heap为空, 退出tick")
            break;
        }
    }
    printf("当前为Tick测试心跳之后\n");
    for(const auto& sp : users){
        printf("引用计数为:%ld\n", sp.use_count());
    }
}

TEST(HeapTimerTest, DelTimer){

    timer_node_ptr timer1 = std::make_shared<timer_node>(15000);
    client_ptr client1 = std::make_shared<client_data>();
    timer1->user_data = client1;
    client1->timer = timer1;
    client1->sockfd = 1024;


    LOG_INFO("添加第一个timer")
    test_heap.add_timer(timer1);
    LOG_INFO("DelTimer测试, 目前第一个timer为%d", test_heap.top()->user_data->sockfd)


    timer_node_ptr timer2 = std::make_shared<timer_node>(5000);
    client_ptr client2 = std::make_shared<client_data>();
    timer2->user_data = client2;
    client2->timer = timer2;
    client2->sockfd = 2048;

    LOG_INFO("添加第二个timer")
    test_heap.add_timer(timer2);
    LOG_INFO("DelTimer测试, 目前第一个timer为%d", test_heap.top()->user_data->sockfd)
    
    timer_node_ptr timer3 = std::make_shared<timer_node>(10000);
    client_ptr client3 = std::make_shared<client_data>();
    timer3->user_data = client3;
    client3->timer = timer3;
    client3->sockfd = 3072;

    LOG_INFO("添加第三个timer")
    test_heap.add_timer(timer3);
    LOG_INFO("DelTimer测试, 目前第一个timer为%d", test_heap.top()->user_data->sockfd)

    test_heap.del_timer(client2->timer.lock());
    EXPECT_EQ(test_heap.size(), 2);
    LOG_INFO("DelTimer测试, 删除top后, 目前第一个timer为%d", test_heap.top()->user_data->sockfd);

    test_heap.pop_timer();
    LOG_INFO("DelTimer测试, 删除top后, 目前第一个timer为%d", test_heap.top()->user_data->sockfd);
}

TEST(HeapTimerTest, AdjustTimer){
    test_heap.adjust(test_heap.top(), 10000);
    LOG_INFO("Adjust测试, 第一次的的expires为:%d", log_out_expires(*(test_heap.top())));
    test_heap.adjust(test_heap.top(), 20000);
    LOG_INFO("Adjust测试, 第一次的的expires为:%d", log_out_expires(*(test_heap.top())));
}
int main(int argc, char **argv) {
    test_log = log::getInstance();
    log::getInstance()->init("./log", ".timer_test_log", 1024);

    // test_heap = timer_heap(10);
    for (int i = 0; i < 50; ++i){
        client_ptr tmp_client_ptr = std::make_shared<client_data>();
        tmp_client_ptr->sockfd = i;
        users.push_back(tmp_client_ptr);
    }
    printf("当前为初始化users\n");
    for(const auto& sp : users){
        printf("引用计数为:%ld\n", sp.use_count());
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

