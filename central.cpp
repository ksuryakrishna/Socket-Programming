/*
** central.cpp
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>

#define PORT1 "25716"  // TCP1 - the port users will be connecting to

#define PORT2 "26716" // TCP2 - the port users will be connecting to

#define SERVER_T_PORT "21716"	// the port central server uses to connect to T

#define SERVER_S_PORT "22716"	// the port central server uses to connect to S

#define SERVER_P_PORT "23716"	// the port central server uses to connect to P

#define MY_UDP_PORT "24716"  //the port serverT will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

using namespace std;

	int sockfd1, sockfd2, new_fd1, new_fd2;  // TCP sockets - listen on sockfd1 and sockfd2, new connection on new_fd
	int sockfdS, sockfdT, sockfdP, sockUDP_binded;  //UDP sockets on the server side
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, addr_len;
	struct sigaction sa;

	char s[INET6_ADDRSTRLEN];
	char clientA_Name[512], clientB_Name1[512], clientB_Name2[512];
	int yes=1;			// for setsockopt() SO_REUSEADDR, below
	int i, j, rv, res;

	//for select inclusion
	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    char buf[512];    // buffer for client data
    int nbytes;
	int pathfound1 = 0, pathfound2 = 0;  //-1 - path not found, 1- path found
    int clientA_rec = 0, clientB_rec = 0;
    int clientA_done = 0, clientB_done = 0;
    //end - select inclusion declaration

    int length_1D = 0; //variable that contains the 1D length of adj matrix

    struct numV{	//struct to get total number of vertices in the graph received
    	int numstruct;
    }numobj;

	struct convert_map_to_struct{	//struct to store data of a map of names and their indices
		int indexvalue;
		char names[512];
	}obj[400];

	struct adj_matrix{			//the adjacency matrix - received as 1D struct and recreated as 2D in server P
		uint8_t adj_m[160000];
	}adj;

	struct score_map{		//struct to receive the score map as struct from server S
		int score_value;
		char names[512];
	}obj_score[400];

	struct index_matrix{	//struct to get the index of the clients in the graph received
		int indexA;
		int indexB;
		int indexC;
	}index_m;

	struct numVP{	//struct to receive the no.of paths in the received path and the matching gap
		int numVinP;
		float match_gap;
	}NVP1, NVP2;

	struct vert_in_path{	//struct to receive the names in the resultant path(s)
		char names[512];
	}VIP1[400], VIP2[400];

	struct Bnames{		//struct to store clientB username(s)
		char name[512];
	}Bnamesobj1, Bnamesobj2;

    int numVertices = 0; //get this from T through numV struct

    char numUsernames[] = "1";	//default username count from B is taken as 1

void sigchld_handler(int s)  //called by reap_dead_processes()
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// // get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
// 	if (sa->sa_family == AF_INET) {
// 		return &(((struct sockaddr_in*)sa)->sin_addr);
// 	}

// 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

//snippet from Beej's guide to setup, bind and listen on the socket
void bind_listen_clientA_parent(){

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd1 = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket1");
			continue;
		}

		if (setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt1");
			exit(1);
		}

		if (bind(sockfd1, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd1);
			perror("server: bind1");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind1\n");
		exit(1);
	}

	if (listen(sockfd1, BACKLOG) == -1) {
		perror("listen1");
		exit(1);
	}
}

//snippet from Beej's guide to setup, bind and listen on the socket
void bind_listen_clientB_parent(){

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT2, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd2 = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket2");
			continue;
		}

		if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt2");
			exit(1);
		}

		if (bind(sockfd2, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd2);
			perror("server: bind2");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind2\n");
		exit(1);
	}

	if (listen(sockfd2, BACKLOG) == -1) {
		perror("listen2");
		exit(1);
	}

	printf("The Central server is up and running.\n");
}

//snippet from Beej to clear all dead processes
void reap_all_dead_process(){

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}

//function to receive results from server P
void Receive_Path_MG_from_ServerP(){

		// printf("listener: waiting to recvfrom serverP...\n");

	//receive the no. of vertices in this path
	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &NVP1, sizeof(NVP1), 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	if(NVP1.numVinP != -1){	//if compatibility found
		pathfound1 = 1;
		for(auto x = 0; x < NVP1.numVinP; x++){
			addr_len = sizeof their_addr;
			if ((nbytes = recvfrom(sockUDP_binded, (char*) &VIP1[x], sizeof(vert_in_path), 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
		}
			//cout<<"going to display received path for username1\n";
				// //sample display 
				// for(auto x = 0; x < NVP1.numVinP; x++){
				// 	cout<<x<<": \t" <<VIP1[x].names<<endl;
				// }		
	}
	else{
		pathfound1 = -1;
			// cout<<"Path not found by serverT";
	}

	if(index_m.indexC != -1 && index_m.indexC != -2){ //path for 2nd username

		addr_len = sizeof their_addr;
		//receive the no. of vertices in this path
		if ((nbytes = recvfrom(sockUDP_binded, (char*) &NVP2, sizeof(NVP2), 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		if(NVP2.numVinP != -1){//if compatibility found
			pathfound2 = 1;
			for(auto x = 0; x < NVP2.numVinP; x++){
				addr_len = sizeof their_addr;
				if ((nbytes = recvfrom(sockUDP_binded, (char*) &VIP2[x], sizeof(vert_in_path), 0,
					(struct sockaddr *)&their_addr, &addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}
			}
				//cout<<"going to display received path for username2 \n";
				// //sample display 
				// for(auto x = 0; x < NVP2.numVinP; x++){
				// 	cout<<x<<": \t" <<VIP2[x].names<<endl;
				// }		
		}
		else{
			pathfound2 = -1;
				// 	cout<<"Path not found by serverT for 2nd username";
		}

	}

	printf("The Central server received the results from backend server P.\n");

}

//function to connect to server P through UDP
//Snippets from Beej's guide are used for the connection and sendto
void Connect_to_ServerP(){

	sockfdP = 0; //used for UDP connection between central and P

		// cout << "Entered connect to serverP\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("127.0.0.1", SERVER_P_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdP = socket(p->ai_family, p->ai_socktype,
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

		{
		//send numvertices
			if ((nbytes = sendto(sockfdP, (char*) &numobj, sizeof(numobj), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
				// printf("talker: sent %d bytes to P\n", nbytes);
			
		//send map as struct objs
			// cout<<"Going to send index map";
			for (auto x = 0; x < numVertices; x++){
				if ((nbytes = sendto(sockfdP, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
			}
				// printf("talker: sent %d bytes to P\n", nbytes);
				
		//send adjacency matrix as 1d
			// cout<<"Going to send matrix\n";
			if ((nbytes = sendto(sockfdP, (char*) &adj, 65000, 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

		//send score map
			// cout<<" Going to send score_map\n";
		//send map as struct objs
			for (auto x = 0; x < numVertices; x++){
				if ((nbytes = sendto(sockfdP, (char*) &obj_score[x], sizeof(score_map), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
				// printf("talker: sent %d bytes to P\n", nbytes);
			}

			// cout<<"going to send index list\n";

			if ((nbytes = sendto(sockfdP, (char*) &index_m, sizeof(index_matrix), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			// printf("talker: sent %d bytes to P\n", nbytes);			
		}

	printf("The Central server sent a processing request to Backend-Server P.\n");
	
	freeaddrinfo(servinfo);
	close(sockfdP);
	/*Sending to Server S is done. Now wait for reply of
	graphs and list of nodes*/

	Receive_Path_MG_from_ServerP();
}

//function to receive the scores from server S and store it in a struct called score map
void Receive_score_from_ServerS(){

	// printf("listener: waiting to recvfrom serverS...\n");
	
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((nbytes = recvfrom(sockUDP_binded, (char*) &obj_score[x], sizeof(score_map), 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
		// cout<<"going to display received score map \n";
		// //sample display 
		// for(auto x = 0; x<numVertices;x++){
		// 	cout<<x<<": \t" <<obj_score[x].score_value<<"  "<<obj_score[x].names<<endl;
		// }

	printf("The Central server received information from Backend-Server S using UDP over port %s. \n", MY_UDP_PORT);
	
	Connect_to_ServerP();
	// close(sockUDP_binded);   //dont close serverP needs it


}

//Function to connect to ServerS
//snippets from Beej's guide is used for connection and sendto
void Connect_to_ServerS(){

	sockfdS = 0; //used for UDP connection between central and S
	char score_req[] = "RFS"; //request for score

		// cout << "Entered connect to serverS\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("127.0.0.1", SERVER_S_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdS = socket(p->ai_family, p->ai_socktype,
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

	if ((nbytes = sendto(sockfdS, score_req, strlen(score_req), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
		//printf("talker: sent %d bytes to server T\n", nbytes);

	printf("The Central server sent a request to Backend-Server S.\n");

	freeaddrinfo(servinfo);
	close(sockfdS);
	/*Sending to Server S is done. Now wait for reply of
	score map*/

	Receive_score_from_ServerS();
}

//function to send the results to clientA 
void Send_Results_to_ClientA(){

	//have to let A know if there are 2 usernames from B
	if((nbytes = send(new_fd1, numUsernames, 1, 0)) == -1){
		perror("Error sending numUsernames to clientA");
	}
		// printf("talker: sent %d bytes to clientA\n", nbytes);
	//index used as a flag to let A know if there is an issue with the input given
	if((nbytes = send(new_fd1, (char*) &index_m, sizeof(index_m), 0)) == -1){
		perror("Error sending index to clientA");
	}
		// printf("talker: sent %d bytes to clientA\n", nbytes);
	//send num of vertices in this path
	if((nbytes = send(new_fd1, (char*) &NVP1, sizeof(NVP1), 0)) == -1){
		perror("Error sending NVP to clientA");
	}
		// printf("talker: sent %d bytes to clientA\n", nbytes);

	if(pathfound1 == 1){
		for (auto x = 0; x < NVP1.numVinP; x++){ //send the vertices in the path 1
			if ((nbytes = send(new_fd1, (char*) &VIP1[x], sizeof(vert_in_path), 0)) == -1) {
				perror("central to clientA error");
				exit(1);
			}
				// printf("talker: sent %d clientA\n", nbytes);
		}
	}
	else{ //if no pathfound, then send the clientB name so that A knows
		//send clientB name
		if ((nbytes = send(new_fd1, clientB_Name1, sizeof clientB_Name1, 0)) == -1) {
			perror("central to clientA error");
			exit(1);
		}		

	}

	if(index_m.indexC != -1 && index_m.indexC != -2){ //if 2 usernames and path exists

		if((nbytes = send(new_fd1, (char*) &NVP2, sizeof(NVP2), 0)) == -1){
			perror("Error sending NVP to clientB");
		}
			// printf("talker: sent %d bytes to clientB\n", nbytes);

		if(pathfound2 == 1){
			for (auto x = 0; x < NVP2.numVinP; x++){ //send vertices in path 2
				if ((nbytes = send(new_fd1, (char*) &VIP2[x], sizeof(vert_in_path), 0)) == -1) {
					perror("central to clientB error");
					exit(1);
				}
				// printf("talker: sent %d clientB\n", nbytes);
			}
		}
		else{
			//send clientB name
			if ((nbytes = send(new_fd1, clientB_Name2, sizeof clientB_Name2, 0)) == -1) {
				perror("central to clientB error");
				exit(1);
			}		

		}		
	}

	printf("The Central server sent the results to client A.\n");
	close(new_fd1); //tear down the tcp child socket not needed anymore
}

//function to send the results to clientB
void Send_Results_to_ClientB(){

//index used as a flag to let B know if there is an issue with the input given
	if((nbytes = send(new_fd2, (char*) &index_m, sizeof(index_m), 0)) == -1){
		perror("Error sending index to clientB");
	}
		// printf("talker: sent %d bytes to clientB\n", nbytes);
	//num of vertices in path 1
	if((nbytes = send(new_fd2, (char*) &NVP1, sizeof(NVP1), 0)) == -1){
		perror("Error sending NVP to clientB");
	}
		// printf("talker: sent %d bytes to clientB\n", nbytes);

	if(pathfound1 == 1){ //send the path in reverse order
		for (auto x = NVP1.numVinP - 1; x >= 0; x--){
			if ((nbytes = send(new_fd2, (char*) &VIP1[x], sizeof(vert_in_path), 0)) == -1) {
				perror("central to clientB error");
				exit(1);
			}
			// printf("talker: sent %d clientB\n", nbytes);
		}
	}
	else{
		//send clientA name to B
		if ((nbytes = send(new_fd2, clientA_Name, sizeof clientA_Name, 0)) == -1) {
			perror("central to clientB error");
			exit(1);
		}		

	}

	if(index_m.indexC != -1 && index_m.indexC != -2){//if 2 usernames and path exists
		//num of vertices in this path
		if((nbytes = send(new_fd2, (char*) &NVP2, sizeof(NVP2), 0)) == -1){
			perror("Error sending NVP to clientB");
		}
		// 	printf("talker: sent %d bytes to clientB\n", nbytes);

		if(pathfound2 == 1){//send the path in reverse order
			for (auto x = NVP2.numVinP - 1; x >= 0; x--){
				if ((nbytes = send(new_fd2, (char*) &VIP2[x], sizeof(vert_in_path), 0)) == -1) {
					perror("central to clientB error");
					exit(1);
				}
					// printf("talker: sent %d clientB\n", nbytes);
			}
		}
		else{
			//send clientB name
			if ((nbytes = send(new_fd2, clientA_Name, sizeof clientA_Name, 0)) == -1) {
				perror("central to clientB error");
				exit(1);
			}		

		}		
	}	

	printf("The Central server sent the results to client B.\n");
	close(new_fd2);
	clientA_rec = 0; clientB_rec = 0;  //reset these flags before looking for other requests
	clientA_done = 0; clientB_done = 0;
}

//function to receive the required graph info from server T
//Snippets from Beej's guide are used to connect bind the UDP socket
void Receive_graph_from_ServerT(){

	sockUDP_binded = 0;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo("127.0.0.1", MY_UDP_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockUDP_binded = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockUDP_binded, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockUDP_binded);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return;
	}

	freeaddrinfo(servinfo);

		// printf("listener: waiting to recvfrom T...\n");
		
	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &numobj, sizeof(numobj), 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	numVertices = numobj.numstruct;
		// cout << "numobj.numstruct (numVertices) = "<<numVertices<<endl;

		// printf("listener: got packet from %s\n",
		// 	inet_ntop(their_addr.ss_family,
		// 		get_in_addr((struct sockaddr *)&their_addr),
		// 		s, sizeof s));
		// printf("listener: packet is %d bytes long\n", nbytes);

	//get ready to receive the adjacency matrix and the map in the form struct objects
		// printf("listener: waiting to recv map...\n");

	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((nbytes = recvfrom(sockUDP_binded, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
		// cout<<"going to display received map \n";
		// //sample display 
		// for(auto x = 0; x<numVertices;x++){
		// 	cout<<x<<": \t" <<obj[x].indexvalue<<"  "<<obj[x].names<<endl;
		// }
	
			// printf("listener: waiting to recv adjacency matrix...\n");
//now get the matrix
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &adj, 65000, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	length_1D = numVertices*numVertices;  //used when sent to P
		//sample display
		// cout<<"Received the matrix as 1D\n";
		// for(auto x = 0; x < length_1D; x++){
		// 	cout<<adj.adj_m[x]<<" ";
		// }
		// cout<<endl;

	//receive the index list to be used as flags and sent to P
	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &index_m, sizeof(index_matrix), 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	//if any name is not found on the graph, then display invalid name
	if(index_m.indexA == -2 || index_m.indexB == -2 || index_m.indexC == -2){

		perror("ENTERED INVALID NAME...EXITING PROGRAM, PLEASE RESTART the PROGRAM");
			Send_Results_to_ClientA();
			Send_Results_to_ClientB();
			close(sockUDP_binded);
		exit(1);
	}
		// cout<<"Received the indexs\n";

		// cout<<"indexA: "<<index_m.indexA<<"\t indexB: "<<index_m.indexB<<"\t indexC: "<<index_m.indexC<<endl;

	printf("The Central server received information from Backend-Server T using UDP over port %s.\n", MY_UDP_PORT);

	Connect_to_ServerS();
	//close(sockfd2); dont close because central server listens to serverS and serverP with this


}
 
//function used to connect to server T and send the client usernames
//snippets from Beej's guide are used to connect to serverT
void Connect_to_ServerT(){

	sockfdT = 0; //used for UDP connection between central and T

		//cout << "Entered connect to serverT\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("127.0.0.1", SERVER_T_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfdT = socket(p->ai_family, p->ai_socktype,
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
//send the number of usernames to be sent
	if ((nbytes = sendto(sockfdT, numUsernames, strlen(numUsernames), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
		// printf("talker: sent %d bytes to server T\n", nbytes);
//send the usernames
	if ((nbytes = sendto(sockfdT, clientA_Name, strlen(clientA_Name), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
		// printf("talker: sent %d bytes to server T\n", nbytes);

	if ((nbytes = sendto(sockfdT, clientB_Name1, strlen(clientB_Name1), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}	
		// printf("talker: sent %d bytes to server T\n", nbytes);

	if(numUsernames[0] == '2'){

		if ((nbytes = sendto(sockfdT, clientB_Name2, strlen(clientB_Name2), 0,
				 p->ai_addr, p->ai_addrlen)) == -1){
			perror("talker: sendto");
			exit(1);
		}	
		// printf("talker: sent %d bytes to server T\n", nbytes);		
	}

	printf("The Central server sent a request to Backend-Server T\n");

	freeaddrinfo(servinfo);
	close(sockfdT);
	/*Sending to Server T is done. Now bind socket and wait for reply of
	graphs and list of nodes*/

	Receive_graph_from_ServerT();
}

//the main function
 // -creates parent sockets and uses select function to select between 
 // new requests received to create child sockets
	//snippets of select is referred, modified and used according to the req. here

int main(void)
{

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
//create, bind, and listen to the clients using the 2 parent sockets
    bind_listen_clientA_parent();
    bind_listen_clientB_parent();

    reap_all_dead_process();

	//adding parent socks to master
	FD_SET(sockfd1, &master); 
	FD_SET(sockfd2, &master);

	//put the bigger socket to fdmax
	fdmax = (sockfd2 > sockfd1) ? sockfd2 : sockfd1;

	while(1){ //always keep looking for new requests coming from the clients

		read_fds = master; 
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			cout << "select error\n";
			exit(4);
		}

		// run through the existing parent sockets to find if there is any client waiting for connection
		if(FD_ISSET(sockfd1, &read_fds)){  //new request from client A
				//process clientA request
			sin_size = sizeof their_addr;
			new_fd1 = accept(sockfd1, (struct sockaddr *)&their_addr, &sin_size);

			if(new_fd1 == -1){
				cout << "Error in clientA accept";
				continue;
			}

				// inet_ntop(their_addr.ss_family,
				// 			get_in_addr((struct sockaddr *)&their_addr),
				// 			s, sizeof s);
				// printf("server: got connection from %s\n", s);	

			if (nbytes = recv(new_fd1, buf, sizeof buf, 0) <= 0){
				// if 0 socket closed other end, -1 is error
				if(nbytes == 0){
					cout << "clientA socket closed with ID: " << new_fd1;
				}else{
					perror("recv");
				}
				
			} else {
				//data received from client
				//store value in buf as clientA;
				strcpy(clientA_Name, buf);
				printf("%s\n",clientA_Name);

				printf("The Central server received input=%s from the client using TCP over port %s.\n", clientA_Name, PORT1);

				clientA_rec = 1;

				if (clientA_rec == 1 && clientB_rec == 1){ //inputs from both clients are received

					Connect_to_ServerT();
				}				
			}
					// cout<<clientA_rec<<"\tClient A msg received\n";	
			clientA_done = 1;
		}	

		if(FD_ISSET(sockfd2, &read_fds)){	//new request from client B
			//process clientB request
			sin_size = sizeof their_addr;
			new_fd2 = accept(sockfd2, (struct sockaddr *)&their_addr, &sin_size);

			if(new_fd2 == -1){
				cout << "Error in clientB accept";
				continue;
			}

				// inet_ntop(their_addr.ss_family,
				// 			get_in_addr((struct sockaddr *)&their_addr),
				// 			s, sizeof s);
				// printf("server: got connection from %s\n", s);

			//received 1 byte denoting the no.of usernames B is going to send
			if (nbytes = recv(new_fd2, buf, 1, 0) <= 0){
				// if 0 socket closed other end, -1 is error
				if(nbytes == 0){
					cout << "clientA socket closed with ID: " << new_fd2;
				}else{
					perror("recv");
				}

			} else {

				//receive numUsernames from B
				numUsernames[0] = buf[0];
				numUsernames[1] = '\0';

				// printf("numUsernames: %s\n", numUsernames);
				if(numUsernames[0] == '1'){ //if only one username from B

					if (nbytes = recv(new_fd2, (char*)&Bnamesobj1, sizeof(Bnames), 0) <= 0){
						// if 0 socket closed other end, -1 is error
						if(nbytes == 0){
							cout << "clientA socket closed with ID: " << new_fd2;
						}else{
							perror("recv");
						}
					}
					else{

						//data received from client
						//store value in buf as clientB;
						strcpy(clientB_Name1, Bnamesobj1.name);
						printf("%s\n",clientB_Name1);

						printf("The Central server received input=%s from the client using TCP over	port %s. \n", clientB_Name1, PORT2);

						clientB_rec = 1;

						if (clientA_rec == 1 && clientB_rec == 1){  //inputs from both clients are received

							Connect_to_ServerT();
						}
					}					
				}
				if(numUsernames[0] == '2'){  //if 2 usernames from B

					if (nbytes = recv(new_fd2, (char*)&Bnamesobj1, sizeof(Bnames), 0) <= 0){
						// if 0 socket closed other end, -1 is error
						if(nbytes == 0){
							cout << "clientA socket closed with ID: " << new_fd2;
						}else{
							perror("recv");
						}
					}
					else{

						//data received from client
						//store value in buf as clientB;
						strcpy(clientB_Name1, Bnamesobj1.name);
						// strcpy(clientB_Name2, Bnamesobj.name);
						printf("The Central server received input = %s from the client using TCP over port %s. \n", clientB_Name1, PORT2);
						
						if (nbytes = recv(new_fd2, (char*)&Bnamesobj2, sizeof(Bnames), 0) <= 0){ //get the second username
							// if 0 socket closed other end, -1 is error
							if(nbytes == 0){
								cout << "clientA socket closed with ID: " << new_fd2;
							}else{
								perror("recv");
							}
						}
						else{

							strcpy(clientB_Name2, Bnamesobj2.name);

							printf("The Central server received input = %s from the client using TCP over port %s. \n", clientB_Name2, PORT2);

							clientB_rec = 1;	
							
							if (clientA_rec == 1 && clientB_rec == 1){ //inputs from both clients are received

								Connect_to_ServerT();
							}
						}

					}					
				}

			}	
					// cout<<clientB_rec<<"\tClient B msg received\n";	
			clientB_done = 1;

		}

		if(clientA_done && clientB_done){  //processing of the given names are done, send the results

			Send_Results_to_ClientA();
			Send_Results_to_ClientB();
			close(sockUDP_binded);
		}
	}
	return 0;
}

