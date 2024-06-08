/*TODO: 实现数据库连接池
        单例模式, RAII机制获取连接  */

#ifndef SQL_CONN_POOL_H
#define SQL_CONN_POOL_H

#include <queue>
#include <mysql/mysql.h>
#include <mutex>
#include <semaphore.h>
#include <assert.h>
#include "../log/log.h"
class sql_pool{

public:
    static sql_pool* instance();

    void init  (const char* host, int port,
                const char* user, const char* pwd,
                const char* db_name, int conn_size);
    MYSQL* getConn();
    void freeConn(MYSQL* conn);
    int getFreeConnCnt();

    void closePool();
private:
    sql_pool() = default;
    ~sql_pool();

    sql_pool& operator=(const sql_pool&) = delete;
    sql_pool(const sql_pool&) = delete;

    int max_conn;   

    std::queue<MYSQL*> conn_que;
    std::mutex mtx;
    sem_t sem_id;
};

//NOTE: RAII机制, 通过这类建立的连接, 不再需要自己手动释放
class conn_RAII{
public:
    conn_RAII(MYSQL** conn, sql_pool* conn_pool);
    ~conn_RAII();
private:
    MYSQL* con_RAII;
    sql_pool* conn_pool_RAII;
};



#endif