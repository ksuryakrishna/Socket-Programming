/*
** client.c -- a stream socket client demo
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "26716" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

	struct numVP{
		int numVinP;
		float match_gap;
	}NVP;

	struct vert_in_path{
		char names[512];
	}VIP[400];

using namespace std;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//send username to serverC here
	if(send(sockfd, argv[1], sizeof argv[1], 0) == -1){
		perror("Error sending clientB Name to ServerC");
	}
	cout << "Waiting to receive\n";
	if ((numbytes = recv(sockfd, (char*) &NVP, sizeof(NVP), 0)) == -1) {
	    perror("recv: from ServerC NVP");
	    exit(1);
	}

	cout << "NumVinP: " << NVP.numVinP << endl;

	if(NVP.numVinP != -1){
		cout<<"PATH: ";
		for (auto x = 0; x < NVP.numVinP; x++){
			if ((numbytes = recv(sockfd, (char*) &VIP[x], sizeof(vert_in_path), 0)) == -1) {
			    perror("recv: from ServerC VIP");
			    exit(1);
			}
		}
	}
	else{
		cout << "PATH not found";
	}

	int k = 0;
	for(k = 0; k < NVP.numVinP - 1; k++){
		cout << VIP[k].names << "--->";
	}

	cout << VIP[k].names << endl;

	cout <<  "Matching GAP = " << NVP.match_gap;

	close(sockfd);

	return 0;
}

