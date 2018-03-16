/* 

Jonathan Woong
804205763
CS118 - DIS 1A
PROJECT 2

*/

////////// HEADERS //////////
#include <string.h>
#include <thread>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <bitset>
#include <fstream>
#include <unistd.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include "packet.h"
#include "errorHandler.cpp"
#include "packetManager.cpp"

using namespace std;

////////// FLAGS //////////
bool completedHandshake = false;
bool doneSendingFile = false;
bool serverDead = false;
bool sentLastPacket = false;
bool firstCycle = true;

////////// GLOBAL VARIABLES //////////
string fileName; // acquired in main()
int port = 0; // set in main()
int serverFd = 0; // calculated in main()
struct sockaddr_in serverAddr; // calculated in main()
struct sockaddr_in clientAddr; // calculated in main()
socklen_t clientAddrLen; // calculated in getClientData()

char emptyPayload[MAX_PAYLOAD_SIZE] = {0}; 
uint32_t currentCWND = INIT_CONGESTION_WINDOW_SIZE; // increase when receive ack
uint32_t numberOfPackets = 0; // calculated in divideFileIntoPackets()
uint32_t numberOfDataAcks = 0; // increment when receive ack
uint32_t numberOfPacketsInTransit = 0; // increment when sendto(), decrement when recvfrom() data ack
uint32_t transmissionWindow = 0; // max number of packets that can be in transit at a given time
uint32_t fileSize = 0; // calculated in divideFileIntoPackets()
uint16_t finSeqNum = 0; // sequence number of fin to client
uint16_t currentPacketIndex = 0; // increments as packets get sent out
vector<packet*> outgoingPackets;
vector<packet*> receivedPackets;
vector<uint16_t> receivedAckNums;
vector<uint16_t> allOutgoingSeqNums;
vector<uint16_t> allExpectedIncomingAckNums;
map<uint16_t, uint32_t> seqAndIndex; // used in timeout <seq,index>
map<uint32_t, struct timeval> indexOfPacketAndTime; // used in timeout <index, time>

////////// HELPER CLASSES //////////
packetManager packetManager; // builds packets & prints packet* activity
errorHandler errorHandler; // prints errors

////////// SERVER FUNCTION PROTOTYPES //////////
void createSocket();
void bindSocket();
void getClientData();

void sendPacket(packet* outgoingPacket);
void beginHandshake();
void completeHandshake(packet* incomingPacket);
void divideFileIntoPackets(string fileName);
void processDataAck(packet* incomingPacket);
uint32_t choosePacket();
void attemptSendData();
void beginClosingConnection();
void finishClosingConnection();

struct timeval getCurrentTime();
void checkForTimeOut();
void linkSeqToIndex();

////////// PROGRAM //////////
int main(int argc, char *argv[])
{
	////////// CMD-LINE ARGUMENTS //////////
	port = stoi(argv[1]);
	fileName = argv[2];

	////////// CREATE SOCKET //////////
	createSocket();

	////////// BIND SOCKET //////////
	bindSocket();

	////////// CLIENT DATA //////////
	getClientData();

	////////// SERVER ROUTINE //////////
	while (!serverDead) {
		///// TIMEOUT?
		checkForTimeOut();

		///// RECEIVE PACKET?
		packet incomingPacket;

		////////// RECEIVE PACKETS //////////
		int numBytesReceived = recvfrom(serverFd, &incomingPacket, MAX_PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

		if (numBytesReceived > 0) {
			///// IDENTIFY PACKET TYPE
			uint16_t packetType = packetManager.getPacketType(&incomingPacket);

			///// SAVE PACKET
			receivedPackets.push_back(&incomingPacket);

			////////// HANDLE PACKET BASED ON TYPE //////////
			switch (packetType) {
				////// RECEIVE SYN
				case (SYN): {
					beginHandshake();
					break;
				}

				///// RECEIVE ACK
				case (ACK): {
					///// RECEIVING FIRST ACK COMPLETES THE HANDSHAKE
					if (!completedHandshake) {	
						completeHandshake(&incomingPacket);
						divideFileIntoPackets(fileName);
						linkSeqToIndex(); 						
					}
					///// RECEIVING DATA ACK 
					else {
						processDataAck(&incomingPacket);
					}
					
					attemptSendData();

					///// DONE SENDING FILE?
					if (doneSendingFile) {
						beginClosingConnection();
					}
					break;
				}

				///// RECEIVE FIN-ACK
				case (FIN_ACK): {
					packetManager.printActivity(RECEIVING, FIN_ACK, finSeqNum+1, 0, 0, 0);
					break;
				}

				///// RECEIVE FIN
				case (FIN): {
					finishClosingConnection();
					break;
				}

				///// DEFAULT
				default: {
					break;
				}
			}
		}
	}
	return 0;
}

////////// SERVER METHODS //////////
void createSocket() {
	serverFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverFd < 0) {
		errorHandler.print(SOCKET_ERROR);
		exit(SOCKET_ERROR);
	}
}

void bindSocket() {
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDR.c_str(), &(serverAddr.sin_addr)); // convert string to sin_addr
	serverAddr.sin_port = htons(port);
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

	int bindResult = bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (bindResult < 0) {
		errorHandler.print(BIND_ERROR);
		exit(BIND_ERROR);
	}
}

void getClientData() {
	clientAddrLen = sizeof(clientAddr);
}

void sendPacket(packet* outgoingPacket) {
	int numBytesSent = sendto(serverFd, outgoingPacket, MAX_PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
	if (numBytesSent<0) {
		errorHandler.print(SENDTO_ERROR);
	}
} 

void beginHandshake() {
	///// RECEIVED SYN
	printf("=== PERFORMING THREE WAY HANDSHAKE ===\n");
	packetManager.printActivity(RECEIVING, SYN, INIT_SEQ_NUM, 0, 0, 0);
	///// SEND SYN-ACK
	packetManager.printActivity(SENDING,SYN_ACK, INIT_SEQ_NUM, 0, 0, 0);
	packet* synAckPacket = packetManager.buildPacket(INIT_SEQ_NUM, INIT_SEQ_NUM+1, ack, syn, 0, emptyPayload);
	sendPacket(synAckPacket);
}

void completeHandshake(packet* incomingPacket) {
	uint16_t receivedAckNum = packetManager.getAckNum(incomingPacket);
	packetManager.printActivity(RECEIVING, ACK, receivedAckNum, 0, 0, 0);
	receivedAckNums.push_back(receivedAckNum);
	completedHandshake = true;
	printf("=== COMPLETED HANDSHAKE ===\n");
}

void divideFileIntoPackets(string fileName) {
	///// OPEN FILE
	ifstream readStream(fileName.c_str(), ios::binary);
	if (!readStream.is_open()) {
		errorHandler.print(OPEN_ERROR);
	}

	////// GET FILE SIZE
	readStream.seekg(0, readStream.end);
	fileSize = readStream.tellg();
	readStream.seekg(0, readStream.beg);

	///// GET NUMBER OF PACKETS NECESSARY TO TRANSFER FILE
	numberOfPackets = (fileSize/MAX_PAYLOAD_SIZE)+1;

	///// POPULATE packet* ARRAY
	uint32_t outgoingPacketSeqNum = INIT_SEQ_NUM+1; // initial value
	uint32_t packetIndex = 0; // initial value
	char fileContents[MAX_PAYLOAD_SIZE]; // buffer to hold file data

	while (packetIndex < numberOfPackets) {
		if (outgoingPacketSeqNum > 30720) {
			outgoingPacketSeqNum = 0;
		}
		fill(fileContents, fileContents+MAX_PAYLOAD_SIZE, 0); // zero out buffer
		readStream.read(fileContents,MAX_PAYLOAD_SIZE); // fill buffer with file contents
		outgoingPackets.push_back(packetManager.buildPacket(outgoingPacketSeqNum,0,0,0,0,fileContents)); // build packet
		allOutgoingSeqNums.push_back(outgoingPacketSeqNum);
		allExpectedIncomingAckNums.push_back(outgoingPacketSeqNum+readStream.gcount());
		outgoingPacketSeqNum += readStream.gcount(); // increase seq num, used for next packet
		packetIndex++; 
	}
	///// CLOSE FILE
	readStream.close();
}

void processDataAck(packet* incomingPacket) {
	uint16_t incomingAckNum = packetManager.getAckNum(incomingPacket);
	packetManager.printActivity(RECEIVING, ACK, incomingAckNum, 0, 0, 0);

	///// UPDATE GLOBAL VALUES
	numberOfDataAcks++;
	numberOfPacketsInTransit--;
	receivedAckNums.push_back(incomingAckNum);

	
	///// REMOVE FROM MAP
	// get index from seq number
	uint32_t index = 0;
	map<uint16_t, uint32_t>::iterator it = seqAndIndex.begin();
	it = seqAndIndex.find(incomingAckNum);
	if (it !=seqAndIndex.end()) {
		it--; // incoming ACK num - payload = outgoing seq num
		index = it->second;
	}
	
	// use index to find a packet's current timeout count, remove it (since it was acked)
	map<uint32_t, struct timeval>::iterator it2 = indexOfPacketAndTime.begin();
	it2 = indexOfPacketAndTime.find(index);
	if (it2 != indexOfPacketAndTime.end()) {
		indexOfPacketAndTime.erase(it2);
	}
	
	
	///// DONE SENDING FILE?
	if (numberOfDataAcks == numberOfPackets) { 
		doneSendingFile = true;
		printf("=== FINISHED SENDING DATA PACKETS ===\n");
	}

	///// INCREASE CURRENT CWND
	if (currentCWND < SSTHRESH) {
		currentCWND *= 2;
	}
	else {
		currentCWND += 1;
	}						
}

uint32_t currentIndex = 0;
uint32_t choosePacket() {
	int returnThis = currentIndex;
	currentIndex++;
	return returnThis;
}

void attemptSendData() {
	transmissionWindow = (currentCWND/MAX_PAYLOAD_SIZE);

	while ((numberOfPacketsInTransit < transmissionWindow) && (numberOfDataAcks < numberOfPackets) && !sentLastPacket) {
		uint32_t packetIndex = choosePacket();
		uint16_t outgoingPacketSeqNum = packetManager.getSeqNum(outgoingPackets[packetIndex]);
		packetManager.printActivity(SENDING, DATA, outgoingPacketSeqNum, currentCWND, SSTHRESH, 0);
		sendPacket(outgoingPackets[packetIndex]);
		indexOfPacketAndTime[packetIndex] = getCurrentTime(); // <index, time> assiociate current time with outgoing packet index
		if (packetIndex == numberOfPackets-1) { // last packet sent
			sentLastPacket = true;
		}
		numberOfPacketsInTransit++;
	}
}

void beginClosingConnection() {
	printf("=== BEGIN CLOSING CONNECTION ===\n");
	///// SEND FIN
	finSeqNum = packetManager.getAckNum(receivedPackets.back());
	packet* finPacket = packetManager.buildPacket(finSeqNum, fileSize+1, 0, 0, fin, emptyPayload);
	packetManager.printActivity(SENDING, FIN, packetManager.getSeqNum(finPacket), 0, 0, 0);
	sendPacket(finPacket);
}

void finishClosingConnection() {
	///// RECEIVE FIN
	packetManager.printActivity(RECEIVING, FIN, finSeqNum+1, 0, 0, 0);
	
	///// SEND FIN-ACK
	packet* finAckPacket = packetManager.buildPacket(finSeqNum+2, 0, ack, 0, fin, emptyPayload);
	sendPacket(finAckPacket);
	packetManager.printActivity(SENDING, FIN_ACK, finSeqNum+2, 0, 0, 0);

	///// CLOSE CONNECTION
	close(serverFd);
	serverDead = true;
	printf("=== FINISHED CLOSING CONNECTION ===\n");
}

struct timeval getCurrentTime() {
	struct timeval currentTime;
	gettimeofday(&currentTime, 0);
	return currentTime;
}

void checkForTimeOut() {
	map<uint32_t, struct timeval>::iterator it = indexOfPacketAndTime.begin();
	// it.first = index of packet
	// it.second = time that packet was sent
	while (it != indexOfPacketAndTime.end()) {
		// find a packet index in indexOfPacketAndTime that has a send time > 500 ms ago
		struct timeval currentTime = getCurrentTime();
		if (currentTime.tv_usec - (it->second).tv_usec >= 5000000) { // 500 ms
			// index
			uint32_t index = it->first;
			// resend packet
			sendPacket(outgoingPackets[index]);
			// reset timer
			indexOfPacketAndTime.erase(it);
			indexOfPacketAndTime[index] = getCurrentTime();
		}
		it++;
	}
}

void linkSeqToIndex() {
	for (uint32_t i=0; i<allOutgoingSeqNums.size(); i++) {
		// create mapping <outgoingSeq, index>
		seqAndIndex[allOutgoingSeqNums[i]] = i;
	}
}
