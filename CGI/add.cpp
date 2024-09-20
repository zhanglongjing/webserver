#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main( int argc, char **argv ) {
    //argc 表示命令行参数的数量，argv 是指向命令行参数的指针数组。
    //argv[0] 是第一个命令行参数，它包含传递给程序的字符串。
    int a, b, accp_fd;
    sscanf( argv[0], "%d+%d,%d", &a, &b, &accp_fd );

    int t = a+b;
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\n\r\n";
    string body = "<html><head><title>niliushall's CGI</title></head>";
    body += "<body><p>The result is " + to_string(a) + "+" + to_string(b) + " = " + to_string(t);
    body += "</p></body></html>";
    response += body;
    //调用 dup2() 函数将 accp_fd（客户端的文件描述符）重定向到标准输出 STDOUT_FILENO。
    //这意味着接下来所有输出到 cout 的内容都会通过套接字发送到客户端。
    dup2(accp_fd, STDOUT_FILENO);
    //使用 cout 将 response 的内容发送给客户端。
    //由于前面使用了 dup2()，所以 cout 的输出会通过套接字发送到客户端，而不是输出到终端。
    cout << response.c_str();
    // send( accp_fd, response.c_str(), response.size(), 0 );
    return 0;
}