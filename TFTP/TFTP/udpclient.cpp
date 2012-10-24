#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <winsock2.h>

using namespace std;

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 0);
	if (WSAStartup(version, &wsaData)) {
		cerr << "Failure to find WinSock 2.2 or better." << endl;
		return EXIT_FAILURE;
	}

	SOCKET sock;
	sockaddr_in serverAddr;
	hostent *host;
	char sendData[1024];

	host = (hostent *) gethostbyname((char *) "127.0.0.1");
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		cerr << sock << endl;
		cerr << "Error connecting to socket! " << endl;
		return EXIT_FAILURE;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5000);
	serverAddr.sin_addr = *((struct in_addr *) host->h_addr);
	memset(&(serverAddr.sin_zero), 0, 8);

	while (true) {
		cout << "Type Something (q or Q to quit):" << endl;
		cin.getline(sendData, 1024);
		if (tolower(sendData[0]) == 'q') {
			break;
		}
		cout << "Sent: " << sendData << endl;
		sendto(sock, sendData, strlen(sendData), 0,
			(sockaddr *)&serverAddr, sizeof(sockaddr));
	}
	WSACleanup();
	return EXIT_SUCCESS;
}