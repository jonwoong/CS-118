#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>

using namespace std;

#define MAX_BUFFER_SIZE 5000000

const string METHOD =  "GET";
const string VERSION = "HTTP/1.0";
const string CRLF =  "\r\n";

using namespace std;

class requestBuilder {
public:
	///// data members
	string requestLine;
	struct headerLines {
		string host;
		string userAgent;
		string accept;
		string acceptLanguage;
		string acceptEncoding;
		string acceptCharset;
		string keepAlive;
		string connection;
	}; headerLines header;
	requestBuilder(string input);
	virtual ~requestBuilder(void);

	///// functions
	string getProtocol(void);
	string getHost(void);
	string getPort(void);
	string getObjectPath(void);
	string getObjectName(void);

	int URLcorrect(void);
	void setRequestFields(void);
	string encodeReq(void);

private:
	string inputString;
};