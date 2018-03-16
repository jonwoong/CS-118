#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "responseBuilder.h"

using namespace std;

responseBuilder::responseBuilder(string inputBuffer) {
	workingBuffer = inputBuffer;
}

responseBuilder::~responseBuilder(void) {
	
}

string responseBuilder::getFileName(void) {
	return fileName;
}

string responseBuilder::getContentType(void) {
	string contentType = "Content-Type: text/html" + CRLF;
	return contentType;
}

string responseBuilder::getServer(void) {
	string server = "Server: Localhost" + CRLF;
	return server;
}

string responseBuilder::getConnection(void) {
	string connection = "Connection: close" + CRLF;
	return connection;
}

string responseBuilder::getContentLen(void) {
	string contentLength = "Content-Length: " + to_string(getFileSize()) + CRLF;
	return contentLength;
}

int responseBuilder::getFileSize(void) {
	int fileSize;
	FILE *requestedFile = fopen(fileName.c_str(),"r");
	fseek(requestedFile,-1,SEEK_END); // count chars
	fileSize = ftell(requestedFile)+1;
	fclose(requestedFile);
	return fileSize;
}

string responseBuilder::getDateTime(void) {
	time_t now = time(0);
	struct tm timeStruct;
	timeStruct = *localtime(&now);
	char buf[80];
	strftime(buf,sizeof(buf), "%a, %d %b %Y %T %Z", &timeStruct);
	string buffer(buf);
	string dateTime = "Date: " + buffer + CRLF;
	return dateTime;
}

string responseBuilder::getLastModTime(void) {
	struct tm* lastMod;
	struct stat attribute;
	stat(fileName.c_str(), &attribute);
	lastMod = gmtime(&(attribute.st_mtime));
	char lastMT[35];
	strftime(lastMT, 35, "%a, %d %b %Y %T %Z", lastMod);
	string lastMTstring(lastMT);
	string lastModTime = "Last-Modified: ";
	lastModTime += lastMTstring + CRLF;
	return lastModTime;
}

void responseBuilder::parseRequest(void) {
	stringstream requestBlock;
	requestBlock << workingBuffer << endl;

	bool errorDetected=false;
	string requestMethod;
	string requestFilename;
	string requestVersion;
	string errorMessage;
	string status;

	///// PARSE REQUEST /////
	if(getline(requestBlock, requestMethod, ' ') && !requestBlock.eof()) { // if successfully parse method
		if(getline(requestBlock, requestFilename, ' ') && !requestBlock.eof()) { // if successfully parse filename
			if (getline(requestBlock, requestVersion, '\n') && !requestBlock.eof()) { // if successfully parse HTTP verion

				if (requestMethod != METHOD) { // if method is not supported
					cerr << "Request message does not use GET method" << endl;
					errorMessage = requestVersion + " " + BAD_REQ + CRLF;
					errorDetected = true;
				}

				if (requestVersion.substr(0,requestVersion.length()-1) != VERSION) { // if version is not supported
					cerr << "HTTP version not supported" << endl;
					errorMessage = requestVersion + " " + BAD_VERSION + CRLF;
					errorDetected = true;
				}
			}
		}
	}
	if (!errorDetected) {
		status = VERSION + SPACE + OK_REQ + CRLF;
	}
	else {
		status = errorMessage;
	}

	///// SET MEMBER FIELDS /////
	string fileNameString = requestFilename.substr(1);
	fileName = &fileNameString[0u];
	method = &requestMethod[0u];
	statusLine = &status[0u];
	header.connection = getConnection();
	header.date = getDateTime();
	header.serverName = getServer();
	header.lastModified = getLastModTime();
	header.contentLength = getContentLen();
	header.contentType = getContentType();
}

string responseBuilder::generateResponseHeader(void) {
	string headerMsg;
	headerMsg = statusLine + header.connection + header.date + header.serverName + header.lastModified + header.contentLength + header.contentType + CRLF;
	return headerMsg;
}