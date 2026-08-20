

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	// winsock 초기화된 winsock 관련정보 담아둘 구조체
	WSADATA wsaData;

	// winsock 초기화 - winsock 2.2 버전 사용
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// WSAStartup 성공/실패 판단값을 반환함
	
	// 실패 체크
	if (result != 0)
	{
		std::cout << "WSAStartup failed : " << result << '\n';
		return 1;
	}
	std::cout << "Winsock initialized.\n";

	// listenSocket 생성 (주소체계 IPv4, Socket 형태 stream, 프로토콜 TCP)
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// socket() 실패 체크
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "socket failed : " << WSAGetLastError() << '\n';
		WSACleanup();
		return 1;
	}
	std::cout << "Socket created.\n";

	// 네트워크에는 숫자를 표현하는 표준 byte order 가 있다 
	// 예시로 숫자를 메모리에 저장할 때, 컴퓨터마다 여러 byte 로 된 숫자를 저장할 때 어느 byte 부터 놓는가에 대한 방식이 있을 수 있다. 
	// -> Little Endian : 낮은 자리 byte 부터 저장, Big Endian : 높은 자리 byte 부터 저장 
	// -> 컴퓨터 내부에서 숫자를 표현하는 byte 순서와 네트워크에서 약속한 byte 순서가 다를 수 있다. 
	// 근데 네트워크에서는 서로 다른 컴퓨터들이 통신하기 때문에, 표준 순서를 사용하도록 함 -> Network Byte Order

	// htons : host to network short, host->network 로 16bit(short)
	// htonl : host to network long, host->network 로 32bit(long)
	// host 가 사용하는 byte order 를 network byte order 로 변환한다
	// shot : TCP/UDP Port 번호가 16bit 값
	// long : IPv4 주소값은 32bit 값

	// 반대 기능 ntohs, ntohl

	// IPv4 socket 주소 정보 담는 구조체 
	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET; // IPv4 주소체계 사용
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // 이 머신의 사용 가능한 로컬 인터페이스로 들어오는 연결 받음 
	// 네트워크에는 숫자를 표현하는 표준 byte order 가 있다 
	// 예시로 숫자를 메모리에 저장할 때, 컴퓨터마다 여러 byte 로 된 숫자를 저장할 때 어느 byte 부터 놓는가에 대한 방식이 있을 수 있다. 
	// -> Little Endian : 낮은 자리 byte 부터 저장, Big Endian : 높은 자리 byte 부터 저장 
	// -> 컴퓨터 내부에서 숫자를 표현하는 byte 순서와 네트워크에서 약속한 byte 순서가 다를 수 있다. 
	// 근데 네트워크에서는 서로 다른 컴퓨터들이 통신하기 때문에, 표준 순서를 사용하도록 함 -> Network Byte Order
	
	// htons : host to network short, host->network 로 16bit(short)
	// htonl : host to network long, host->network 로 32bit(long)
	// host 가 사용하는 byte order 를 network byte order 로 변환한다
	// shot : TCP/UDP Port 번호가 16bit 값
	// long : IPv4 주소값은 32bit 값
	
	// 반대 기능 ntohs, ntohl

	serverAddress.sin_port = htons(5000); // 포트 번호, 5000 은 임시 
	// 포트번호 : 한 컴퓨터 안에서 어떤 네트워크 프로그램에게 데이터를 전달할지 구분하는 번호 
	// 포트번호의 범위는 16bit 라서 0~65535 (근데 0~1023 까지는 HTTP 같은 시스템/표준 서비스용 포트들도 있어서 이건 피하면 된다.. 일단 ㅇㅋ)
	// 무조건 비어있다고 보장되는 번호는 아님, 만약 다른 프로그램에서 이미 같은 주소의 port 5000 을 사용 중이라면 bind() 가 실패할 수 있음 

	// listenSocket 이 serverAddress 를 사용하도록 bind
	int bindResult = bind(listenSocket, 
		reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
	// bind 는 여러 종류의 주소 구조체를 받을 수 있게 공통형인 sockaddr* 을 요구함. 
	// -> IPv4 전용 sockaddr_in 을 쓰고 있으니까, 포인터 타입을 맞춰주는것 (cast)
	// 주소 구조체 크기를 알려주는 sizeof 까지 넘겨준다 

	// 실패 체크
	if (bindResult == SOCKET_ERROR)
	{
		std::cout << "bind failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Socket bound to port 5000.\n";

	// Client 연결 요청 받을 준비 - listen
	// SOMAXCONN : 연결 요청 대기열(backlog)을 시스템이 허용하는 적절한 최대값으로 사용하겠다
	int listenResult = listen(listenSocket, SOMAXCONN);
	
	// 실패 체크
	if (listenResult == SOCKET_ERROR)
	{
		std::cout << "listen failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Server listening on port 5000... \n";

	std::cout << "Waiting for client...\n";
	// client 하나가 접속할 때까지 기다리고, 접속하면 그 clinet 전용 socket 얻기 - accept
	// 뒤 nullptr 두개는 client 의 주소값, 주소값 사이즈 받아올 수 있는 공간
	// 근데 지금 작업에서는 굳이 받을 필요 없어서 nullptr 로 그냥 냅둠 -> clinet 주소 정보 안받는다
	SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

	// 실패 체크
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed : " << WSAGetLastError() << '\n';
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "Client connected!\n";

	// recv
	
	// recv 는 메모리 공간에 받은 데이터를 써넣어야 하기 때문에 const 아님 
	char buffer[512]{}; // 넉넉하게 512byte 정도까지 받을 수 있는 공간 
	int receivedBytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
	// 최대 512 까지 받게 해놓고 진짜 512byte 까지 꽉 채워 받으면 '\0' 버퍼 값까지 받을 수 없다
	// 511byte 까지만 받고, 안전하게 '\0' 받을 한 칸 남기기

	// receivedBytes > 0 : 실제로 받은 byte 수 
	// receivedBytes == 0 : 상대가 정상적으로 연결을 종료함 
	// receivedBytes == SOCKET_ERROR : recv 실패 

	// 실패 처리
	if (receivedBytes == SOCKET_ERROR)
	{
		std::cout
			<< "recv failed: " << WSAGetLastError() << '\n';

		closesocket(clientSocket);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (receivedBytes == 0)
	{
		std::cout << "Client disconnected.\n";
	}
	else 
	{
		buffer[receivedBytes] = '\0';
		std::cout << "Received " << receivedBytes << " bytes : "
			<< buffer << '\n';
	}

	// 네트워크 사용을 끝낼 때 자원 정리하는 종료 흐름 
	// socket 먼저 다 닫고 -> winsock 정리 
	
	// socket 도 운영체제가 관리하는 리소스라서 계속 열어둔 채로 프로그램이 길게 살아 있으면
	// 자원 누수나 연결 관리 문제가 생길 수 있음. 
	// 프로그램이 곧바로 종료된다면 OS 가 정리해주기는 하지만, 명시적으로 정리하는 습관이 있어야한다. 
	// 특히 나중에 Server 는 계속 켜저 있고, Client 가 수십번 들어왔다 나갔다 한다면, 그때끄때 닫아줘야 한다 

	// ERROR 코드 처리처럼 bind, send, recv 등의 각 단계에서 실패할 때에도 
	// 성공했던 단계에서 만든 자원은 다 정리하고 나가야 하기 때문에 이 원칙을 잘 지켜줘야 한다. 

	// 성공해서 확보한 자원은, 이후 어디서 실패하든 반드시 반납한다. 

	// 특정 socket 연결/자원을 더이상 사용하지 않겠다는 선언의 함수
	closesocket(clientSocket); // 특정 client 와의 연결을 끝낸다
	closesocket(listenSocket); // 새 client 연결을 더 이상 받지 않겠다 

	// 이 프로그램에서 winsock 사용 자체를 정리한다 
	WSACleanup();

	// Socket 은 closesocket() 으로 개별 정리
	// Winsock 전체 사용은 마지막에 WSACleanup() 
	// 중간에 실패해도 이미 만든 자원은 다 정리하고 종료하기 

	return 0;
}

