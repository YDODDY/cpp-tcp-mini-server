#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <string>
#include "C:\Users\USER\Desktop\Dev\cpp\tcpMiniServer\cpp-tcp-mini-server\Common\Protocol.h"

#pragma comment(lib, "Ws2_32.lib")

struct TestMessage
{
    MessageType type;
    std::string payload;
};

TestMessage messages[] =
{
    {MessageType::Login, "schekk"},
    {MessageType::Chat, "Hello!"},
    {MessageType::Move, "100,200"}
};

bool SendMessage(SOCKET clientSocket, MessageType type, const std::string& payload)
{
    uint32_t payloadLength =
        static_cast<uint32_t>(payload.size());
    uint32_t networkLength = htonl(payloadLength);

    // length
    int sentLengthBytes = send(clientSocket,
        reinterpret_cast<const char*>(&networkLength),
        sizeof(networkLength), 0);

    if (sentLengthBytes == SOCKET_ERROR)
    {
        std::cout
            << "Length send failed: "
            << WSAGetLastError()
            << '\n';
        return false;
    }

    // type
    int sentTypeBytes = send(clientSocket,
        reinterpret_cast<const char*>(&type), sizeof(type), 0);

    if (sentTypeBytes == SOCKET_ERROR)
    {
        std::cout
            << "Type send failed: "
            << WSAGetLastError()
            << '\n';
        return false;
    }

    // payload
    int sentPayloadBytes = send(clientSocket,
        payload.c_str(), static_cast<int>(payload.size()), 0);

    if (sentPayloadBytes == SOCKET_ERROR)
    {
        std::cout
            << "Payload send failed: "
            << WSAGetLastError()
            << '\n';
        return false;
    }

    return true;
}

int main()
{
    // winsock 초기화
    WSADATA wsaData;

    int result = WSAStartup(
        MAKEWORD(2, 2),
        &wsaData
    );

    if (result != 0)
    {
        std::cout
            << "WSAStartup failed: "
            << result
            << '\n';

        return 1;
    }

    std::cout << "Winsock initialized.\n";

    // client socket 생성
    SOCKET clientSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );

    if (clientSocket == INVALID_SOCKET)
    {
        std::cout
            << "socket failed: "
            << WSAGetLastError()
            << '\n';

        WSACleanup();
        return 1;
    }

    std::cout << "Client socket created.\n";

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);

    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // connect 
    int connectResult = connect(clientSocket, 
        reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
    // 실패 체크
    if (connectResult == SOCKET_ERROR)
    {
        std::cout << "connect failed : " << WSAGetLastError() << '\n';
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to server!\n";

    if (!SendMessage(clientSocket, MessageType::Login, "schekk"))
    {
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (!SendMessage(clientSocket, MessageType::Chat, "hi?"))
    {
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    
    if (!SendMessage(clientSocket, MessageType::Move, "100,200,-30"))
    {
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

  


    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

