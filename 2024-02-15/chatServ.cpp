#include "lib.h"

#define DEFAULT_BUFLEN 1024
#define MAX_THREAD 8
#define PORT 12345

using namespace std;

struct Session
{
    SOCKET sock = INVALID_SOCKET;
    char buf[DEFAULT_BUFLEN] = {};
    WSAOVERLAPPED readOverLapped = {};
    WSAOVERLAPPED writeOverLapped = {};

    Session() {}
    Session(SOCKET sock) : sock(sock) {}
};

atomic<bool> TPoolRunning = true;
vector<Session *> sessions;
MemoryPool *MemPool = new MemoryPool(sizeof(Session), 1000);

void WorkerThread(HANDLE iocpHd)
{
    DWORD bytesTransferred;
    Session *session;
    LPOVERLAPPED overlapped;
    WSABUF wsaBuf;
    DWORD flags = 0;

    while (TPoolRunning)
    {
        BOOL ret = GetQueuedCompletionStatus(iocpHd, &bytesTransferred, (PULONG_PTR)&session, &overlapped, INFINITE);
        if (!ret || bytesTransferred == 0)
        {
            if (session != nullptr)
            {
                cout << "Client disconnected" << endl;
                closesocket(session->sock);
                MemPool_delete(*MemPool, session);
                continue;
            }
        }

        if (overlapped == &session->readOverLapped)
        {
            for (auto target : sessions)
            {
                if (target->sock != INVALID_SOCKET && target != session)
                {
                    wsaBuf.buf = session->buf;
                    wsaBuf.len = bytesTransferred;

                    DWORD sendBytes;
                    WSASend(target->sock, &wsaBuf, 1, &sendBytes, 0, &target->writeOverLapped, NULL);
                }
            }
            ZeroMemory(&session->readOverLapped, sizeof(WSAOVERLAPPED));
            wsaBuf.buf = session->buf;
            wsaBuf.len = DEFAULT_BUFLEN;
            WSARecv(session->sock, &wsaBuf, 1, NULL, &flags, &session->readOverLapped, NULL);
        }
        else if (overlapped == &session->writeOverLapped)
        {
            cout << "Data sent successfully to client." << endl;
        }
    }
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET servSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (servSock == INVALID_SOCKET)
    {
        cout << "WSASocket() error" << endl;
        return 1;
    }

    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    if (bind(servSock, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        cout << "bind() error" << endl;
        return 1;
    }

    if (listen(servSock, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "listen() error" << endl;
        return 1;
    }

    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort((HANDLE)servSock, iocp, 0, 0);

    for (int i = 0; i < MAX_THREAD; i++)
    {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkerThread, iocp, 0, NULL);
    }

    while (true)
    {
        SOCKET clientSock = accept(servSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                cout << "accept() error" << endl;
                return 1;
            }
            continue;
        }

        Session *session = MemPool_new<Session>(*MemPool, clientSock);
        sessions.push_back(session);
        cout << "Client connected" << endl;
        CreateIoCompletionPort((HANDLE)clientSock, iocp, (ULONG_PTR)session, 0);

        ZeroMemory(&session->readOverLapped, sizeof(WSAOVERLAPPED));
        WSABUF wsaBuf = {DEFAULT_BUFLEN, session->buf};

        DWORD recvBytes = 0;
        DWORD flags = 0;

        WSARecv(clientSock, &wsaBuf, 1, NULL, &flags, &session->readOverLapped, NULL);
    }

    TPoolRunning = false;
    for (auto session : sessions)
    {
        closesocket(session->sock);
        MemPool_delete(*MemPool, session);
    }
    CloseHandle(iocp);
    closesocket(servSock);
    WSACleanup();

    return 0;
}