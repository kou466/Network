#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Can't initialize winsock! Quitting" << std::endl;
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Can't create a socket, Err #" << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    std::string serverIp;
    std::cout << "Enter server IP or domain name: ";
    std::getline(std::cin, serverIp);

    int serverPort = 55555; // 서버 포트 번호
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &server.sin_addr);

    std::string msg;
    std::cout << "Enter message: ";
    std::getline(std::cin, msg);

    int sendOk = sendto(clientSocket, msg.c_str(), msg.size() + 1, 0, (sockaddr *)&server, sizeof(server));
    if (sendOk == SOCKET_ERROR)
    {
        std::cerr << "Can't send msg, Err #" << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Optional: Receive echo from server
    sockaddr_in from;
    int fromSize = sizeof(from);
    char buf[1024];
    ZeroMemory(buf, 1024);

    int bytesIn = recvfrom(clientSocket, buf, 1024, 0, (sockaddr *)&from, &fromSize);
    if (bytesIn == SOCKET_ERROR)
    {
        std::cerr << "Error receiving from server, Err #" << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "Server says: " << buf << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
