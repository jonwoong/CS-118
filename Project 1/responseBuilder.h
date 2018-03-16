#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>

using namespace std;

static const string METHOD =  "GET";
static const string VERSION = "HTTP/1.0";
static const string CRLF =  "\r\n";
static const string OK_REQ = "200 OK";
static const string BAD_REQ = "400 Bad Request";
static const string NOT_FOUND = "404 Not Found";
static const string BAD_VERSION = "505 HTTP Version Not Supported";
static const string SPACE = " ";

#define MAX_BUFFER_SIZE 1024

using namespace std;

class responseBuilder {
public:
	responseBuilder(string inputBuffer);
	virtual ~responseBuilder(void);

	///// functions
	int getFileSize(void);
	string getLastModTime(void);
	string getDateTime(void);
	void parseRequest(void);
	string generateResponseHeader(void);
	string getContentLen(void);
	string getConnection(void);
	string getServer(void);
	string getContentType(void);
	string getFileName(void);

private:
	string method;
	string statusLine;
	struct headerLines {
		string connection;
		string date;
		string serverName;
		string lastModified;
		string contentLength;
		string contentType;
	}; headerLines header;
	string fileName;
	string workingBuffer;
	
};