/*
Jonathan Woong
804205763
CS118
Project 2
*/

#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "defaultValues.h"
#include "errorHandler.h"

using namespace std;

void errorHandler::print(int errorNumber) {
	switch (errorNumber) {
		case SOCKET_ERROR: {
			cerr << "Error creating socket" << endl;
			break;
		}
		case BIND_ERROR: {
			cerr << "Error binding socket" << endl;
			break;
		}
		case HOST_ERROR: {
			cerr << "Error obtaining host address" << endl;
			break;
		}
		case SENDTO_ERROR: {
			cerr << "Error sendto() failed" << endl;
			break;
		}
		case RECEIVE_ERROR: {
			cerr << "Error receiving data" << endl;
			break;
		}
		case OPEN_ERROR: {
			cerr << "Error opening file" << endl;
			break;
		}
		default: {
			break;
		}
	}
}

#endif