/*
Jonathan Woong
804205763
CS118
Project 2
*/

#include "packetManager.h"
#include <bitset>
#include <string>
#include <vector>

using namespace std;

/*
buildPacket(SEQUENCE NUMBER, ACKNOWLEDGE NUMBER, ACK FLAG, SYN FLAG, FIN FLAG, PAYLOAD)
*/
packet* packetManager::buildPacket(uint16_t seqNum, uint16_t ackNum, uint8_t ack, uint8_t syn, uint8_t fin, char payload[MAX_PAYLOAD_SIZE]) {
	packet* outgoingPacket = new packet;
	outgoingPacket->header.seqNum = htons(seqNum);
	outgoingPacket->header.ackNum = htons(ackNum);
	outgoingPacket->header.window = htons(CLIENT_WINDOW_SIZE);
	outgoingPacket->header.notUsed = 0;
	outgoingPacket->header.ack = ack;
	outgoingPacket->header.syn = syn;
	outgoingPacket->header.fin = fin;
	memcpy(outgoingPacket->payload, payload, MAX_PAYLOAD_SIZE);
	return outgoingPacket;
}

// return payload size
uint16_t packetManager::getPayloadSize(packet* packet) {
	uint16_t payLoadSize = strlen(packet->payload);
	return payLoadSize;
}

// print packet headers
void packetManager::printPacket(packet* packet) {
	cout << "SeqNum: " << ntohs(packet->header.seqNum) << endl;
	cout << "AckNum: " << ntohs(packet->header.ackNum)  << endl;
	cout << "Window: " << ntohs(packet->header.window)  << endl;
	cout << "Ack: " << (int)packet->header.ack << endl;
	cout << "Syn: " << (int)packet->header.syn << endl;
	cout << "Fin: " << (int)packet->header.fin << endl;
	cout << "Payload size: " << getPayloadSize(packet) << endl;
}

// check if a packet is syn
bool packetManager::isSyn(packet* packet) {
	if (packet->header.ack == 0
		&& packet->header.syn == 1
		&& packet->header.fin == 0) {
		return true;
	}
	else {
		return false;
	}
} 

// check if a packet is syn-ack
bool packetManager::isSynAck(packet* packet) {
	if (packet->header.ack == 1
		&& packet->header.syn == 1
		&& packet->header.fin == 0) {
		return true;
	}
	else {
		return false;
	}
} 

// check if a packet is ack
bool packetManager::isAck(packet* packet) {
	if (packet->header.ack == 1
		&& packet->header.syn == 0
		&& packet->header.fin == 0) {
		return true;
	}
	else {
		return false;
	}
} 

// check if a packet is fin
bool packetManager::isFin(packet* packet) {
	if (packet->header.ack == 0
		&& packet->header.syn == 0
		&& packet->header.fin == 1) {
		return true;
	}
	else {
		return false;
	}
} 

// check if a packet is fin-ack
bool packetManager::isFinAck(packet* packet) {
	if (packet->header.ack == 1
		&& packet->header.syn == 0
		&& packet->header.fin == 1) {
		return true;
	}
	else {
		return false;
	}
} 

// check if packet is data packet
bool packetManager::isData(packet* packet) {
	if (packet->header.ack == 0
		&& packet->header.syn == 0
		&& packet->header.fin == 0) {
		return true;
	}
	else {
		return false;
	}
}

// return type of packet
int packetManager::getPacketType(packet* packet) {
	if (isSyn(packet)) {
		return SYN;
	}
	else if (isSynAck(packet)) {
		return SYN_ACK;
	}
	else if (isAck(packet)) {
		return ACK;
	}
	else if (isFin(packet)) {
		return FIN;
	}
	else if (isFinAck(packet)) {
		return FIN_ACK;
	}
	else if (isData(packet)) {
		return DATA;
	}
	return -1;
}

// get sequence number
uint16_t packetManager::getSeqNum(packet* packet) {
	return ntohs(packet->header.seqNum);
}

// get ack num
uint16_t packetManager::getAckNum(packet* packet) {
	return ntohs(packet->header.ackNum);
}

// get window size
uint32_t packetManager::getWindowSize(packet* packet) {
	return ntohs(packet->header.window);
}

// print to console
void packetManager::printActivity(uint8_t action, uint8_t packetType, uint16_t number, uint32_t cwnd, uint32_t ssthresh, bool retransmission) {
	switch (action) {
		case (SENDING): {
			cout << "Sending ";
			break;
		}
		case (RECEIVING): {
			cout << "Receiving ";
			break;
		}
		default: {
			break;
		}
	}

	switch (packetType) {
		case (SYN): {
			cout << "packet " << number << " SYN" << endl;
			break;
		}
		case (ACK): {
			cout << "packet " << number << endl;
			break;
		}
		case (SYN_ACK): {
			cout << "packet " << number << " SYN-ACK" << endl;
			break;
		}
		case (DATA): {
			cout << "packet " << number;

			///// SENDING 
			if (cwnd) {
				cout << " " << cwnd << " " << ssthresh;
				if (retransmission) {
					cout << " " << "Retransmission";
				}
			}

			cout << endl;

			break;
		}
		case (FIN): {
			cout << "packet " << number << " FIN" << endl;
			break;
		}
		case (FIN_ACK): {
			cout << "packet " << number << " FIN-ACK" << endl;
		}
		default: {
			break;
		}
	}
}



