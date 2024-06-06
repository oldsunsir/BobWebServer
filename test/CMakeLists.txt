cmake_minimum_required(VERSION 3.10)
project(Test)

set(CMAKE_CXX_STANDARD 14)

# 添加Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 源文件
# block_queue_test
set(BQT_SOURCE_FILES ../../code/log/blockqueue.h block_queue_test.cpp)
add_executable(BlockQueueTest ${BQT_SOURCE_FILES})
target_link_libraries(BlockQueueTest ${GTEST_LIBRARIES} pthread)

# log_test
set(LOG_SOURCE_FILES ../../code/log/log.cpp ../../code/log/blockqueue.h log_test.cpp)
add_executable(LogTest ${LOG_SOURCE_FILES})   
target_include_directories(LogTest PUBLIC ${PROJECT_SOURCE_DIR}../code/log)
target_link_libraries(LogTest ${GTEST_LIBRARIES} pthread)

#timer_test
set(TIMER_SOURCE_FILES ../../code/timer/heaptimer.cpp heap_timer_test.cpp ../../code/log/log.cpp)
add_executable(TimerTest ${TIMER_SOURCE_FILES}) 
target_include_directories(TimerTest PUBLIC ${PROJECT_SOURCE_DIR}../code/log ${PROJECT_SOURCE_DIR}../code/timer)
target_link_libraries(TimerTest ${GTEST_LIBRARIES} pthread) 