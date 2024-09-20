#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main( int argc, char **argv ) {
    //argc ��ʾ�����в�����������argv ��ָ�������в�����ָ�����顣
    //argv[0] �ǵ�һ�������в��������������ݸ�������ַ�����
    int a, b, accp_fd;
    sscanf( argv[0], "%d+%d,%d", &a, &b, &accp_fd );

    int t = a+b;
    string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=utf-8\r\n\r\n";
    string body = "<html><head><title>niliushall's CGI</title></head>";
    body += "<body><p>The result is " + to_string(a) + "+" + to_string(b) + " = " + to_string(t);
    body += "</p></body></html>";
    response += body;
    //���� dup2() ������ accp_fd���ͻ��˵��ļ����������ض��򵽱�׼��� STDOUT_FILENO��
    //����ζ�Ž�������������� cout �����ݶ���ͨ���׽��ַ��͵��ͻ��ˡ�
    dup2(accp_fd, STDOUT_FILENO);
    //ʹ�� cout �� response �����ݷ��͸��ͻ��ˡ�
    //����ǰ��ʹ���� dup2()������ cout �������ͨ���׽��ַ��͵��ͻ��ˣ�������������նˡ�
    cout << response.c_str();
    // send( accp_fd, response.c_str(), response.size(), 0 );
    return 0;
}