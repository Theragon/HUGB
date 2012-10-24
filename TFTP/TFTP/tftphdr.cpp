#include "tftphdr.h"
#include <string.h>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

Packet * generate_packet(std::vector<char> data) {
	char buf[data.size()];
	for(unsigned int i = 0; i < data.size(); i++)
		buf[i] = data[i];

	unsigned int i = 2;
	unsigned short opcode;
	memcpy(&opcode, buf, 2);
	opcode = ntohs(opcode);

	if(opcode == RRQ) {
		string filename;
		while(data[i] != 0) {
			filename.push_back((data[i]));
			i++;
		}
		string Mode;
		i += 1;
		while(data[i] != 0) {
			Mode.push_back(data[i]);
			i += 1;
		}
		return (Packet*) new RRQ_Packet(opcode, filename, Mode);
	}
	else if(opcode == DATA) {
		unsigned short block_no;
		memcpy(&block_no, buf+i, 2);
		block_no = ntohs(block_no);
		i += 2;
		vector<char> new_data(data.size()-4);
		memcpy(new_data.data(), buf+i, data.size()-4);
		return (Packet*) new DATA_Packet(opcode, block_no, new_data);
	}
	else if(opcode == ACK) {
		unsigned short block_no;
		memcpy(&block_no, buf+i, 2);
		block_no = ntohs(block_no);
		return (Packet*) new ACK_Packet(opcode, block_no);
	}
	else if(opcode == ERROR) {
		unsigned short ErrorCode;
		memcpy(&ErrorCode, buf+i, 2);
		ErrorCode = ntohs(ErrorCode);
		i += 2;
		std::string ErrorMsg;
		while(buf[i] != 0) {
			ErrorMsg.push_back(buf[i]);
			i += 1;
		}
		return (Packet*) new ERROR_Packet(opcode, ErrorCode, ErrorMsg);
	}
	else {
		/* Bad packet; ignore */
		return new Packet();
	}
}

std::vector<char> ERROR_Packet::to_array() {
	std::vector<char> out;
	char buf[4];
	unsigned short oc = htons(opcode);
	unsigned short ec = htons(ErrorCode);
	memcpy(buf, &oc, 2);
	memcpy(buf+2, &ec, 2);
	for(unsigned int i = 0; i < 4; i++)
		out.push_back(buf[i]);
	for(unsigned int i = 0; i < ErrMsg.size(); i++)
		out.push_back(ErrMsg[i]);
	out.push_back(0);
	return out;
}

std::vector<char> DATA_Packet::to_array() {
	vector<char> pkt(data.size() + 4);
	// convert to network byte order
	unsigned short oc = htons(opcode);
	unsigned short bn = htons(block_no);
	memcpy(pkt.data(), &oc, 2);
	memcpy(pkt.data()+2, &bn, 2);
	memcpy(pkt.data()+4, data.data(), data.size());
	return pkt;
}