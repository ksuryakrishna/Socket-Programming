/*
** clientA.cpp:
		- Sends 1 username to the central and waits for the results
		- Prints the path and matching gap(if available), else says not compatabile
		- closes the TCP socket after receving the results
	- by Surya Krishna Kasiviswanathan, USC ID: 9083261716
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

#define PORT "25716" // the port client will be connecting to 

#define MAXDATASIZE 512 // max number of bytes we can get at once  


	struct numVP{ 			//struct to receive the no.of paths in the received path and the matching gap
		int numVinP;
		float match_gap;
	}NVP1, NVP2;

	struct vert_in_path{	//struct to receive the names in the resultant path(s)
		char names[512];
	}VIP1[400], VIP2[400];

	struct index_matrix{	//struct to get error flags if invalid name
		int indexA;
		int indexB;
		int indexC;
	}index_m;

char clientB_Name1[512], clientB_Name2[512], matching_gap_to_2_dec[40];

using namespace std;

char numUsernames[] = "1";  //default username count from B is taken as 1

// get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
// 	if (sa->sa_family == AF_INET) {
// 		return &(((struct sockaddr_in*)sa)->sin_addr);
// 	}

// 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

//snippets from the Beej guide are used here to setup the tcp socket
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

	printf("The client is up and running.\n");

		// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		// 		s, sizeof s);
		// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//send username to serverC here
	if(send(sockfd, argv[1], sizeof argv[1], 0) == -1){
		perror("Error sending clientA Name to ServerC");
	}
	printf("The client sent %s to the Central server.\n", argv[1]);

		// cout << "Waiting to receive\n";	

	if ((numbytes = recv(sockfd, numUsernames, 1, 0)) == -1) {
	    perror("recv: from ServerC NVP");
	    exit(1);
	}
		//cout << "numUsernames: " <<numUsernames[0]<<endl;

	if ((numbytes = recv(sockfd, (char*) &index_m, sizeof(index_m), 0)) == -1) {
	    perror("recv: from ServerC NVP");
	    exit(1);
	}

	if(index_m.indexA == -2 || index_m.indexB == -2 || index_m.indexC == -2){
		perror("ENTERED INVALID NAME...EXITING PROGRAM");
		exit(1);
	}

	if(numUsernames[0] == '1'){  //when there is only one username from B

		if ((numbytes = recv(sockfd, (char*) &NVP1, sizeof(NVP1), 0)) == -1) {
		    perror("recv: from ServerC NVP");
		    exit(1);
		}

			//cout << "NumVinP: " << NVP1.numVinP << endl;

		if(NVP1.numVinP != -1){
			
			for (auto x = 0; x < NVP1.numVinP; x++){
				if ((numbytes = recv(sockfd, (char*) &VIP1[x], sizeof(vert_in_path), 0)) == -1) {
				    perror("recv: from ServerC VIP");
				    exit(1);
				}
			}
			printf("Found compatibility for %s and %s:\n", VIP1[0].names, VIP1[NVP1.numVinP - 1].names);
			//print the path
			int k = 0;
			for(k = 0; k < NVP1.numVinP - 1; k++){
				cout << VIP1[k].names << " --- ";
			}

			cout << VIP1[k].names << endl;

			//print the matching gap
			sprintf(matching_gap_to_2_dec, "%0.02f", NVP1.match_gap);
			cout <<  "Matching Gap: " << matching_gap_to_2_dec << endl;
			memset(matching_gap_to_2_dec, 0, sizeof(matching_gap_to_2_dec));

		}
		else{
			if ((numbytes = recv(sockfd, clientB_Name1, sizeof clientB_Name1, 0)) == -1) {
			    perror("recv: from ServerC VIP");
			    exit(1);
			}
			printf("Found no compatibility for %s and %s\n", argv[1], clientB_Name1);
		}

		close(sockfd);  //close the socket
	}



	else if (numUsernames[0] == '2'){	//when there are 2 usernames from B
//1st path
		if ((numbytes = recv(sockfd, (char*) &NVP1, sizeof(NVP1), 0)) == -1) {
		    perror("recv: from ServerC NVP");
		    exit(1);
		}

			//cout << "NumVinP: " << NVP1.numVinP << endl;

		if(NVP1.numVinP != -1){	//if compatibility found
			// cout<<"PATH: ";
			for (auto x = 0; x < NVP1.numVinP; x++){
				if ((numbytes = recv(sockfd, (char*) &VIP1[x], sizeof(vert_in_path), 0)) == -1) {
				    perror("recv: from ServerC VIP");
				    exit(1);
				}
			}
			printf("Found compatibility for %s and %s:\n", VIP1[0].names, VIP1[NVP1.numVinP - 1].names);
			//print path
			int k = 0;
			for(k = 0; k < NVP1.numVinP - 1; k++){
				cout << VIP1[k].names << " --- ";
			}

			cout << VIP1[k].names << endl;

			//print matching gap
			sprintf(matching_gap_to_2_dec, "%0.02f", NVP1.match_gap);
			cout <<  "Matching Gap: " << matching_gap_to_2_dec << endl;
			memset(matching_gap_to_2_dec, 0, sizeof(matching_gap_to_2_dec));

		}
		else{
			if ((numbytes = recv(sockfd, clientB_Name1, sizeof clientB_Name1, 0)) == -1) {
			    perror("recv: from ServerC VIP");
			    exit(1);
			}
			printf("Found no compatibility for %s and %s\n", argv[1], clientB_Name1);
		}
//2nd path
		if ((numbytes = recv(sockfd, (char*) &NVP2, sizeof(NVP2), 0)) == -1) {
		    perror("recv: from ServerC NVP");
		    exit(1);
		}

			// cout << "NumVinP: " << NVP2.numVinP << endl;

		if(NVP2.numVinP != -1){	//if compatibility found
			// cout<<"PATH: ";
			for (auto x = 0; x < NVP2.numVinP; x++){
				if ((numbytes = recv(sockfd, (char*) &VIP2[x], sizeof(vert_in_path), 0)) == -1) {
				    perror("recv: from ServerC VIP");
				    exit(1);
				}
			}
			printf("Found compatibility for %s and %s:\n", VIP2[0].names, VIP2[NVP2.numVinP - 1].names);
			//print path
			int k = 0;
			for(k = 0; k < NVP2.numVinP - 1; k++){
				cout << VIP2[k].names << " --- ";
			}

			cout << VIP2[k].names << endl;

			//print matching gap
			sprintf(matching_gap_to_2_dec, "%0.02f", NVP2.match_gap);
			cout <<  "Matching Gap: " << matching_gap_to_2_dec << endl;
			memset(matching_gap_to_2_dec, 0, sizeof(matching_gap_to_2_dec));

		}
		else{
			if ((numbytes = recv(sockfd, clientB_Name2, sizeof clientB_Name2, 0)) == -1) {
			    perror("recv: from ServerC VIP");
			    exit(1);
			}
			printf("Found no compatibility for %s and %s\n", argv[1], clientB_Name2);
		}


		close(sockfd);  //close the socket
	}

	return 0;
}

