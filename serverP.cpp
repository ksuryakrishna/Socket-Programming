#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <map>
#include <fstream>
#include <cstring>

using namespace std; 

#define MYPORT "23716"	// the port users will be connecting to

#define CENTRAL_PORT "24716" //the port S uses to connect to central

#define MAXBUFLEN 100

	int sockfd_binded, sockfd_to_central;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	string st1, st2;
    map<string,int> m; 
    
    // int numVertices = 0;
	int Vertno = 0;


    int length_1D = 0; //variable that contains the 1D length of adj matrix

    struct numV{	
    	int numstruct;
    }numobj;

	struct convert_map_to_struct{
		int keyvalue;
		char names[512];
	}obj[400];

	struct adj_matrix{
		int adj_m[160000];
	}adj;

	struct score_map{
		int score_value;
		char names[512];
	}obj_score[400];

    int numVertices = 0; //get this from Central through numV struct

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Connect_to_Central_to_send_score(){
	//add content of talker here
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET; 
		hints.ai_socktype = SOCK_DGRAM;

		if ((rv = getaddrinfo("127.0.0.1", CENTRAL_PORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return;
		}

		// loop through all the results and make a socket
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd_to_central = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
				perror("talker: socket");
				continue;
			}

			break;
		}

		if (p == NULL) {
			fprintf(stderr, "talker: failed to create socket\n");
			return;
		}
		// obj.a = 1;
		// obj.b = 2;
		// strcpy(obj.names, "Amma");
		// for(auto p = 0;p<10;p++)
		// 	for(auto q = 0; q<3;q++)
		// 		obj.arr[p][q] = p;

	// //send numvertices
	// 	if ((numbytes = sendto(sockfd_to_central, (char*) &numobj, sizeof(numobj), 0,
	// 			 p->ai_addr, p->ai_addrlen)) == -1) {
	// 		perror("talker: sendto");
	// 		exit(1);
	// 	}
	// 	printf("talker: sent %d bytes to central\n", numbytes);
		
	//send map as struct objs
		for (auto x = 0; x < numVertices; x++){
			if ((numbytes = sendto(sockfd_to_central, (char*) &obj_score[x], sizeof(score_map), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			printf("talker: sent %d bytes to central\n", numbytes);
		}
		
			
	// //send adjacency matrix
	// 	cout<<"Going to send matrix";
	// 	if ((numbytes = sendto(sockfd_to_central, (char*) &adj, 65000, 0,
	// 			 p->ai_addr, p->ai_addrlen)) == -1) {
	// 		perror("talker: sendto");
	// 		exit(1);
	// 	}

		freeaddrinfo(servinfo);

		// printf("talker: sent %d bytes to central\n", numbytes);
		close(sockfd_to_central);

}

void Connect_to_Central_to_send_results(){


}

void Recv_from_central(){

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_binded, (char*) &numobj, sizeof(numobj)/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	numVertices = numobj.numstruct;
	cout << "numobj.numstruct (numVertices) = "<<numVertices<<endl;

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);

	//get ready to receive the adjacency matrix and the map in the form struct objects
	printf("listener: waiting to recv map...\n");
	//char int_buffer[25];	
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, (char*) &obj[x], sizeof(convert_map_to_struct)/*MAXBUFLEN-1*/, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
	cout<<"going to display received map \n";
	//sample display 
	for(auto x = 0; x<numVertices;x++){
		cout<<x<<": \t" <<obj[x].keyvalue<<"  "<<obj[x].names<<endl;
	}
	//now get the matrix
	printf("listener: waiting to recv adjacency matrix...\n");

	if ((numbytes = recvfrom(sockfd_binded, (char*) &adj, 65000/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	length_1D = numVertices*numVertices;  //remember to use this when you send to P
	//sample display
	cout<<"Received the matrix as 1D\n";
	for(auto x = 0; x < length_1D; x++){
		cout<<adj.adj_m[x]<<" ";
	}
	cout<<endl;	

	//score from Central
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, (char*) &obj_score[x], sizeof(score_map)/*MAXBUFLEN-1*/, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
	cout<<"going to display received score map \n";
	//sample display 
	for(auto x = 0; x<numVertices;x++){
		cout<<x<<": \t" <<obj_score[x].score_value<<"  "<<obj_score[x].names<<endl;
	}


	Connect_to_Central_to_send_results();

}

int main(){  
     
		

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo("127.0.0.1", MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd_binded = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd_binded, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd_binded);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom CENTRAL SERVER...\n");

	Recv_from_central();

	// addr_len = sizeof their_addr;
	// if ((numbytes = recvfrom(sockfd_binded, buf, MAXBUFLEN-1 , 0,
	// 	(struct sockaddr *)&their_addr, &addr_len)) == -1) {
	// 	perror("recvfrom");
	// 	exit(1);
	// }

	// printf("listener: got packet from %s\n",
	// 	inet_ntop(their_addr.ss_family,
	// 		get_in_addr((struct sockaddr *)&their_addr),
	// 		s, sizeof s));
	// printf("listener: packet is %d bytes long\n", numbytes);
	// buf[numbytes] = '\0';
	// printf("listener: packet contains \"%s\"\n", buf);

// received the two usernames up until this point

	
	//sample msg sent as reply to central server
	// if ((numbytes = sendto(sockfd, a, strlen(a), 0,
	// 		 p->ai_addr, p->ai_addrlen)) == -1){
	// 	perror("T sends to central error: sendto");
	// 	exit(1);
	// }	
	close(sockfd_binded);  //finally remove this, dont close because server should be 
	//continously listening on the binded socket


	



    return 1;
	
}