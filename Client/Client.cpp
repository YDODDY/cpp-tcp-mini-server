#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

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
    

    // [4byte Length]   [Payload]
    // [00][00][00][03] [A][A][A]

    // 4-byte length prefix
    std::string messages[] =
    {
        "AAA",
        "HELLO",
        "C"
    };

    for (const std::string& message : messages)
    {
        uint32_t messageLength = static_cast<uint32_t>(message.size());
        uint32_t networkLength = htonl(messageLength);
     
        int sentLengthBytes = send(clientSocket,
            reinterpret_cast<const char*>(&networkLength), sizeof(networkLength), 0);

        if (sentLengthBytes == SOCKET_ERROR)
        {
                std::cout<< "Length send failed for message: "
                << message
                << "\nWSA Error: "
                << WSAGetLastError()
                << '\n';
            break;
        }

        int sentPayloadBytes = send(clientSocket, message.c_str(), 
            static_cast<int>(message.size()), 0);
        if (sentPayloadBytes == SOCKET_ERROR)
        {
            std::cout << "payload send failed : " << WSAGetLastError() << '\n';
            break;
        }

        std::cout << "Sent message : " << message << " (" << messageLength << " bytes)\n";
    }


    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

