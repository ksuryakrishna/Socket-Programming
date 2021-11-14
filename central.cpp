/*
** server.c -- a stream socket server demo
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

#define MAXBUFLEN 512

using namespace std;

	int sockfd1, sockfd2, sockfdS, sockfdT, sockfdP, sockUDP_binded, new_fd1, new_fd2;  // listen on sockfd1 and sockfd2, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size, addr_len;
	struct sigaction sa;

	char s[INET6_ADDRSTRLEN];
	char clientA_Name[512], clientB_Name[512];
	int yes=1;			// for setsockopt() SO_REUSEADDR, below
	int i, j, rv, res;

	//for select inclusion
	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    char buf[512];    // buffer for client data
    int nbytes;
	int pathfound = 0;  //-1 - path not found, 1- path found
    int clientA_rec = 0, clientB_rec = 0;
    //end - select inclusion declaration
    // struct sample{
    // 	int a;
    // 	int b;
    // 	char names[50];
    // 	int arr[40];
    // }obj;
    int length_1D = 0; //variable that contains the 1D length of adj matrix

    struct numV{	
    	int numstruct;
    }numobj;

	struct convert_map_to_struct{
		int indexvalue;
		char names[512];
	}obj[400];

	struct adj_matrix{
		int adj_m[160000];
	}adj;

	struct score_map{
		int score_value;
		char names[512];
	}obj_score[400];

	struct index_matrix{
		int indexA;
		int indexB;
		int indexC;
	}index_m;

	struct numVP{
		int numVinP;
		float match_gap;
	}NVP;

	struct vert_in_path{
		char names[512];
	}VIP[400];

    int numVertices = 0; //get this from T through numV struct

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
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

void bind_listen_clientB_parent(){

	//For second listener to bind on port 2 for client B
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

void reap_all_dead_process(){

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
}

void Receive_Path_MG_from_ServerP(){

	printf("listener: waiting to recvfrom serverP...\n");

	//char int_buffer[25];	
	//receive the no. of vertices in this path

	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &NVP, sizeof(NVP)/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	if(NVP.numVinP != -1){
		pathfound = 1;
		for(auto x = 0; x < NVP.numVinP; x++){
			addr_len = sizeof their_addr;
			if ((nbytes = recvfrom(sockUDP_binded, (char*) &VIP[x], sizeof(vert_in_path)/*MAXBUFLEN-1*/, 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
		}
		cout<<"going to display received path \n";
		//sample display 
		for(auto x = 0; x < NVP.numVinP; x++){
			cout<<x<<": \t" <<VIP[x].names<<endl;
		}		
	}
	else{
		pathfound = -1;
		cout<<"Path not found by serverT";
	}

	printf("The Central server received the results from backend server P.\n");

}

void Connect_to_ServerP(){

	sockfdP = 0; //used for UDP connection between central and T

	cout << "Entered connect to serverP\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
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
	//char test[] = "ServerT send rec test";
	// char score_req[] = "Request for score";

//send numVertices, score map, index map, 1d array to serverP
	// if ((nbytes = sendto(sockfdP, score_req, strlen(score_req), 0,
	// 		 p->ai_addr, p->ai_addrlen)) == -1){
	// 	perror("talker: sendto");
	// 	exit(1);
	// }
	// printf("talker: sent %d bytes to server T\n", nbytes);

		{
		//send numvertices
			if ((nbytes = sendto(sockfdP, (char*) &numobj, sizeof(numobj), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			printf("talker: sent %d bytes to P\n", nbytes);
			
		//send map as struct objs
			cout<<"Going to send index map";
			for (auto x = 0; x < numVertices; x++){
				if ((nbytes = sendto(sockfdP, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
			}
			printf("talker: sent %d bytes to P\n", nbytes);
				
		//send adjacency matrix as 1d
			cout<<"Going to send matrix\n";
			if ((nbytes = sendto(sockfdP, (char*) &adj, 65000, 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

		//send score map
			cout<<" Going to send score_map\n";
		//send map as struct objs
			for (auto x = 0; x < numVertices; x++){
				if ((nbytes = sendto(sockfdP, (char*) &obj_score[x], sizeof(score_map), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
				printf("talker: sent %d bytes to P\n", nbytes);
			}

			cout<<"going to send index list\n";

			if ((nbytes = sendto(sockfdP, (char*) &index_m, sizeof(index_matrix), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
			printf("talker: sent %d bytes to P\n", nbytes);			
		}

		// //sample receive from T
		// if ((nbytes = recvfrom(sockfd1, buf, MAXBUFLEN-1 , 0,
		// 	(struct sockaddr *)&their_addr, &sin_size)) == -1) {
		// 	perror("recvfrom T error");
		// 	exit(1);
		// }

		// printf("Received %s, %d from %s \n", buf, nbytes,
		// 	inet_ntop(their_addr.ss_family,
		// 		get_in_addr((struct sockaddr *)&their_addr),
		// 		s, sizeof s) );

	printf("The Central server sent a processing request to Backend-Server P.\n");
	
	freeaddrinfo(servinfo);
	close(sockfdP);
	/*Sending to Server S is done. Now wait for reply of
	graphs and list of nodes*/

	Receive_Path_MG_from_ServerP();
}

void Receive_score_from_ServerS(){
	// //Add listener code here and add recv two times(depends on how i receive)
	// sockfd2 = 0;

	// memset(&hints, 0, sizeof hints);
	// hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	// hints.ai_socktype = SOCK_DGRAM;
	// hints.ai_flags = AI_PASSIVE; // use my IP

	// if ((rv = getaddrinfo("127.0.0.1", MY_UDP_PORT, &hints, &servinfo)) != 0) {
	// 	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	// 	return;
	// }

	// // loop through all the results and bind to the first we can
	// for(p = servinfo; p != NULL; p = p->ai_next) {
	// 	if ((sockfd2 = socket(p->ai_family, p->ai_socktype,
	// 			p->ai_protocol)) == -1) {
	// 		perror("listener: socket");
	// 		continue;
	// 	}

	// 	if (bind(sockfd2, p->ai_addr, p->ai_addrlen) == -1) {
	// 		close(sockfd2);
	// 		perror("listener: bind");
	// 		continue;
	// 	}

	// 	break;
	// }

	// if (p == NULL) {
	// 	fprintf(stderr, "listener: failed to bind socket\n");
	// 	return;
	// }

	// freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom serverS...\n");

	//char int_buffer[25];	
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((nbytes = recvfrom(sockUDP_binded, (char*) &obj_score[x], sizeof(score_map)/*MAXBUFLEN-1*/, 0,
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

	printf("The Central server received information from Backend-Server S using UDP over port 24716. \n");
	// for(auto x = 0; x < )
	//buf[nbytes] = '\0';
	// cout<<"obj.a="<<obj.a<<endl;
	// cout<<"obj.b="<<obj.b<<endl;
	// printf("obj.names=%s\n",obj.names);

		// for(auto p = 0;p<10;p++){
		// 	cout << p << " : ";
		// 	for(auto q = 0; q<3;q++)
		// 		 cout << obj.arr[p][q] << " ";
		// 	cout << "\n";	
		// }
		// for(auto p = 0;p<40;p++){
		// 	cout << p << " : ";
		// 		 cout << obj.arr[p] << " ";
		// 	cout << "\n";	
		// }
		
	//printf("listener: packet contains \"%s \"\n", int_buffer);
	
	Connect_to_ServerP();
	// close(sockUDP_binded);   //dont close serverP needs it


}

void Connect_to_ServerS(){

	sockfdS = 0; //used for UDP connection between central and S

	cout << "Entered connect to serverS\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
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
	//char test[] = "ServerT send rec test";
	char score_req[] = "Request for score";

	if ((nbytes = sendto(sockfdS, score_req, strlen(score_req), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	printf("talker: sent %d bytes to server T\n", nbytes);


		// //sample receive from T
		// if ((nbytes = recvfrom(sockfd1, buf, MAXBUFLEN-1 , 0,
		// 	(struct sockaddr *)&their_addr, &sin_size)) == -1) {
		// 	perror("recvfrom T error");
		// 	exit(1);
		// }

		// printf("Received %s, %d from %s \n", buf, nbytes,
		// 	inet_ntop(their_addr.ss_family,
		// 		get_in_addr((struct sockaddr *)&their_addr),
		// 		s, sizeof s) );

	printf("The Central server sent a request to Backend-Server S.\n");

	freeaddrinfo(servinfo);
	close(sockfdS);
	/*Sending to Server S is done. Now wait for reply of
	graphs and list of nodes*/

	Receive_score_from_ServerS();
}

void Receive_graph_from_ServerT(){
	//Add listener code here and add recv two times(depends on how i receive)
	sockUDP_binded = 0;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
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

	printf("listener: waiting to recvfrom...\n");
	//char int_buffer[25];	
	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &numobj, sizeof(numobj)/*MAXBUFLEN-1*/, 0,
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
	printf("listener: packet is %d bytes long\n", nbytes);

	//get ready to receive the adjacency matrix and the map in the form struct objects
	printf("listener: waiting to recv map...\n");
	//char int_buffer[25];	
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((nbytes = recvfrom(sockUDP_binded, (char*) &obj[x], sizeof(convert_map_to_struct)/*MAXBUFLEN-1*/, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
	}
	cout<<"going to display received map \n";
	//sample display 
	for(auto x = 0; x<numVertices;x++){
		cout<<x<<": \t" <<obj[x].indexvalue<<"  "<<obj[x].names<<endl;
	}
	//now get the matrix
	printf("listener: waiting to recv adjacency matrix...\n");

	if ((nbytes = recvfrom(sockUDP_binded, (char*) &adj, 65000/*MAXBUFLEN-1*/, 0,
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

	addr_len = sizeof their_addr;
	if ((nbytes = recvfrom(sockUDP_binded, (char*) &index_m, sizeof(index_matrix)/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	cout<<"Received the indexs\n";

	cout<<"indexA: "<<index_m.indexA<<"\t indexB: "<<index_m.indexB<<endl;
	// for(auto x = 0; x < )
	//buf[nbytes] = '\0';
	// cout<<"obj.a="<<obj.a<<endl;
	// cout<<"obj.b="<<obj.b<<endl;
	// printf("obj.names=%s\n",obj.names);

		// for(auto p = 0;p<10;p++){
		// 	cout << p << " : ";
		// 	for(auto q = 0; q<3;q++)
		// 		 cout << obj.arr[p][q] << " ";
		// 	cout << "\n";	
		// }
		// for(auto p = 0;p<40;p++){
		// 	cout << p << " : ";
		// 		 cout << obj.arr[p] << " ";
		// 	cout << "\n";	
		// }
	printf("The Central server received information from Backend-Server T using UDP over port 24716.\n");

	//printf("listener: packet contains \"%s \"\n", int_buffer);
	Connect_to_ServerS();
	//close(sockfd2); dont close because central server listens to serverS and serverP with this


}


void Connect_to_ServerT(){

	sockfdT = 0; //used for UDP connection between central and T

	cout << "Entered connect to serverT\n";

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
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
	//char test[] = "ServerT send rec test";
	if ((nbytes = sendto(sockfdT, clientA_Name, strlen(clientA_Name), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}
	printf("talker: sent %d bytes to server T\n", nbytes);

	if ((nbytes = sendto(sockfdT, clientB_Name, strlen(clientB_Name), 0,
			 p->ai_addr, p->ai_addrlen)) == -1){
		perror("talker: sendto");
		exit(1);
	}	
	printf("talker: sent %d bytes to server T\n", nbytes);

		// //sample receive from T
		// if ((nbytes = recvfrom(sockfd1, buf, MAXBUFLEN-1 , 0,
		// 	(struct sockaddr *)&their_addr, &sin_size)) == -1) {
		// 	perror("recvfrom T error");
		// 	exit(1);
		// }

		// printf("Received %s, %d from %s \n", buf, nbytes,
		// 	inet_ntop(their_addr.ss_family,
		// 		get_in_addr((struct sockaddr *)&their_addr),
		// 		s, sizeof s) );

	printf("The Central server sent a request to Backend-Server T\n");

	freeaddrinfo(servinfo);
	close(sockfdT);
	/*Sending to Server T is done. Now bind socket and wait for reply of
	graphs and list of nodes*/

	Receive_graph_from_ServerT();
}

void Send_Results_to_ClientA(){

	if((nbytes = send(new_fd1, (char*) &NVP, sizeof(NVP), 0)) == -1){
		perror("Error sending NVP to clientA");
	}
	printf("talker: sent %d bytes to clientA\n", nbytes);

	if(pathfound == 1){
		for (auto x = 0; x < NVP.numVinP; x++){
			if ((nbytes = send(new_fd1, (char*) &VIP[x], sizeof(vert_in_path), 0)) == -1) {
				perror("central to clientA error");
				exit(1);
			}
			printf("talker: sent %d clientA\n", nbytes);
		}
	}
	else{
		//send clientB name
		if ((nbytes = send(new_fd1, clientB_Name, sizeof clientB_Name, 0)) == -1) {
			perror("central to clientA error");
			exit(1);
		}		

	}

	printf("The Central server sent the results to client A.\n");
	close(new_fd1);
}

void Send_Results_to_ClientB(){

	if((nbytes = send(new_fd2, (char*) &NVP, sizeof(NVP), 0)) == -1){
		perror("Error sending NVP to clientB");
	}
	printf("talker: sent %d bytes to clientB\n", nbytes);

	if(pathfound == 1){
		for (auto x = NVP.numVinP - 1; x >= 0; x--){
			if ((nbytes = send(new_fd2, (char*) &VIP[x], sizeof(vert_in_path), 0)) == -1) {
				perror("central to clientB error");
				exit(1);
			}
			printf("talker: sent %d clientB\n", nbytes);
		}
	}
	else{
		//send clientB name
		if ((nbytes = send(new_fd2, clientA_Name, sizeof clientA_Name, 0)) == -1) {
			perror("central to clientB error");
			exit(1);
		}		

	}	

	printf("The Central server sent the results to client B.\n");
	close(new_fd2);
	clientA_rec = 0; clientB_rec = 0;
}

int main(void)
{
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    bind_listen_clientA_parent();
    bind_listen_clientB_parent();

    reap_all_dead_process();

	//adding parent socks to master
	FD_SET(sockfd1, &master); 
	FD_SET(sockfd2, &master);

	//put the bigger socket to fdmax
	fdmax = (sockfd2 > sockfd1) ? sockfd2 : sockfd1;

	while(1){

		read_fds = master; 
		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
			cout << "select error\n";
			exit(4);
		}

		// run through the existing parent sockets to find if there is any client waiting for connection
		if(FD_ISSET(sockfd1, &read_fds)){
				//process clientA request
			sin_size = sizeof their_addr;
			new_fd1 = accept(sockfd1, (struct sockaddr *)&their_addr, &sin_size);

			if(new_fd1 == -1){
				cout << "Error in clientA accept";
				continue;
			}

			inet_ntop(their_addr.ss_family,
						get_in_addr((struct sockaddr *)&their_addr),
						s, sizeof s);
			printf("server: got connection from %s\n", s);	

			if (nbytes = recv(new_fd1, buf, sizeof buf, 0) <= 0){
				// if 0 socket closed other end, -1 is error
				if(nbytes == 0){
					cout << "clientA socket closed with ID: " << new_fd1;
				}else{
					perror("recv");
				}
				//close(new_fd1);
				//FD_CLR(sockfd1, &master);
			} else {
				//data received from client
				//store value in buf as clientA;
				strcpy(clientA_Name, buf);
				printf("%s\n",clientA_Name);

				printf("The Central server received input=%s from the client using TCP over port 25716.\n", clientA_Name);

				clientA_rec = 1;

				if (clientA_rec == 1 && clientB_rec == 1){
					//cout << "Can start UDP";
					// FD_CLR(sockfd1, &master);
					// FD_CLR(sockfd2, &master);
					// res = close(sockfd1);
					// //cout<<"res1:"<<res;
					// res = close(sockfd2); 
					//cout<<"\nres2:"<<res;

					Connect_to_ServerT();
				}				
			}
			cout<<clientA_rec<<"\tClient A msg received\n";	
		}	

		if(FD_ISSET(sockfd2, &read_fds)){
			//process clientB request
			sin_size = sizeof their_addr;
			new_fd2 = accept(sockfd2, (struct sockaddr *)&their_addr, &sin_size);

			if(new_fd2 == -1){
				cout << "Error in clientB accept";
				continue;
			}

			inet_ntop(their_addr.ss_family,
						get_in_addr((struct sockaddr *)&their_addr),
						s, sizeof s);
			printf("server: got connection from %s\n", s);

			if (nbytes = recv(new_fd2, buf, sizeof buf, 0) <= 0){
				// if 0 socket closed other end, -1 is error
				if(nbytes == 0){
					cout << "clientA socket closed with ID: " << new_fd2;
				}else{
					perror("recv");
				}
				//close(new_fd2);
				//FD_CLR(sockfd2, &master);
			} else {
				//data received from client
				//store value in buf as clientB;
				strcpy(clientB_Name, buf);
				printf("%s\n",clientB_Name);

				printf("The Central server received input=%s from the client using TCP over	port 25716. \n", clientA_Name);

				clientB_rec = 1;

				if (clientA_rec == 1 && clientB_rec == 1){
					//cout << "Can start UDP";
					// FD_CLR(sockfd1, &master);
					// FD_CLR(sockfd2, &master);
					//res = close(sockfd1);
					//cout<<"res1:"<<res;
					//res = close(sockfd2); 
					//cout<<"\nres2:"<<res;

					Connect_to_ServerT();
				}
			}	
			cout<<clientB_rec<<"\tClient B msg received\n";	

			Send_Results_to_ClientA();
			Send_Results_to_ClientB();
			close(sockUDP_binded);
		}

	}
	return 0;
}

