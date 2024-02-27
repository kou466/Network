#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <map>

#pragma comment(lib, "ws2_32.lib")

int main()
{ // 'void' 대신 'int' 반환 타입을 사용합니다.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Winsock error()" << std::endl;
        return -1; // 오류 시 -1 반환
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "socekt error()" << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1; // 오류 시 -1 반환
    }

    sockaddr_in serverHint;
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(55555); // 예제 포트 번호

    if (bind(serverSocket, (sockaddr *)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
    {
        std::cerr << "bind error()" << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1; // 오류 시 -1 반환
    }

    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];

    while (true)
    {
        ZeroMemory(&client, clientLength);
        ZeroMemory(buf, 1024);

        int bytesIn = recvfrom(serverSocket, buf, 1024, 0, (sockaddr *)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cerr << "receiving error()" << WSAGetLastError() << std::endl;
            continue; // 오류가 발생해도 서버는 계속 실행됩니다.
        }

        char clientIp[256];
        ZeroMemory(clientIp, 256);

        inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
        std::cout << "Message recv from " << clientIp << " : " << buf << std::endl;

        sendto(serverSocket, buf, bytesIn, 0, (sockaddr *)&client, clientLength);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0; // 정상 종료 시 0 반환
}
