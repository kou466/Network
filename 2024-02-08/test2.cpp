#include "lib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

string readHtmlFile(const string &path)
{
    ifstream file(path);
    stringstream buffer;
    if (file)
    {
        buffer << file.rdbuf();
        file.close();
        return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + buffer.str();
    }
    else
    {
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
    }
}

void handle_about(char *buf, size_t bufSize)
{
    string response = readHtmlFile("data/about.html");
    strncpy(buf, response.c_str(), bufSize);
    buf[bufSize - 1] = '\0'; // NULL 문자 추가
}

void handle_contact(char *buf, size_t bufSize)
{
    string response = readHtmlFile("data/contact.html");
    strncpy(buf, response.c_str(), bufSize);
    buf[bufSize - 1] = '\0';
}

void handle_index(char *buf, size_t bufSize)
{
    string response = readHtmlFile("data/index.html");
    strncpy(buf, response.c_str(), bufSize);
    buf[bufSize - 1] = '\0';
}

void handle_main(char *buf, size_t bufSize)
{
    string response = readHtmlFile("data/main.html");
    strncpy(buf, response.c_str(), bufSize);
    buf[bufSize - 1] = '\0';
}

void handle_test(char *buf, size_t bufSize)
{
    string response = readHtmlFile("data/test.html");
    strncpy(buf, response.c_str(), bufSize);
    buf[bufSize - 1] = '\0';
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
            buf[recvlen] = '\0';
            cout << "Recv: " << buf << endl;

            if (strstr(buf, "GET /index HTTP/1.1") != NULL)
            {
                handle_index(buf, sizeof(buf));
            }
            else if (strstr(buf, "GET /about HTTP/1.1") != NULL)
            {
                handle_about(buf, sizeof(buf));
            }
            else if (strstr(buf, "GET /contact HTTP/1.1") != NULL)
            {
                handle_contact(buf, sizeof(buf));
            }
            else if (strstr(buf, "GET /test HTTP/1.1") != NULL)
            {
                handle_test(buf, sizeof(buf));
            }
            else if (strstr(buf, "GET /main HTTP/1.1") != NULL)
            {
                handle_main(buf, sizeof(buf));
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