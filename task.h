#ifndef _TASK_H_
#define _TASK_H_

#include "threadPool.h"
#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
using namespace std;

const int buffer_size = 1024;//缓冲区1024字节

class Task {
private:
    int accp_fd;// 客户端连接的文件描述符，用于通信

public:
    Task() {}
    Task(int fd) : accp_fd(fd) {}//初始化 Task 对象，并将文件描述符 fd 赋值给成员变量 accp_fd，用于通信。
    ~Task() { close( accp_fd ); }

    void doit();//处理来自客户端的 HTTP 请求
    const string construct_header( const int num, const string & info,
                                        const string & type );//该函数根据状态码、信息和内容类型构造 HTTP 响应头并返回字符串
    //函数根据文件名和文件类型构造响应，并通过文件描述符发送文件内容。默认状态码为 200，信息为 "OK"
    int send_file( const string & filename, const string & type, 
                        const int num = 200, const string & info = "OK" );
    int deal_get( const string & uri );
    int deal_post( const string & uri, char *buf );
    int get_size( const string & filename );
};

void Task::doit() {
    char buf[ buffer_size ] = {0};
    //recv() 函数接收来自客户端的数据，返回接收到的字节数
    //如果返回 0 表示客户端关闭连接，返回负数表示出错
    while( int r = recv( accp_fd, buf, 1024, 0 ) ) {
// cout << "r = " << r << endl;
// sleep(1);
        if( !r ) {
            cout << "browser exit.\n";
            // removefd( epoll_fd, accp_fd );
            return;
        } else if( r < 0 ) {  //如果接收出错则继续接收数据
            continue;
        }

        string method, uri, version;//用于存储 HTTP 请求的请求方法、请求 URI 和协议版本。
        stringstream ss;
        // ss.clear();
        ss << buf;
        ss >> method >> uri >> version;

        cout << "method = " << method << endl;
        cout << "uri = " << uri << endl;
        cout << "version = " << version << endl << endl;

        // cout << "request = ------\n" << buf << "\n------------" << endl;

        // sleep(1);

        if( method == "GET" ) {  //为GET
            deal_get( uri );
        } else if( method == "POST" ) {
            deal_post( uri, buf );
        } else {
            string header = construct_header( 501, "Not Implemented", "text/plain" );
            send( accp_fd, header.c_str(), header.size(), 0 );
        }
        break;  // 只要处理完就退出循环，避免浏览器一直处于pending状态
    }
    // close( accp_fd );  // 任务完成直接close，不能在析构函数close(如果不delete task的话，
                          // 不delete task不够条用析构函数)
}

const string Task::construct_header( const int num, 
                        const string & info, const string & type ) {
    string response = "HTTP/1.1 " + to_string(num) + " " + info + "\r\n";
    response += "Server: longjing\r\nContent-Type: " + type + ";charset=utf-8\r\n\r\n";
    return response;
}

int Task::deal_get( const string & uri ) {
    string filename = uri.substr(1);

    if( uri == "/" || uri == "/index.html" ) {
        send_file( "index.html", "text/html" );
    } else if( uri.find( ".jpg" ) != string::npos || uri.find( ".png" ) != string::npos ) {
        send_file( filename, "image/jpg" );
    } else if( uri.find( ".html" ) != string::npos ) {
        send_file( filename, "text/html" );
    } else if( uri.find( ".ico" ) != string::npos ) {
        send_file( filename, "image/x-icon" );
    } else if( uri.find( ".js" ) != string::npos ) {
        send_file( filename, "yexy/javascript" );
    } else if( uri.find( ".css" ) != string::npos ) {
        send_file( filename, "text/css" );
    } else if( uri.find( ".mp3" ) != string::npos ) {
        send_file( filename, "audio/mp3" );
    } else {
        send_file( "404.html", "text/html", 404, "Not Found" );
    }
}

int Task::deal_post( const string & uri, char *buf ) {
    string filename = uri.substr(1);
    if( uri.find( "adder" ) != string::npos ) {
        char *tmp = buf;
        int len, a, b;
        char *l = strstr( tmp, "Content-Length:" );
        sscanf( l, "Content-Length: %d", &len );
        len = strlen( tmp ) - len;
        tmp += len;
        sscanf( tmp, "a=%d&b=%d", &a, &b );
        sprintf(tmp, "%d+%d,%d", a, b, accp_fd);
        if( fork() == 0 ) {
            // dup2( accp_fd, STDOUT_FILENO );
            execl( filename.c_str(), tmp, NULL );
        }
        wait( NULL );
    } else {
        //
    }
}

int Task::send_file( const string & filename, const string & type, 
                                const int num, const string & info ) {
    string header = construct_header( num, info, type );

    // send第二个参数只能是c类型字符串，不能使用string
    send( accp_fd, header.c_str(), header.size(), 0 );
    /*
    定义 stat 结构体 filestat，并调用 stat() 函数获取文件的状态信息（如大小、权限等）
    stat() 成功时返回 0，filestat 会被填充文件的元数据。
    */
    struct stat filestat;
    int ret = stat( filename.c_str(), &filestat );
    if( ret < 0 || !S_ISREG( filestat.st_mode ) ) {
        cout << "file not found : " << filename << endl;
        send_file( "404.html", "text/html", 404, "Not Found" );
        return -1;
    }

    if( !filename.empty() ) {
        int fd = open( filename.c_str(), O_RDONLY );
        sendfile( accp_fd, fd, NULL, get_size( filename ) );
        close( fd );
    }
cout << filename << " send finish to " << accp_fd << endl;
    return 0;
}

int Task::get_size( const string & filename ) {
    struct stat filestat;
    int ret = stat( filename.c_str(), &filestat );
    if( ret < 0 ) {
        cout << "file not found : " << filename << endl;
        return 0;
    }
    return filestat.st_size;
}

#endif