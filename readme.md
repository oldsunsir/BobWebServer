## 功能
* 利用EPOLL与线程池实现多线程的Reactor高并发模型, 主线程负责监听与建立连接, 子线程负责IO与逻辑处理
* 利用正则表达式与有限状态机解析HTTP中GET与POST请求, 处理静态资源
* 基于vector封装char, 实现自动增长的缓冲区
* 基于小根堆实现定时器, 关闭超时连接
* 基于生产者消费者模型, 实现异步日志系统
* 实现数据库连接池, 通过信号量实现同步, 并利用RAII机制封装, 自动返回处理完的连接
* 基于googleTest实现各模块对应的单测, 基于CMake构建整个项目


## 目录树
```
.
├── code           源代码
│   ├── buffer
│   ├── http
│   ├── log
│   ├── pool
│   ├── server
│   ├── timer
│   ├── main.cpp
│   └── CMakeLists.txt
├── test           单元测试
│   ├── build
│   ├── *.cpp
│   └── CMakeLists.txt
├── resources      静态资源
│   ├── index.html
│   ├── image
│   ├── video
│   ├── js
│   └── css
├── build          
│   └── log        日志文件
├── .gitignore
└── readme.md
```

## 项目启动
```bash
cd build
cmake ..
make
./Server
```


## TODO
* LOG添加级别设置, 在只需要输出info时不打印其他信息, 去除冗余日志
* 高并发下仍有逻辑bug, dmesg查看段错误显示访问了不存在的地址, 问题发生在解析http_request请求,
    未解析完但read_buff已被清空, 应该是并发时访问变量冲突导致

## 致谢
Linux高性能服务器编程，游双著.
[markparticle](https://github.com/markparticle/WebServer)