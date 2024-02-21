#include "lib.h"

#define DEFAULT_BUFLEN 1024
#define PORT 12345

using namespace std;

void recvMessage(SOCKET socket)
{
    char recvbuf[DEFAULT_BUFLEN] = {};
    DWORD recvBytes = 0, flags = 0;
    WSABUF recvWsabuf = {DEFAULT_BUFLEN, recvbuf};
    WSAOVERLAPPED recvOverlapped = {};
    WSAEVENT recvEvent = WSACreateEvent();
    recvOverlapped.hEvent = recvEvent;

    while (true)
    {
        ZeroMemory(&recvOverlapped, sizeof(recvOverlapped));
        recvOverlapped.hEvent = recvEvent;
        ZeroMemory(recvbuf, DEFAULT_BUFLEN);

        if (WSARecv(socket, &recvWsabuf, 1, &recvBytes, &flags, &recvOverlapped, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                cout << "recv() error" << WSAGetLastError() << endl;
                break;
            }
        }

        DWORD waitResult = WSAWaitForMultipleEvents(1, &recvEvent, TRUE, INFINITE, FALSE);
        if (waitResult == WAIT_FAILED)
        {
            cout << "Wait failed: " << WSAGetLastError() << endl;
            break;
        }

        WSAGetOverlappedResult(socket, &recvOverlapped, &recvBytes, FALSE, &flags);
        if (recvBytes > 0)
        {
            recvbuf[recvBytes] = '\0';
            cout << "Received: " << recvbuf << endl;
        }
        else
        {
            cout << "Server closed connection" << endl;
            break;
        }
    }

    WSACloseEvent(recvEvent);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (socket == INVALID_SOCKET)
    {
        cout << "socket() error" << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAddr.sin_port = htons(PORT);

    if (connect(socket, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        cout << "connect() error" << WSAGetLastError() << endl;
        closesocket(socket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server" << endl;

    thread recvThread(recvMessage, socket);
    recvThread.detach();

    char sendbuf[DEFAULT_BUFLEN] = {};
    while (true)
    {
        cout << "Enter: ";
        cin.getline(sendbuf, DEFAULT_BUFLEN);

        int sendBytes = send(socket, sendbuf, strlen(sendbuf), 0);
        if (sendBytes == SOCKET_ERROR)
        {
            cout << "send() error" << WSAGetLastError() << endl;
            break;
        }
    }

    closesocket(socket);
    WSACleanup();
    return 0;
}