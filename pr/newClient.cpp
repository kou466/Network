#include "lib.h"

using namespace std;

char *getIPonDNS(string domain);

void getTargetIP(char *buf, string &ip, string &port, bool isLocalNet);

void processServerResponse(char *buffer, string &outIp, string &outPort);

int main()
{
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    string server;
    cout << "Input server IP or domain: ";
    cin >> server;

    char *ip = getIPonDNS(server);

    SOCKET clisock = socket(AF_INET, SOCK_DGRAM, 0);
    if (clisock == INVALID_SOCKET)
    {
        cout << "socket() error" << endl;
        return 0;
    }

    SOCKADDR_IN servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(ip);
    servAddr.sin_port = htons(3478);

    if (connect(clisock, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        cout << "connect() error" << endl;
        closesocket(clisock);
        return 0;
    }

    SOCKADDR_IN localAddr;
    int addrSize = sizeof(localAddr);
    if (getsockname(clisock, (SOCKADDR *)&localAddr, &addrSize) == SOCKET_ERROR)
    {
        cout << "getsockname() error" << endl;
        closesocket(clisock);
        return 0;
    }

    char buf[1024];
    sprintf(buf, "%s:%d", inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));

    int sendlen = sendto(clisock, buf, strlen(buf) + 1, 0, (SOCKADDR *)&servAddr, sizeof(servAddr));
    if (sendlen == SOCKET_ERROR)
    {
        cout << "sendto() error" << endl;
        return 0;
    }

    int recvlen = recvfrom(clisock, buf, sizeof(buf), 0, NULL, NULL);
    if (recvlen == SOCKET_ERROR)
    {
        cout << "recvfrom() error" << endl;
        return 0;
    }

    cout << "Received from STUN server: " << buf << endl;

    bool isLocalNet = false; // 로컬 넷(같은 NAT)에서 홀펀칭을 하려면 true로 변경
    string destIP, destPort;
    getTargetIP(buf, destIP, destPort, isLocalNet);

    cout << "Destination IP: " << destIP << ":" << destPort << endl;

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(destIP.c_str());
    servAddr.sin_port = htons(atoi(destPort.c_str()));

    if (connect(clisock, (sockaddr *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        cout << "connect() error" << endl;
        closesocket(clisock);
        return 0;
    }

    while (true)
    {
        string message = "send from windows";

        int sendlen = sendto(clisock, message.c_str(), message.length() + 1, 0, (sockaddr *)&servAddr, sizeof(servAddr));
        if (sendlen == SOCKET_ERROR)
        {
            cout << "sendto() error" << endl;
            return 0;
        }

        int recvlen = recvfrom(clisock, buf, sizeof(buf), 0, NULL, NULL);
        if (recvlen == SOCKET_ERROR)
        {
            cout << "recvfrom() error" << endl;
            return 0;
        }

        cout << "Echo: " << buf << endl;

        this_thread::sleep_for(100ms);
    }

    closesocket(clisock);
    WSACleanup();
    return 0;
}

char *getIPonDNS(string domain)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct hostent *host;
    char *ip = new char[16];
    host = gethostbyname(domain.c_str());
    if (!host)
    {
        cout << "gethostbyname() error" << endl;
        return nullptr;
    }
    strcpy(ip, inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
    WSACleanup();
    return ip;
}

void getTargetIP(char *buf, string &ip, string &port, bool isLocalNet)
{
    string response = buf;
    size_t ipStart = response.find("is ") + 3;
    size_t ipEnd = response.find(':', ipStart);
    ip = response.substr(ipStart, ipEnd - ipStart);
    port = response.substr(ipEnd + 1);
}

void processServerResponse(char *buffer, string &outIp, string &outPort)
{
    string response = buffer;
    vector<string> clientInfos;
    stringstream ss(response);
    string item;
    while (getline(ss, item, ';'))
    {
        clientInfos.push_back(item);
    }

    if (!clientInfos.empty())
    {
        string firstClientInfo = clientInfos[0];
        size_t pos = firstClientInfo.find(":");
        if (pos != string::npos)
        {
            outIp = firstClientInfo.substr(0, pos);
            outPort = firstClientInfo.substr(pos + 1);
        }
    }
}