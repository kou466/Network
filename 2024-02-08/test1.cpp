#include "lib.h"
void handle_root(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Root Page</h1>");
}
void handle_index(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Index Page</h1>");
}

void handle_about(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>About Page</h1>");
}

void handle_contact(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Contact Page</h1>");
}

void handle_other_page_1(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Other Page 1</h1>");
}

void handle_other_page_2(char *buf)
{
    strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Other Page 2</h1>");
}
int main()
{
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Blocking Socket
    // accept (접속한 클라이언트가 있으면)
    // -> connect (클라이언트가 접속에 성공하면)
    // -> send/sendto (클라이언트/서버가 데이터를 송신버퍼에 복사하면)
    // -> recv/recvfrom (클라이언트/서버가 데이터를 수신버퍼에서 복사해옴)

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
            // 블로킹 일때는 바로 문제가 되지만,
            // cout << "accept() error" << endl;
            // return 0;

            // 논블로킹 일때는 한번 더 확인해야 한다
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
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

        while (true)
        {
            int recvlen;
            // 논블로킹 소켓은 send()도 루프를 돌면서 될 때까지 계속 시도해야함
            while (true)
            {
                recvlen = recv(clisock, buf, sizeof(buf), 0);
                if (recvlen == SOCKET_ERROR)
                {
                    // 논블로킹 소켓은 recv()에서 한번 더 체크해줘야함
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                    {
                        continue;
                    }
                    else
                    {
                        cout << "recv() error" << endl;
                        return 0;
                    }
                }
                else
                {
                    break;
                }
            }

            if (recvlen == 0)
            {                         // 클라이언트가 접속을 끊었으면
                closesocket(clisock); // 소켓을 닫고
                cout << "Client Disconnected" << endl;
                break; // 루프를 빠져나가서 다음 클라이언트를 받는다
            }

            buf[recvlen] = '\0';

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
            else if (strstr(buf, "GET /other_page_1 HTTP/1.1") != NULL)
            {
                handle_other_page_1(buf);
            }
            else if (strstr(buf, "GET /other_page_2 HTTP/1.1") != NULL)
            {
                handle_other_page_2(buf);
            }

            int sendlen;
            // 논블로킹 소켓은 send()도 루프를 돌면서 될 때까지 계속 시도해야함
            while (true)
            {
                sendlen = send(clisock, buf, strlen(buf) + 1, 0);
                if (sendlen == SOCKET_ERROR)
                {
                    // 논블로킹 소켓은 send()에서 한번 더 체크해줘야함
                    if (WSAGetLastError() == WSAEWOULDBLOCK)
                    {
                        continue;
                    }
                    else
                    {
                        cout << "send() error" << endl;
                        return 0;
                    }
                }
                else
                {
                    break;
                }
            }

            if (sendlen == 0)
            {                         // 클라이언트가 접속을 끊었으면
                closesocket(clisock); // 소켓을 닫고
                cout << "Client Disconnected" << endl;
                break; // 루프를 빠져나가서 다음 클라이언트를 받는다
            }
        }
    }

    closesocket(servsock);

    WSACleanup();
    return 0;
} // 논블로킹 + 동기 방식은 상당히 많은 루프를 필요로 하기 때문에 비효율적이다