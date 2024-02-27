#include "lib.h"

using namespace std;

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "socket() error" << endl;
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(3478);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "bind() error" << endl;
        return 1;
    }

    cout << "Server is running on port 3478" << endl;

    char buf[1024];
    map<string, sockaddr_in> clientMap;

    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        int recvlen = recvfrom(serverSocket, buf, sizeof(buf), 0, (sockaddr *)&clientAddr, &clientAddrLen);
        if (recvlen == SOCKET_ERROR)
        {
            cerr << "recvfrom() error" << endl;
            return 1;
        }

        string clientMessage(buf, recvlen);
        string clientId = inet_ntoa(clientAddr.sin_addr) + ":" + to_string(ntohs(clientAddr.sin_port));
        clientMap[clientId] = clientAddr;

        stringstream ss;
        for (auto &[newId, newAddr] : clientMap)
        {
            if (newId != clientId)
            {
                ss << newId << " ";
            }
        }

        string clientList = ss.str();
        sendto(serverSocket, clientList.c_str(), clientList.length() + 1, 0, (sockaddr *)&clientAddr, sizeof(clientAddr));

        // string clientResponse = "Your public IP:Port is " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string(ntohs(clientAddr.sin_port));
        // sendto(serverSocket, clientResponse.c_str(), clientResponse.length() + 1, 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}