/*
Jonathan Woong
804205763
CS118
Project 2
*/

#ifndef PACKET_H
#define PACKET_H

#include "defaultValues.h"
#include <string>
#include <bitset>
#include <vector>
#include <sys/time.h>

using namespace std;

#pragma pack(push,1)
struct tcpHeader {
	uint16_t seqNum;
	uint16_t ackNum;
	uint16_t window;
	uint16_t notUsed : 13;
	uint8_t ack : 1;
	uint8_t syn : 1;
	uint8_t fin : 1;
};

struct packet {
	struct tcpHeader header;
	char payload[MAX_PAYLOAD_SIZE];
};
#pragma pack(pop)


#endif