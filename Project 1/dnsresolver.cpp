#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include "dnsresolver.h"

using namespace std;

string DNSresolver::resolveAddr(string hostName) {
	struct addrinfo hints;
	struct addrinfo* res;

	// prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // get address
  int status = 0;
  if ((status = getaddrinfo(hostName.c_str(), "http", &hints, &res)) != 0) {
    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
  }

  string IP_ADDRESS="";
  for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
    // convert address to IPv4 address
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

    // convert the IP to a string and print it:
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
    IP_ADDRESS = ipstr;
  }

freeaddrinfo(res); // free the linked list
  return IP_ADDRESS;
  
}