#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <fstream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include "requestBuilder.cpp"
#include "dnsresolver.cpp"

using namespace std;

int main(int argc, char* argv[])
{
  for (int i=1; i<argc ; i++) {
    /////////// RESOLVE DNS //////////
    struct addrinfo hints;
    struct addrinfo* res;

    // prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // get address
    string inputString(argv[i]);
    requestBuilder reqBuilder(inputString);
    int status = 0;
    if ((status = getaddrinfo(reqBuilder.getHost().c_str(), "http", &hints, &res)) != 0) {
     cerr << "getaddrinfo: " << gai_strerror(status) << endl;
      return 2;
    } 

    char* IPaddr;
    for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
      // convert address to IPv4 address
      struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

      // convert the IP to a string and print it:
      char ipstr[INET_ADDRSTRLEN] = {'\0'};
      inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
      IPaddr = ipstr;
    }

    freeaddrinfo(res); // free the linked list

    ////////// CREATE SOCKET //////////
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
    // set socket timeout
    struct timeval timeOut;
    timeOut.tv_sec = 10; // 10 second timeout
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(struct timeval *)&timeOut,sizeof(struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&timeOut,sizeof(struct timeval));

    ////////// GET SERVER ADDRESS //////////
    DNSresolver getDNS;
    string serverIPAddress = getDNS.resolveAddr(reqBuilder.getHost());
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(stoi(reqBuilder.getPort()));     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr(serverIPAddress.c_str());
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    ////////// CONNECT TO SERVER /////////
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      perror("connect");
      return 2;
    }

    struct sockaddr_in clientAddr;
    clientAddr.sin_port = ntohs(stoi(reqBuilder.getPort()));
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
      perror("getsockname");
      return 3;
    }

    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));

    ////////// CREATE REQUEST HEADER ////////// 
    reqBuilder.setRequestFields(); // set all header fields
    string request = reqBuilder.encodeReq(); // capture header as string

    ////////// SEND REQUEST //////////
    if (send(sockfd, request.c_str(), request.length(), 0) != request.length()) {
        cout << "Error sending request." << endl;
        exit(1);
      }

    ////////// RECEIVE DATA //////////
    string fileName = reqBuilder.getObjectName(); // get name of requested file
    char* receivedContent; // buffer to hold incoming data
    receivedContent = (char*)malloc(MAX_BUFFER_SIZE); // chunks received
    ofstream outputFile(fileName.c_str()); // open stream to write data to file
    while(recv(sockfd, receivedContent, MAX_BUFFER_SIZE, 0) > 0) {
      cout << "Receiving data for file: " << fileName << endl;
      outputFile << receivedContent; // receive incoming data
      outputFile.flush(); // save incoming data
    }

    ////////// CLOSE CONNECTION //////////
    close(sockfd);
    cout << "Closing connection" << endl;
    outputFile.close(); // close file
    free(receivedContent); // free dynamic memory
  }
  return 0;
}