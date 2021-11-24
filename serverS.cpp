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
    map<string,int> m; 		//map created based on string as key and score as value
    
    int numVertices = 0;	//total no.of vertices
	int Vertno = 0;

    fstream fs(file_name);

    struct score_map{    	//struct to convert map to struct to be sent to central
		int score_value;
		char names[512];
	}obj_score[400];

// // get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
// 	if (sa->sa_family == AF_INET) {
// 		return &(((struct sockaddr_in*)sa)->sin_addr);
// 	}

// 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

//function to connect to central through UDP socket and send the scores
//snippets from Beej's guide is used here to establish connection and use sendto
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
		
	//send map as struct objs
		for (auto x = 0; x < numVertices; x++){
			if ((numbytes = sendto(sockfd_to_central, (char*) &obj_score[x], sizeof(score_map), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
				// printf("talker: sent %d bytes to central\n", numbytes);
		}

		freeaddrinfo(servinfo);
		close(sockfd_to_central);
}

//function that reads the contents of score.txt and forms a map based on the names and score
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

	    // //sample display
	    map<string,int>::iterator i;
	    // for(i=m.begin();i!=m.end();i++) {
	    // 	cout<<i->first<<"\t"<<i->second<<endl;
	    // }

    fs.close();

	//add the map values on to a struct obj - total numVertices
	for(i=m.begin(); i!=m.end(), Vertno < numVertices; Vertno++, i++){
		obj_score[Vertno].score_value = i->second;
		strcpy(obj_score[Vertno].names, i->first.c_str());
	}
		// //sample display
		// for(Vertno = 0; Vertno < numVertices; Vertno++){
		// 	cout<<obj_score[Vertno].score_value<<"\t";
		// 	printf("%s\n",obj_score[Vertno].names);
		// }
}

// the main function 
// Snippets from Beej's guide are used for binding the UDP socket and listen
int main(){  
     
	generate_score_map();	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
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

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		buf[numbytes] = '\0';

	// received the two usernames up until this point
		printf("The ServerS received a request from Central to get the scores.\n");

//send the score map
		Connect_to_Central_to_send_score();	
		printf("The ServerS finished sending the scores to Central.\n");
	}

    return 1;
	
}