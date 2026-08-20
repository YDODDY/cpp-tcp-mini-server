#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

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

    // client 접속할 server 주소 만들기 
    // -> client 는 어디에 접속할 것인지 주소를 정확히 알아야 함 
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    // 127.0.0.1 -> loopback address : 이 컴퓨터 자기 자신 
    // 사람이 읽는 127.0.0.1 문자열을 socket 이 사용할 수 있는 주소 형태로 변환하는 함수 
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


    //send 

    const char* message = "ABC";
    // strlen 반환형이 size_t 인데 send 길이 인자는 int 니까 명시적으로 변환하는게 깔끔함
    int messageLength = static_cast<int>(strlen(message));

    // send 는 실패했을 때 SOCKET_ERROR 를 반환
    // 성공하면 실제 몇 byte 를 전송하도록 받아들였는지 값을 반환함 (처리된 byte 수)
    int sendBytes = send(clientSocket, message, messageLength, 0);
    // 실패 체크 
    if (sendBytes == SOCKET_ERROR)
    {
        std::cout << "send failed : " << WSAGetLastError() << '\n';
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Sent " << sendBytes << " bytes : " << message << '\n';

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

