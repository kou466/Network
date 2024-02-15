#include "lib.h"
void handle_root(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Root Page</h1>\r\n");
}

void handle_index(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Index Page</h1>\r\n");
}

void handle_about(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>About Page</h1>\r\n");
}

void handle_contact(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Contact Page</h1>\r\n");
}

void handle_test(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Test Page 1</h1>\r\n");
}

void handle_main(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Main Page 2</h1>\r\n");
}
int main()
{
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Non-blocking Socket
    SOCKET servsock = socket(AF_INET, SOCK_STREAM, 0);
    if (servsock == INVALID_SOCKET)
    {
        cout << "socket() error" << endl;
        return 0;
    }

    // 논블로킹 소켓으로 만들기
    u_long on = 1;
    if (ioctlsocket(servsock, FIONBIO, &on) == SOCKET_ERROR)
    {
        cout << "ioctlsocket() error" << endl;
        return 0;
    }

    SOCKADDR_IN servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12345);

    if (bind(servsock, (SOCKADDR *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
    {
        cout << "bind() error" << endl;
        return 0;
    }

    if (listen(servsock, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "listen() error" << endl;
        return 0;
    }

    while (true)
    {
        SOCKADDR_IN cliaddr;
        int addrlen = sizeof(cliaddr);
        SOCKET clisock = accept(servsock, (SOCKADDR *)&cliaddr, &addrlen);
        if (clisock == INVALID_SOCKET)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                Sleep(100);
                continue;
            }
            else
            {
                cout << "accept() error" << endl;
                return 0;
            }
        }

        cout << "Client Connected" << endl;

        char buf[1024] = "";
        int recvlen = recv(clisock, buf, sizeof(buf) - 1, 0);
        if (recvlen > 0)
        {
            buf[recvlen] = '\0'; // 성공적으로 데이터를 받았을 경우에만 처리
            cout << "Recv: " << buf << endl;

            if (strstr(buf, "GET / HTTP/1.1") != NULL)
            {
                handle_root(buf);
            }
            else if (strstr(buf, "GET /index HTTP/1.1") != NULL)
            {
                handle_index(buf);
            }
            else if (strstr(buf, "GET /about HTTP/1.1") != NULL)
            {
                handle_about(buf);
            }
            else if (strstr(buf, "GET /contact HTTP/1.1") != NULL)
            {
                handle_contact(buf);
            }
            else if (strstr(buf, "GET /test HTTP/1.1") != NULL)
            {
                handle_test(buf);
            }
            else if (strstr(buf, "GET /main HTTP/1.1") != NULL)
            {
                handle_main(buf);
            }

            send(clisock, buf, strlen(buf), 0);
        }
        else if (recvlen == 0 || (recvlen == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK))
        {
            cout << "Connection Error" << endl;
        }

        closesocket(clisock);
        cout << "Client Disconnected" << endl;
    }

    closesocket(servsock);
    WSACleanup();
    return 0;
}