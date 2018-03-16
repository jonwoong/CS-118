/*
Jonathan Woong
804205763
CS 118 - DIS 1A
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include "responseBuilder.cpp"

#define MAX_CLIENT_SIZE 5
// MAX_BUFFER_SIZE = 1024

int main(int argc, char* argv[])
{
  ////////// PARSE ARGUMENTS //////////
  string host = argv[1]; // get hostname
  if (host == "") {
    host = "localhost";
  }

  string port = argv[2]; // get port number
  if (port == "") {
    port = "4000";
  }  

  string directory = argv[3]; // get directory
  if (directory == "") {
    directory = ".";
  }
  if (directory[directory.length()-1]!='/') { // if server directory doesnt end in /
    directory += "/";
  }
  if (chdir(directory.c_str())==-1) { // change server directory 
    cerr << "Error changing directory" << endl;
  }

	////////// RESOLVE DNS //////////
	struct addrinfo hints;
	struct addrinfo* res;
	// prepare hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	// get address
	int status = 0;
	if ((status = getaddrinfo(host.c_str(), "http", &hints, &res)) != 0) {
  	std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
  	return 2;
	}
	char* IPaddr;
	for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
  	// convert address to IPv4 address
  	struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
  	// convert the IP to a string and print it:
  	char ipstr[INET_ADDRSTRLEN] = {'\0'};
  	inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
  	IPaddr=ipstr;
	}
  freeaddrinfo(res); // free the linked list

  ////////// CREATE SOCKET //////////
  int masterSocket; // server's main socket
  int maxSocket; // highest socket #
  int newSocket; // new connection
  int clientSockfd; // client socket
  int tru = true;
  fd_set socketFDs; // set of socket fds
  int clientSockets[30]; // array of client fds
  for (int i=0; i< MAX_CLIENT_SIZE; i++) {
    clientSockets[i]=0;
  }
  // create socket
  if ((masterSocket = socket(AF_INET,SOCK_STREAM,0)) == 0) {
    cerr << "Error creating master socket" << endl;
  }
  // set socket timeout
  struct timeval timeOut;
  timeOut.tv_sec = 10; // 10 second timeout
  // allow multiple connections/reuse 
  
  if (setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR,(char*)&tru,sizeof(tru)) < 0) {
    cerr << "Error allowing multiple connections to master socket" << endl;
  }

  ////////// BIND //////////
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(stoi(port));     // short, network byte order
  addr.sin_addr.s_addr = inet_addr(IPaddr);
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (::bind(masterSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  ////////// LISTEN //////////
  if (listen(masterSocket, MAX_CLIENT_SIZE) == -1) { // allow for multiple clients to connect
    perror("listen");
    return 3;
  }

  ////////// DATA MEMBERS USED IN PARSING //////////
  char buf[MAX_BUFFER_SIZE] = {0}; // incoming request chunk
  stringstream ss; 

   ////////// ACCEPT CONNECTION //////////
  struct sockaddr_in clientAddr; 
  socklen_t clientAddrSize = sizeof(clientAddr);

  ////////// MAIN STUFF //////////
  while (1) {
    // clear sockets
    FD_ZERO(&socketFDs);
    // add master socket to set
    FD_SET(masterSocket, &socketFDs);
    maxSocket = masterSocket;

    // add child sockets
    for (int i=0; i< MAX_CLIENT_SIZE; i++) {
      clientSockfd = clientSockets[i];
      if (clientSockfd>0) { // if valid sockfd, add to read list
        FD_SET(clientSockfd,&socketFDs);
      }
      if (clientSockfd>maxSocket) {
        maxSocket = clientSockfd; // get highest socket #
      }
    }

    // wait for socket to become active
    int active = select(maxSocket+1, &socketFDs, NULL, NULL, NULL); 
    if ((active<0) && (errno!=EINTR)) {
      cerr << "Error selecting socket" << endl;
    }

    // incoming connection
    if (FD_ISSET(masterSocket, &socketFDs)) {
      if ((newSocket = accept(masterSocket, (struct sockaddr*)&clientAddr, &clientAddrSize))<0) {
        cerr << "Could not accept incoming connection" << endl;
      }
      /////////// PRINT ACCEPTED CONNECTION ///////////
      char ipstr[INET_ADDRSTRLEN] = {'\0'};
      inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
      cout << "Accept a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;

      // add new socket to array
      for (int i=0; i<MAX_CLIENT_SIZE; i++) {
        if (clientSockets[i]==0) { // empty
          clientSockets[i] = newSocket;
          break;
        }
      }
    }

    // I/O on other sockets
    for (int i=0; i<MAX_CLIENT_SIZE; i++) {
      clientSockfd = clientSockets[i];

      if (FD_ISSET(clientSockfd, &socketFDs)) {
          // receive client message
          if (recv(clientSockfd, buf, MAX_BUFFER_SIZE, 0) == -1) {
            perror("recv");
            return 5;
          }
            ////////// PARSE REQUEST //////////
            // capture incoming message
            ss << buf << endl;
            // convert incoming message to string
            string buffer(buf);
            // initiate responseBuilder to generate response message
            responseBuilder buildResponse(buffer);
            // parse the request message
            buildResponse.parseRequest();
            // get file name
            string fileName = buildResponse.getFileName();
            // construct header for response message
            string responseHeader = buildResponse.generateResponseHeader();

            ////////// SEND HEADER //////////
            send(clientSockfd, responseHeader.c_str(), responseHeader.length(), 0);

            ////////// CONSTRUCT DATA SECTION OF MESSAGE //////////
            // get path to file
            // convert file contents to string
            ifstream sourceFile(fileName.c_str());
            if (!sourceFile) {
              perror("Could not find file");
            }
            stringstream fileContainer;
            fileContainer << sourceFile.rdbuf(); // copy file contents to stringstream
            sourceFile.close(); // no longer used
            string sourceFileInString = fileContainer.str(); // capture file contents as string
            // calculate file size
            int sourceFileSize = sourceFileInString.length();
            
            ////////// SEND FILE //////////
            FILE* requestedFile;
            requestedFile = fopen(fileName.c_str(),"r"); // open file for reading
            if (requestedFile==NULL) {
              cerr << "Could not find file" << endl;
            }
            int sizeCheck=0; // keeps track of how many bytes have sent so far
            char* sendContents; // buffer
            sendContents = (char*)malloc(sourceFileSize); // buffer to hold file
            
              ////////// SEND IN CHUNKS /////////
              while (sizeCheck < sourceFileSize) {
              int amountRead = fread(sendContents,sizeof(char),MAX_BUFFER_SIZE,requestedFile);
              int amountSent = send(clientSockfd,sendContents,amountRead,0);
              sizeCheck += amountSent;
              cout << sizeCheck << endl;
              if (sizeCheck==sourceFileSize) {
                free(sendContents);
                fclose(requestedFile); // no longer used
                close(clientSockfd);
                clientSockets[i]=0;
                cout << "Successfully sent file: " << fileName << endl;
                break;
                }
              }
        }
      }     
    }
  return 0;
}
