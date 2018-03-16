#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>
#include "requestBuilder.h"

using namespace std;

requestBuilder::requestBuilder(string input) {
	inputString = input;
}

requestBuilder::~requestBuilder(void) {
	
}

string requestBuilder::getProtocol(void) {
	string protocol = inputString.substr(0, inputString.find("//")); // everything before double slash
	return protocol;
}

string requestBuilder::getHost(void) {
	string subString = inputString.substr(inputString.find("//")+2); // everything after double slash
	string host = subString.substr(0, subString.find(":")); // everything between double slash and :
	return host;
}

string requestBuilder::getPort(void) {
	string after1Colon = inputString.substr(inputString.find(":")+1); // everything after 1st colon
	string after2Colon = after1Colon.substr(after1Colon.find(":")+1); // everything after 2nd colon
	string port = after2Colon.substr(0,after2Colon.find("/")); // everything between 2nd colon and /
	return port; // convert string to int
}

string requestBuilder::getObjectPath(void) {
	string objectPath = inputString.substr(inputString.find(getPort()) + getPort().length()); // everything after port #
	if (objectPath=="/") {
		objectPath = "/index.html";
	}
	return objectPath;
}

string requestBuilder::getObjectName(void) {
	string reversedObjectName;
	string actualObjectName;

	for (int i=inputString.length(); i > 0; i--) { // iterate over object path string from tail to head
		char currentChar = inputString[i];
		if (currentChar=='/') {
			break;
		}
		else {
			reversedObjectName += currentChar; // build object name backwards
		}
	}

	for (int j=reversedObjectName.length()-1; j > 0; j--) { // reverse object name again (get correct name)
		actualObjectName += reversedObjectName[j];
	}

	if (actualObjectName=="") {
		actualObjectName = "index.html";
	}
	return actualObjectName;
}

int requestBuilder::URLcorrect(void) {
	if (getProtocol()!="http:") {
		cerr << "Request is not HTTP protocol" << endl;
		return -1;
	}
	if (getHost()!="localhost") {
		cerr << "Unknown hostname" << endl;
		return -1;
	}
	if (stoi(getPort())<0) {
		cerr << "Invalid port #" << endl;
		return -1;
	}
	if (getObjectPath()=="") {
		cerr << "No object specified" << endl;
		return -1;
	}
	return 1; // URL is correctly formatted
}

void requestBuilder::setRequestFields(void) {
	if (URLcorrect()) {
		requestLine = METHOD + " " + getObjectPath() + " " + VERSION + CRLF;
		header.host = "Host: " + getHost() + ":" + getPort() + CRLF;
	}
	else {
		cout << "Could not set req fields" << endl;
	}
}

string requestBuilder::encodeReq(void) {
	string result = requestLine + header.host + CRLF;
	return result;
}

