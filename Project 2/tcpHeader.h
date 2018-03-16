/*
Jonathan Woong
804205763
CS118
Project 2
*/

#include "defaultValues.h"

struct tcpHeader {
	unsigned int 
		seqNum : 16,
		ackNum : 16,
		window : 16,
		notUsed : 13,
		ack : 1,
		syn : 1,
		fin : 1;
};

struct packet {
	struct tcpHeader header;
	unsigned char payload[MAX_PAYLOAD_SIZE]
};