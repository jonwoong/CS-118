/*
Jonathan Woong
804205763
CS118
Project 2
*/

#ifndef PACKET_MANAGER_H
#define PACKET_MANAGER_H

#include "packet.h"
#include <bitset>
#include <vector>

class packetManager {
public:
	packet* buildPacket(
		uint16_t seqNum, 
		uint16_t ackNum,  
		uint8_t ack, 
		uint8_t syn, 
		uint8_t fin,
		char payload[MAX_PAYLOAD_SIZE]);

	uint16_t getPayloadSize(packet* packet);
	void printPacket(packet* packet);
	bool isSyn(packet* packet);
	bool isSynAck(packet* packet);
	bool isAck(packet* packet);
	bool isFin(packet* packet);
	bool isFinAck(packet* packet);
	bool isData(packet* packet);
	int getPacketType(packet* packet);
	uint16_t getSeqNum(packet* packet);
	uint16_t getAckNum(packet* packet);
	uint32_t getWindowSize(packet* packet);
	void printActivity(uint8_t action, uint8_t packetType, uint16_t number, uint32_t cwnd, uint32_t ssthresh, bool retransmission);


private:

};

#endif