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
        memset(buf, 0, sizeof(buf)); // 버퍼 초기화

        bool client_disconnected = false; // 클라이언트가 접속을 끊었는지 여부

        while (true)
        {
            int recvlen;
            while (true)
            {
                recvlen = recv(clisock, buf, sizeof(buf) - 1, 0);
                if (recvlen == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                    {
                        Sleep(100);
                        continue;
                    }
                    else
                    {
                        cout << "recv() error" << endl;
                        return 0;
                    }
                }
                else if (recvlen == 0)
                {
                    client_disconnected = true;
                    cout << "Client Disconnected" << endl;
                    break;
                }
            }

            buf[recvlen] = '\0';

            // http 요청 처리
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

            int sendlen;
            int totalBytes = strlen(buf); // 전체 보내야 할 바이트 수
            int sentBytes = 0;            // 현재까지 보낸 바이트 수

            while (sentBytes < totalBytes)
            {
                sendlen = send(clisock, buf + sentBytes, totalBytes - sentBytes, 0);

                if (sendlen == SOCKET_ERROR)
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                    {
                        Sleep(100);
                        continue;
                    }
                    else
                    {
                        cout << "send() error" << endl;
                        closesocket(clisock);
                        return 0;
                    }
                }
                else if (sendlen > 0)
                {
                    sentBytes += sendlen;
                }
                else
                {
                    break;
                }
            }

            if (sentBytes == totalBytes)
            {
                cout << "Data Success" << endl;
            }

            closesocket(clisock);
            cout << "Client Disconnected" << endl;
        }
    }

    closesocket(servsock);

    WSACleanup();
    return 0;
}