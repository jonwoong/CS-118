/* 

Jonathan Woong
804205763
CS118 - DIS 1A
PROJECT 2

*/

#include <string.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include "defaultValues.h"
#include "errorHandler.cpp"
#include "packet.h"
#include "packetManager.cpp"

using namespace std;

////////// FLAGS //////////
bool gotAllData = false;
bool receivedSynAck = false;

////////// GLOBAL VALUES //////////
string serverHost; // set in main()
int port = 0; // set in main()
int clientFd = 0; // set in main()
struct sockaddr_in clientAddr; // set in main()
struct sockaddr_in serverAddr; // set in main()
socklen_t serverAddrLen; // calculated in main()
char emptyPayload[MAX_PAYLOAD_SIZE] = {0};
ofstream writeStream; // writes output file
uint16_t receivedFinNum = 0; // fin from server

////////// HELPER CLASSES //////////
packetManager packetManager;
errorHandler errorHandler;

////////// CLIENT FUNCTION PROTOTYPES //////////
void createSocket();
void bindSocket();
void getHostData();
void sendPacket(packet* outgoingPacket);
void performHandshake();
void handleData(packet* receivedPacket);
void closeConnection(packet* receivedPacket);


////////// CLIENT ROUTINE //////////
int main(int argc, char *argv[])
{
	////////// CMD-LINE ARGUMENTS //////////
	serverHost = argv[1];
	port = stoi(argv[2]);

	////////// CREATE SOCKET //////////
	createSocket();

	////////// BIND SOCKET //////////
	bindSocket();
	
	////////// HOST DATA //////////
	getHostData();

	////////// THREE WAY HANDSHAKE //////////
	performHandshake();

	///// LOOP UNTIL GOT ALL DATA
	while(!gotAllData) {
		///// LISTEN FOR INCOMING DATA
		packet receivedPacket;

		int numBytesReceived = recvfrom(clientFd, &receivedPacket, MAX_PACKET_SIZE, 0, (struct sockaddr*)&serverAddr, &serverAddrLen);

		///// RESPOND TO INCOMING PACKET BASED ON PACKET TYPE
		if (numBytesReceived > 0) {
			uint16_t packetType = packetManager.getPacketType(&receivedPacket);

			switch (packetType) {
				///// RECEIVE DATA
				case (DATA): {
					handleData(&receivedPacket);
					break;	
				}
				///// RECEIVE FIN
				case (FIN): {
					closeConnection(&receivedPacket);
					break;
				}

				///// DEFAULT
				default: {
					break;	
				}
			}
		}
	}
	
}

////////// CLIENT FUNCTION IMPLEMENTATION //////////
void createSocket() {
	clientFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (clientFd < 0) {
		errorHandler.print(SOCKET_ERROR);
		exit(SOCKET_ERROR);
	}
}

void bindSocket() {
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htons(INADDR_ANY);
	clientAddr.sin_port = htons(port);
	memset(clientAddr.sin_zero, '\0', sizeof(clientAddr.sin_zero));

	int bindResult = bind(clientFd, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
	if (bindResult < 0) {
		errorHandler.print(BIND_ERROR);
		exit(BIND_ERROR);
	}
}

void getHostData() {
	memset((char*)&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	struct hostent *host;
	host = gethostbyname(serverHost.c_str());
	if (!host) {
		errorHandler.print(HOST_ERROR);
		exit(HOST_ERROR);
	}
	memcpy((void*)&serverAddr.sin_addr, host->h_addr_list[0], host->h_length); // set server address
	serverAddrLen = sizeof(serverAddr);
}

void sendPacket(packet* outgoingPacket) {
	int numBytesSent = sendto(clientFd, outgoingPacket, MAX_PACKET_SIZE, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (numBytesSent<0) {
		errorHandler.print(SENDTO_ERROR);
		exit(SENDTO_ERROR);
	}
} 

void performHandshake() {
	printf("=== PERFORMING THREE WAY HANDSHAKE ===\n");
	uint16_t synAckSeqNum = 0;
	packet receivedPacket;

	///// WAIT FOR SYN-ACK
	while(!receivedSynAck) {
		///// SEND SYN
		packetManager.printActivity(SENDING,SYN,INIT_SEQ_NUM,0,0,0);
		packet* synPacket = packetManager.buildPacket(INIT_SEQ_NUM, 0, 0, syn, 0, emptyPayload);
		sendPacket(synPacket);

		///// LISTEN FOR SYN-ACK
		int numBytesReceived = recvfrom(clientFd, &receivedPacket, MAX_PACKET_SIZE, 0, (struct sockaddr*)&serverAddr, &(serverAddrLen));
		if (numBytesReceived < 0) {
			errorHandler.print(RECEIVING);
			exit(RECEIVE_ERROR);
		}
		if(packetManager.isSynAck(&receivedPacket)) {
			synAckSeqNum = packetManager.getSeqNum(&receivedPacket);
			packetManager.printActivity(RECEIVING,SYN_ACK,synAckSeqNum, 0,0,0);
			receivedSynAck = true;
		}
	}

	///// SEND ACK
	packetManager.printActivity(SENDING,SYN_ACK,synAckSeqNum+1,0,0,0);
	packet* synAckPacket = packetManager.buildPacket(INIT_SEQ_NUM+1, synAckSeqNum+1, ack, 0, 0, emptyPayload);
	sendPacket(synAckPacket);
	printf("=== COMPLETED THREE WAY HANDSHAKE===\n");
	writeStream.open("received.data", ios::binary);
}

void handleData(packet* receivedPacket) {
	///// GET PACKET INFO
	uint16_t receivedSeqNum = packetManager.getSeqNum(receivedPacket);
	uint16_t receivedPayloadSize = packetManager.getPayloadSize(receivedPacket);

	packetManager.printActivity(RECEIVING, DATA, receivedSeqNum,0,0,0);

	///// WRITE FILE
	writeStream << receivedPacket->payload;
	writeStream.flush(); // ensure buffer contents are written to file
	
	///// SEND ACK
	uint16_t outgoingAckNum = receivedSeqNum + receivedPayloadSize;
	if (outgoingAckNum > 30720) {
		outgoingAckNum = 0;
	}
	packet* ackPacket = packetManager.buildPacket(0, outgoingAckNum, ack, 0, 0, emptyPayload);
	packetManager.printActivity(SENDING, ACK, outgoingAckNum, 0, 0, 0);
	sendPacket(ackPacket);		
}

void closeConnection(packet* receivedPacket) {
	printf("=== BEGIN CLOSING CONNECTION ===\n");
	uint16_t receivedSeqNum = packetManager.getSeqNum(receivedPacket);
	uint16_t receivedAckNum = packetManager.getAckNum(receivedPacket);
	packetManager.printActivity(RECEIVING, FIN, receivedSeqNum,0,0,0);
	
	///// SEND FIN-ACK
	packet* finAckPacket = packetManager.buildPacket(receivedAckNum, receivedSeqNum+1, ack, 0, fin, emptyPayload);
	packetManager.printActivity(SENDING, FIN_ACK, receivedSeqNum+1, 0, 0, 0);
	sendPacket(finAckPacket);

	///// SEND FIN
	packet* finPacket = packetManager.buildPacket(receivedAckNum, receivedSeqNum+1, 0, 0, fin, emptyPayload);
	packetManager.printActivity(SENDING, FIN, receivedSeqNum+ 1,0,0,0);
	sendPacket(finPacket);

	///// CLOSE CONNECTION
	close(clientFd);

	///// CLOSE FILE
	writeStream.close();
	
	gotAllData = true;
	printf("=== FINISHED CLOSING CONNECTION===\n");
}

