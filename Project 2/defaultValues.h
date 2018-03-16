/*
Jonathan Woong
804205763
CS118
Project 2
*/

#ifndef DEFAULT_H
#define DEFAULT_H

#include <string>

using namespace std;

////////// SERVER/CLIENT CONSTANTS //////////

	const extern string SERVER_ADDR = "10.0.0.1";
	const extern string CLIENT_ADDR = "10.0.0.2";
	const extern uint8_t ack = 1;
	const extern uint8_t syn = 1;
	const extern uint8_t fin = 1;
	const extern uint16_t MAX_PACKET_SIZE = 1032;
	const extern uint16_t MAX_PAYLOAD_SIZE = 1024;
	const extern uint16_t MAX_SEQ_NUM = 15360;
	const extern uint16_t INIT_SEQ_NUM = 0;
	const extern uint16_t INIT_CONGESTION_WINDOW_SIZE = 1024;
	const extern uint16_t INIT_SLOW_START_THRES = 30720;
	const extern uint16_t RETRANSMIT_TIMEOUT = 500;
	const extern uint16_t CLIENT_WINDOW_SIZE = 15360;
	const extern uint16_t SENDING = 100;
	const extern uint16_t RECEIVING = 101;
	const extern uint16_t SYN = 102;
	const extern uint16_t SYN_ACK = 103;
	const extern uint16_t ACK = 104;
	const extern uint16_t DATA = 105;
	const extern uint16_t FIN = 106;
	const extern uint16_t FIN_ACK = 107;
	const extern uint16_t SSTHRESH = 30720;
	const extern uint16_t CWND = 1024;

////////// ERROR CONSTANTS //////////

	const extern uint16_t SOCKET_ERROR = 1;
	const extern uint16_t BIND_ERROR = 2;
	const extern uint16_t HOST_ERROR = 3;
	const extern uint16_t SENDTO_ERROR = 4;
	const extern uint16_t RECEIVE_ERROR = 5;
	const extern uint16_t OPEN_ERROR = 6;

#endif