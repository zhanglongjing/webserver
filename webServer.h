#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include "task.h"
using namespace std;

const int max_event_num = 20;//表示epoll监听处理的最大事件数

//Web服务器功能
class WebServer {
private:
    int port;//服务器端口号
    int sock_fd;//服务器套接字文件描述符，用于网络通信。
    int epoll_fd;//epoll 的文件描述符，用于事件驱动的I/O多路复用。
    struct sockaddr_in server_addr;//服务器的地址结构体，包含IP地址和端口号等信息。
public:
    /*构造函数，初始化 sock_fd 为 0，设置端口号为传入参数 p。
    使用 memset 将 server_addr 结构体的所有字节设为 0，初始化地址结构体。*/
    WebServer( int p ) : sock_fd(0), port(p) { memset( &server_addr, 0, sizeof( server_addr ) ); }
    ~WebServer() { close( sock_fd ); }//析构函数，关闭服务器套接字文件描述符，释放资源。
    int run();//用于启动和运行服务器
};

/*使用 fcntl 函数获取文件描述符 fd 的当前文件状态标志，并存储在 old_option 中。
int new_option = old_option | O_NONBLOCK;：
将 O_NONBLOCK 标志添加到当前文件状态标志中，形成新的标志 new_option，使得文件描述符变为非阻塞。
fcntl(fd, F_SETFL, new_option);：
使用 fcntl 函数设置文件描述符 fd 的新文件状态标志为 new_option。
return old_option;：
返回之前的文件状态标志，便于后续可能的恢复操作。*/
int setnonblocking( int fd ) {
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;//将 O_NONBLOCK 标志添加到当前文件状态标志中，形成新的标志 new_option，使得文件描述符变为非阻塞
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

/*
定义一个函数 addfd，用于将文件描述符 fd 添加到 epoll 事件监听中。
*/
void addfd( int epoll_fd, bool oneshot, int fd ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;//设置事件类型为 EPOLLIN（可读事件）和 EPOLLET（边缘触发模式）
    if( oneshot ) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &event );//使用 epoll_ctl 函数将文件描述符 fd 添加到 epoll 实例中，监听指定的事件
    setnonblocking( fd );
}
/*
定义一个函数 removefd，用于从 epoll 事件监听中移除文件描述符 fd 并关闭它
*/
void removefd( int epollfd, int fd ) {
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

int WebServer::run() {
    server_addr.sin_family = AF_INET;//设置地址族为 AF_INET，表示使用 IPv4
    server_addr.sin_port = htons( port );//将端口号 port 转换为网络字节序（大端）并设置到 server_addr.sin_port
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );//将地址设置为 INADDR_ANY（即监听所有可用的网络接口），并转换为网络字节序

    sock_fd = socket( AF_INET, SOCK_STREAM, 0 );//创建一个TCP套接字，使用 IPv4 协议族 (AF_INET)，面向连接的流式套接字,服务器的
    if( sock_fd < 0 ) {
        cout << "socket error, line " << __LINE__ << endl;
        return -1;
    }
    //将套接字 sock_fd 绑定到之前设置的 server_addr 地址上
    int ret = bind( sock_fd, (struct sockaddr *)&server_addr, sizeof( server_addr ) );
    if( ret < 0 ) {
        cout << "bind error, line " << __LINE__ << endl;
        return -1;        
    }
    //将套接字 sock_fd 设置为被动监听状态，最多允许 5 个等待连接
    ret = listen( sock_fd, 5 );
    if( ret < 0 ) {
        cout << "listen error, line " << __LINE__ << endl;
        return -1;        
    }
    //创建一个 ThreadPool 对象，模板参数为 Task，线程池中包含 20 个线程
    ThreadPool<Task> threadpool(20);

    /* ThreadPool< Task > *threadpool;
    try {
        threadpool = new ThreadPool<Task>(20);
    } catch(...) {
        cout << "init threadpool error\n";
        return -1;
    } */
    //主循环
    while(1) {
        struct sockaddr_in client_addr;//存储客户端的地址信息。
        socklen_t client_addr_size = sizeof( client_addr );
        //调用 accept 函数接受一个新的客户端连接，返回新的连接文件描述符 conn_fd
        int conn_fd = accept( sock_fd, (struct sockaddr *)&client_addr, &client_addr_size );
        if( conn_fd < 0 ) {
            cout << "accept error, line: " << __LINE__ << endl;
            exit(-1);
        }
        Task *task = new Task(conn_fd);  // 不能使用Task task(conn_fd),
                                         // 需要使用new，否则造成线程函数还没运行完，Task就被析构
                                         // 在threadpool中，task结束后，进行delete
        threadpool.append( task );
    }
    return 0;
}

#endif