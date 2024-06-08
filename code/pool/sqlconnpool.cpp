#include "sqlconnpool.h"

sql_pool* sql_pool::instance(){
    static sql_pool conn_pool;
    return &conn_pool;
}

void sql_pool::init(const char* host, int port,
                    const char* user, const char* pwd,
                    const char* db_name, int conn_size){

    assert(conn_size > 0);
    for (int i = 0; i < conn_size; ++i){
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);
        if (!conn){
            LOG_ERROR("Mysql init error")
            exit(1);
        }
        conn = mysql_real_connect(conn, host,
                                    user, pwd,
                                    db_name, port, nullptr, 0);
        if (!conn){
            LOG_ERROR("Mysql connect error")
            exit(1);
        }
        conn_que.push(conn);
    }
    max_conn = conn_size;
    sem_init(&sem_id, 0, conn_size);
}

MYSQL* sql_pool::getConn(){
    /*  NOTE:   这里不用queue的empty是觉得不是原子操作  */
    if (sem_trywait(&sem_id) != 0){
        LOG_WARN("Mysql busy, try later")
        return nullptr;
    }
    MYSQL* conn = nullptr;

    {
        std::lock_guard<std::mutex> locker(mtx);
        if (!conn_que.empty()){
            conn = conn_que.front();
            conn_que.pop();
        }
    }
    return conn;
}

void sql_pool::freeConn(MYSQL* conn){
    /*  NOTE:   注意特殊情况判断    */
    if (!conn){
        return;
    }
    std::lock_guard<std::mutex> locker(mtx);
    conn_que.push(conn);
    sem_post(&sem_id);
}

int sql_pool::getFreeConnCnt(){
    std::lock_guard<std::mutex> locker(mtx);
    return conn_que.size();
}

void sql_pool::closePool(){
    std::lock_guard<std::mutex> locker(mtx);
    while (!conn_que.empty()){
        MYSQL* tmp = conn_que.front();
        mysql_close(tmp);
        conn_que.pop();
    }
    mysql_library_end();
}

sql_pool::~sql_pool(){
    closePool();
}

conn_RAII::conn_RAII(MYSQL** conn, sql_pool* conn_pool){
    assert(conn_pool);
    *conn = conn_pool->getConn();
    con_RAII = *conn;
    conn_pool_RAII = conn_pool;
}
conn_RAII::~conn_RAII(){
    /*  NOTE:   虽然freeConn中也有判断, 但还是要严谨, 注意特判  */
    if (con_RAII)
        conn_pool_RAII->freeConn(con_RAII);
}