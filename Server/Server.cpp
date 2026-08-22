

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include "C:\Users\USER\Desktop\Dev\cpp\tcpMiniServer\cpp-tcp-mini-server\Common\Protocol.h"


#pragma comment(lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;

	// winsock 초기화 - winsock 2.2 버전 사용
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	if (result != 0)
	{
		std::cout << "WSAStartup failed : " << result << '\n';
		return 1;
	}
	std::cout << "Winsock initialized.\n";

	// listenSocket 생성 (주소체계 IPv4, Socket 형태 stream, 프로토콜 TCP)
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "socket failed : " << WSAGetLastError() << '\n';
		WSACleanup();
		return 1;
	}
	std::cout << "Socket created.\n";

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET; // IPv4 주소체계 사용
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
	serverAddress.sin_port = htons(5000); // 포트 번호, 5000 은 임시 

	int bindResult = bind(listenSocket, 
		reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));

	if (bindResult == SOCKET_ERROR)
	{
		std::cout << "bind failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Socket bound to port 5000.\n";

	int listenResult = listen(listenSocket, SOMAXCONN);
	
	if (listenResult == SOCKET_ERROR)
	{
		std::cout << "listen failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Server listening on port 5000... \n";

	std::cout << "Waiting for client...\n";
	SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Client connected!\n";

	// 이전 recv 결과까지 전부 누적한 buffer
	std::vector<char> receiveBuffer;

	while (true)
	{
		// 이번 recv() 한 번의 결과 담고 있는 buffer
		char tempBuffer[512]{};

		int receivedBytes = recv(
			clientSocket,
			tempBuffer,
			sizeof(tempBuffer),
			0
		);

		if (receivedBytes == SOCKET_ERROR)
		{
			std::cout << "receive failed : " << WSAGetLastError() << '\n';
			break;
		}

		if (receivedBytes == 0)
		{
			std::cout << "Client disconnected.\n";
			break;
		}

		// tempBuffer 의 처음부터 receivedBytes 만큼을 receiveBuffer 뒤에 붙임 
		receiveBuffer.insert(receiveBuffer.end(),
			tempBuffer, tempBuffer + receivedBytes);

		while (true)
		{
			// length 4byte 도 다 안왔을 경우 다음 recv 기다림 
			if (receiveBuffer.size() < sizeof(uint32_t))
			{
				break;
			}

			// 앞 4byte-length 읽기 
			uint32_t networkLength;
			memcpy(&networkLength, receiveBuffer.data(), sizeof(uint32_t));
			// host byte order 로 변환해서 length 알아냄 
			uint32_t payloadLength = ntohl(networkLength);

			// 필요한 전체 크기 : 4byte + payload 
			size_t fullMessageSize = sizeof(uint32_t) + sizeof(MessageType) + payloadLength;
			// 필요한 byte 아직 다 못받았으면 다음 recv 기다리기 
			if (receiveBuffer.size() < fullMessageSize)
			{
				break;
			}

			// type 
			uint8_t rawType;
			memcpy(&rawType, receiveBuffer.data() + sizeof(uint32_t), sizeof(rawType));
			MessageType messageType = static_cast<MessageType>(rawType);

			// 완성시 payload 추출
			std::string payload(receiveBuffer.data() + sizeof(uint32_t) 
				+ sizeof(MessageType), payloadLength);
			std::cout << "Message received : " << payload << '\n';

			// 처리한 메세지는 receiveBuffer 에서 제거
			receiveBuffer.erase(
			receiveBuffer.begin(), receiveBuffer.begin() + fullMessageSize);

			switch (messageType)
			{
			case MessageType::Login:
				std::cout
					<< "[LOGIN] Player: "
					<< payload
					<< '\n';
				break;

			case MessageType::Chat:
				std::cout
					<< "[CHAT] "
					<< payload
					<< '\n';
				break;

			case MessageType::Move:
				std::cout
					<< "[MOVE] Position: "
					<< payload
					<< '\n';
				break;

			default:
				std::cout
					<< "[UNKNOWN MESSAGE TYPE]\n";
				break;
			}
		}
		
	}
	
	closesocket(clientSocket); 
	closesocket(listenSocket); 

	WSACleanup();

	return 0;
}

