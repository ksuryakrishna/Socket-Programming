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

#define MYPORT "22716"	// the port users will be connecting to

#define CENTRAL_PORT "24716" //the port S uses to connect to central

#define MAXBUFLEN 512

	int sockfd_binded, sockfd_to_central;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	string st1, st2;
	string file_name = "scores.txt";
    map<string,int> m; 
    
    int numVertices = 0;
	int Vertno = 0;

    fstream fs(file_name);

    struct score_map{
		int score_value;
		char names[512];
	}obj_score[400];

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
void generate_score_map(){

	while(fs>>st1){
	    	//cout<<s<<endl;
    	if(fs>>st2){
		    if(!m.count(st1)){
		    	m.insert(make_pair(st1,stoi(st2)));
		    }
		    numVertices += 1;
		}
    }

    //sample display
    map<string,int>::iterator i;
    for(i=m.begin();i!=m.end();i++) {
    	cout<<i->first<<"\t"<<i->second<<endl;
    }

    fs.close();

	//add the map values on to a struct obj - total numVertices
	for(i=m.begin(); i!=m.end(), Vertno < numVertices; Vertno++, i++){
		obj_score[Vertno].score_value = i->second;
		strcpy(obj_score[Vertno].names, i->first.c_str());
	}
	//sample display
	for(Vertno = 0; Vertno < numVertices; Vertno++){
		cout<<obj_score[Vertno].score_value<<"\t";
		printf("%s\n",obj_score[Vertno].names);
	}
}

int main(){  
     
	generate_score_map();	

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

	printf("The ServerS is up and running using UDP on port 22716.\n");

	while(1){

		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);

	// received the two usernames up until this point
		printf("The ServerS received a request from Central to get the scores.\n");
		
		//sample msg sent as reply to central server
		// if ((numbytes = sendto(sockfd, a, strlen(a), 0,
		// 		 p->ai_addr, p->ai_addrlen)) == -1){
		// 	perror("T sends to central error: sendto");
		// 	exit(1);
		// }	
		//close(sockfd_binded);  //finally remove this, dont close because server should be 
		//continously listening on the binded socket


		Connect_to_Central_to_send_score();	
		printf("The ServerS finished sending the scores to Central.\n");
	}

    return 1;
	
}