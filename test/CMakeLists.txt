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

#sql_pool_test
set(SQL_POOL_SOURCE_FILES ../../code/log/log.cpp ../../code/pool/sqlconnpool.cpp   sql_pool_test.cpp)
add_executable(SqlPoolTest ${SQL_POOL_SOURCE_FILES})
target_include_directories(SqlPoolTest PUBLIC ${PROJECT_SOURCE_DIR}../code/log ${PROJECT_SOURCE_DIR}../code/pool)
target_link_libraries(SqlPoolTest ${GTEST_LIBRARIES} pthread mysqlclient) 

#thread_pool_test
set(THREAD_POOL_SOURCE_FILES ../../code/log/log.cpp ../../code/pool/threadpool.h thread_pool_test.cpp)
add_executable(ThreadPoolTest ${THREAD_POOL_SOURCE_FILES})  
target_include_directories(ThreadPoolTest PUBLIC ${PROJECT_SOURCE_DIR}../code/log ${PROJECT_SOURCE_DIR}../code/pool)
target_link_libraries(ThreadPoolTest ${GTEST_LIBRARIES} pthread)

#buffer_test
set(BUFFER_SOURCE_FILES ../../code/buffer/buffer.cpp buffer_test.cpp)
add_executable(BufferTest ${BUFFER_SOURCE_FILES})
target_include_directories(BufferTest PUBLIC ${PROJECT_SOURCE_DIR}../code/buffer)
target_link_libraries(BufferTest ${GTEST_LIBRARIES})

#http_request_test
set(HTTP_REQUEST_SOURCE_FILES   ../../code/http/httprequest.cpp
                                ../../code/buffer/buffer.cpp
                                ../../code/log/log.cpp
                                ../../code/pool/sqlconnpool.cpp
                                httprequest_test.cpp )
add_executable(HttpRequestTest ${HTTP_REQUEST_SOURCE_FILES})
target_include_directories(HttpRequestTest PUBLIC ${PROJECT_SOURCE_DIR}../code)
target_link_libraries(HttpRequestTest ${GTEST_LIBRARIES} pthread mysqlclient)