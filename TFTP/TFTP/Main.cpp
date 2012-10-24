#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string>
#include <fstream>
#include <iomanip>

//TODO: Sort includes

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

void readFunction(string readorwrite, string destFile, string File, SOCKET sock, sockaddr_in serverAddr);
void writeFunction(string readorwrite, string destFile, string File, SOCKET sock, sockaddr_in serverAddr);
void makeHeader(string readorwrite, string destFile, char headerData[]);
char* makeACK(char ACK[], int &ackCounter);
void makePacket(int &packetSize, int &filesize, char dataPacket[], int &blockCounter, ifstream &infile);

int main(int argc, char*argv[])
{
	for (int n = 1; n < argc; n++)
	{
		//cout << setw( 2 ) << n << ": " << argv[n] << '\n';
		string input = argv[n];
		if(input == "help" || input == "--help" || input == "-help" || input == "-h" || input == "--h")
		{
			cout << "\n	+------------------------------------+" << endl;
			cout << "	|    This is the help for " << argv[0] << "   |" << endl;
			cout << "	+------------------------------------+\n" << endl;
			cout << "	The right arguments for this program are:\n" << endl;
			cout << "	1: \"write\" or \"read\"" << endl;
			cout << "	2: \"/tftp/destination file.xxx\" for example: /tftp/1.txt" << endl;
			cout << "	3: \"filename including full path\" for example: C:\\Users\\My User\\myfile.txt" << endl;
			cout << "	4: \"Server address\" for example: \"127.0.0.1\"" << endl;
			cout << "\n	Example input to write: write /tftp/1.txt C:\\Users\\Me\\sendfile.txt 192.168.42.100" << endl;
			exit(1);
		}
	}

		string readorwrite = argv[1];
		string destFile = argv[2];
		string File = argv[3];
		string server = argv[4];

	char * servArr = new char[server.length()];

	strcpy(servArr, argv[4]);

	for (int i=0; i < readorwrite.length(); i++)
		readorwrite[i] = tolower(readorwrite[i]);



#pragma region Initialize Winsock
	WSADATA wsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
#pragma endregion Initialize Winsock

#pragma region Socket, server and host Variables
	SOCKET sock;
	sockaddr_in serverAddr;
	hostent *host;
#pragma endregion Socket, server and host Variables

#pragma region Declarations of sock and host
	host = (hostent *) gethostbyname((char *) servArr);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#pragma endregion Declarations of sock and host

#pragma region Check for socket error
	if(sock == INVALID_SOCKET)
	{
		cerr << sock << endl;
		cout << "ERROR, can't open socket\n";
		return EXIT_FAILURE;
	}
#pragma endregion Check for socket error

#pragma region Declaration of port and server address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(69);
	serverAddr.sin_addr = *((struct in_addr *) host->h_addr);
	memset(&(serverAddr.sin_zero), 0, 8);
#pragma endregion Declaration of port and server address

//Start of read function
	if(readorwrite == "read")
		readFunction(readorwrite, destFile, File, sock, serverAddr);

//Start of write function
	if(readorwrite == "write")
		writeFunction(readorwrite, destFile, File, sock, serverAddr);

	cout << "This is the end of the program " << endl;

	WSACleanup();
	return EXIT_SUCCESS;
}

void readFunction(string readorwrite, string destFile, string File, SOCKET sock, sockaddr_in serverAddr)
{
	cout << "\n-------------------------------------------------------------";
	cout << "\nThis function is still in progress and may not function properly";
	cout << "\n-------------------------------------------------------------\n";

	for (int i=0; i< destFile.length(); i++)
		destFile[i] = tolower(destFile[i]);

	ofstream outfile;
	outfile.open(File, ios::binary);


#pragma region Send RRQ header

	int headerlength = destFile.length() + 9;	//Get length of Header

	char * headerData;						//Declare a pointer for Header array
	headerData = new char [headerlength];	//Declare headerData as a dynamic array

	makeHeader(readorwrite, destFile, headerData);

	sendto(sock, headerData, headerlength, 0, (sockaddr *)&serverAddr, sizeof(sockaddr));	//Send Header

	delete [] headerData;																	//Delete Header	- Not used again

#pragma endregion Send RRQ header

	char recv_Data[516];

	int bytes_read = 0;					//Bytes received from server
	int addr_len = sizeof(sockaddr);	//Socket length ?
	int totalSize = 0;

	char * ACK = new char[4];
	int ackCounter = 1;

	//Receive data packets from server
	do{
		bytes_read = recvfrom(sock, recv_Data, 516, 0, (sockaddr *)&serverAddr, &addr_len);	//NOT FINISHED - CHECK FOR ERROR CODE
		if (bytes_read > 0)
		{
			totalSize += bytes_read - 4;
			//TEST
			char * buffer = new char[bytes_read];

			for(int i=4; i<bytes_read; i++)
				cout << recv_Data[i];

			//TEST
			outfile.write (recv_Data+4, bytes_read-4);
			cout << "BYTES READ: " << bytes_read << endl;
			ACK = makeACK(ACK, ackCounter);
			sendto(sock, ACK, 4, 0, (sockaddr *)&serverAddr, sizeof(sockaddr));	//Send first ACK
			cout << "ACK number " << ackCounter << " SENT" << endl;
			
			delete [] buffer;
		}
		if(bytes_read < 0) break;
	}while (bytes_read > 515);

	outfile.close();

	cout << "TOTAL FILE SIZE: " << totalSize << endl;
	cout << "END OF FILE" << endl;
}

void writeFunction(string readorwrite, string destFile, string File, SOCKET sock, sockaddr_in serverAddr)
{
	for (int i=0; i< destFile.length(); i++)
		destFile[i] = tolower(destFile[i]);

#pragma region Open file

	ifstream infile;
	infile.open(File, ios::binary);

#pragma region Check for file error
	if (infile.fail())
	{
		cout << "\n---------------------------------------------------";
		cout << "\nAre you sure the file " << File << " exists ?\n";
		cout << "---------------------------------------------------\n";
		exit(1);
	}
#pragma endregion Check for file error

	infile.seekg(0, ios::end);
	int filesize = infile.tellg();
	cout << "File size: " << filesize << endl;

	// Seeks to the very beginning of the file, clearing any fail bits first (such as the end-of-file bit)
	infile.seekg(0, ios::beg);

#pragma endregion Open file

#pragma region Send WRQ header

	int headerlength = destFile.length() + 9;	//Get length of Header

	char * headerData;						//Declare a pointer for Header array
	headerData = new char [headerlength];	//Declare headerData as a dynamic array

	makeHeader(readorwrite, destFile, headerData);

	sendto(sock, headerData, headerlength, 0, (sockaddr *)&serverAddr, sizeof(sockaddr));	//Send Header
	delete [] headerData;																	//Delete Header	- Not used again

#pragma endregion Send WRQ header

	char recv_Data[4];

	char dataPacket[516];
	int bytes_read = 0;
	int addr_len = sizeof(sockaddr);	//Socket length ?
	int blockCounter = 0;			//Counter to count number of blocks
	int bytesRemaining = filesize;
	int packetSize = 0;

//NOT FINISHED YET - STILL NEEDS A TIMEOUT COUNTER	
#pragma region Send packets and receive ack

	do{
		bytes_read = recvfrom(sock, recv_Data, 4, 0, (sockaddr *)&serverAddr, &addr_len);	//NOT FINISHED - CHECK FOR ERROR CODE
		if (recv_Data[1] == 4)
		{
			if (filesize == 0)
			{
				dataPacket[0] = 0;
				dataPacket[1] = 3;
				dataPacket[2] = 0;
				dataPacket[3] = blockCounter+1;
				packetSize = 4;
				sendto(sock, dataPacket, packetSize, 0, (sockaddr *)&serverAddr, sizeof(sockaddr));
				break;
			}

			makePacket(packetSize, filesize, dataPacket, blockCounter, infile);

			sendto(sock, dataPacket, packetSize, 0, (sockaddr *)&serverAddr, sizeof(sockaddr));
			filesize -= packetSize-4;

			cout << "BYTES SENT: " << packetSize << endl;
			cout << "FILESIZE: " << filesize << endl;
		}
	}while (packetSize > 515);

#pragma endregion Send packets and receive ack

	infile.close();			//Close the file

	cout << "The file should have been sent by now" << endl;

	cout << "\n---------------------------------------------";
	cout << "\nWoohoo! The file should have been sent by now\n";
	cout << "---------------------------------------------\n";
}

void makeHeader(string readorwrite, string destFile, char headerData[])
{
	int counter = 2;

	headerData[0] = 0;
	if(readorwrite == "read")
		headerData[1] = 1;

	if(readorwrite == "write")
		headerData[1] = 2;

	for (int i=2, j=0; j<destFile.length(); i++, j++)
	{
		headerData[i] = destFile[j];
		counter++;
	}

	headerData[counter] = 0;
	headerData[counter+1] = 'o';
	headerData[counter+2] = 'c';
	headerData[counter+3] = 't';
	headerData[counter+4] = 'e';
	headerData[counter+5] = 't';
	headerData[counter+6] = 0;
}

char* makeACK(char ACK[], int &ackCounter)
{
	ACK[0] = 0;
	ACK[1] = 4;
	ACK[2] = 0;
	ACK[3] = ackCounter++;

	return ACK;
}

void makePacket(int &packetSize, int &filesize, char dataPacket[], int &blockCounter, ifstream &infile)
{
	blockCounter++;

	dataPacket[0] = 0;
	dataPacket[1] = 3;
	dataPacket[2] = 0;
	if(blockCounter < 256)
		dataPacket[3] = blockCounter;

	if(filesize >= 512)
	{
		packetSize = 516;

		infile.read(dataPacket+4, 512);
	}

	if(filesize < 512)// && filesize > 0)
	{
		packetSize = filesize+4;

		infile.read(dataPacket+4, filesize);	//BANNAÐ AÐ BREYTA
	}

	if(filesize == 0)
	{
		packetSize = 4;
	}

}