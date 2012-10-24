#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <winsock2.h>

using namespace std;

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		cerr << "Failure to find WinSock 2.2 or better." << endl;
		return EXIT_FAILURE;
	}

	SOCKET sock;
	sockaddr_in serverAddr;

	int addr_len, bytes_read;
	char recv_data[1024];
	sockaddr_in server_addr , client_addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		cerr << "Error connecting to socket! " << endl;
		return EXIT_FAILURE;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(serverAddr.sin_zero), 0, 8);

	if (bind(sock, (sockaddr *)&server_addr,
		sizeof(sockaddr)) == -1) {
			cerr << "Error binding to socket! " << endl;
			return EXIT_FAILURE;
	}
	addr_len = sizeof(sockaddr);
	cout << "UDPServer Waiting for client on port 5000" << endl;

	while (true) {
		bytes_read = recvfrom(sock, recv_data, 1024, 0, (sockaddr *)&client_addr, &addr_len);
		recv_data[bytes_read] = '\0';
		cout << "("
			<< inet_ntoa(client_addr.sin_addr)
			<< ", "
			<< ntohs(client_addr.sin_port)
			<< ") said : "
			<< recv_data << endl;
	}
	return EXIT_SUCCESS;
}
