/*
** serverP.cpp:
		- Receives the graph and scores from central
		- Based on the data received, creates a map based on index as key and the names and scores as values
		- Creates weighted graph based on the matching gap between each node using the formula
			Matching Gap between (S1, S2) = |S1-S2| / (S1 + S2)
		- Uses Dijkstra's algorithm to find the minimum spanning tree (MPT) for a given source node
		- finds the shortest path from the source node to the 1/2 given destination nodes
		- identifies the path by storing the parent node information of each node
		- recursively traverses the parent node to get the path
		- sends the results to central
		- Closes the socket
	- by Surya Krishna Kasiviswanathan, USC ID: 9083261716
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
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <map>
#include <fstream>
#include <cstring>
#include <vector>
#include <limits.h>

using namespace std; 

#define MYPORT "23716"	// the port users will be connecting to

#define CENTRAL_PORT "24716" //the port S uses to connect to central

#define MAXBUFLEN 512

#define MAX 1000000000

	int sockfd_binded, sockfd_to_central;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	string st1, st2;

	struct details{
		int score;
		char names[512];
	}det;

    map<int, struct details> m;  //map with index as key and name & score in struct as the value
    
	int Vertno = 0, pathfound1 = 0, pathfound2 = 0;  //-1 - path not found, 1- path found

    int length_1D = 0; //variable that contains the 1D length of adj matrix

    struct numV{	//struct that stores the number of vertices in the graph sent
    	int numstruct;
    }numobj;

	struct convert_map_to_struct{	//struct to store the map received
		int indexvalue;
		char names[512];
	}obj[400];

	struct adj_matrix{				//struct to store the adjacency matrix as 1D array
		uint8_t adj_m[160000];
	}adj;

	struct index_matrix{		//struct to store the indices of the given usernames
		int indexA;
		int indexB;
		int indexC;
	}index_m;

	struct score_map{			//struct to store the received score map
		int score_value;
		char names[512];
	}obj_score[400];

	struct numVP{				//struct to store the computed matching gap and
		int numVinP;			//the number of vertices in a given path
		float match_gap;
	}NVP1, NVP2;

	struct vert_in_path{		//struct to store the vertice names in the path
		char names[512];
	}VIP1[400], VIP2[400];

    int numVertices = 0; //get this from Central through numV struct
    vector<int> path_from_src1, path_from_src2;  //vector to store the path from destination to src if it exists
    vector<string> path_from_src_as_names1, path_from_src_as_names2;  //vector to store the names of the vertices in the path
    vector<float> dist_vector1, dist_vector2; //distane vector for each path

// // get sockaddr, IPv4 or IPv6:
// void *get_in_addr(struct sockaddr *sa)
// {
// 	if (sa->sa_family == AF_INET) {
// 		return &(((struct sockaddr_in*)sa)->sin_addr);
// 	}

// 	return &(((struct sockaddr_in6*)sa)->sin6_addr);
// }

//function to connect to the central to send matching gap and the path
//snippets from Beej's guide are used to connect to the UDP socket of the central server
void Connect_to_Central_to_send_results(){

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

	//send numvertices in path
		if ((numbytes = sendto(sockfd_to_central, (char*) &NVP1, sizeof(NVP1), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		// printf("talker: sent %d bytes to central\n", numbytes);
		
	//send vertices as structs
		if(pathfound1 != -1){
			for (auto x = 0; x < NVP1.numVinP; x++){
				if ((numbytes = sendto(sockfd_to_central, (char*) &VIP1[x], sizeof(vert_in_path), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
				// printf("talker: sent %d bytes to central\n", numbytes);
			}
		}

		if(index_m.indexC != -1 && index_m.indexC != -2){ //if name valid and there are 2 usernames from B

			//send numverticesin path
				if ((numbytes = sendto(sockfd_to_central, (char*) &NVP2, sizeof(NVP2), 0,
						 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
				// printf("talker: sent %d bytes to central\n", numbytes);
				
			//send vertices as structs
				if(pathfound2 != -1){
					for (auto x = 0; x < NVP2.numVinP; x++){
						if ((numbytes = sendto(sockfd_to_central, (char*) &VIP2[x], sizeof(vert_in_path), 0,
							 p->ai_addr, p->ai_addrlen)) == -1) {
							perror("talker: sendto");
							exit(1);
						}
						// printf("talker: sent %d bytes to central\n", numbytes);
					}
				}			
		}

		freeaddrinfo(servinfo);

		close(sockfd_to_central);


}

//function that receives the required details from central
//snippets from Beej's guide is used to recvfrom the central
void Recv_from_central(){

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_binded, (char*) &numobj, sizeof(numobj), 0,
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
		// printf("listener: packet is %d bytes long\n", numbytes);

	//get ready to receive the adjacency matrix and the map in the form of struct objects
	//get the map of nodes
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
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
	//now get the matrix
		// printf("listener: waiting to recv adjacency matrix...\n");
	//ge the adj matrix
	if ((numbytes = recvfrom(sockfd_binded, (char*) &adj, 65500, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	length_1D = numVertices*numVertices;  //remember to use this when you send to P
		// //sample display
		// cout<<"Received the matrix as 1D\n";
		// for(auto x = 0; x < length_1D; x++){
		// 	cout<<adj.adj_m[x]<<" ";
		// }
		// cout<<endl;	

	//receive score map from Central
	for(auto x = 0; x < numVertices; x++){
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, (char*) &obj_score[x], sizeof(score_map), 0,
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

	//index from central
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_binded, (char*) &index_m, sizeof(index_matrix)/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}	
	//when one or more of the input names is invalid, display error message and exit
	if(index_m.indexA == -2 || index_m.indexB == -2 || index_m.indexC == -2){
		perror("ENTERED INVALID NAME...EXITING PROGRAM, PLEASE RESTART the PROGRAM");
		exit(1);
	}
	
		// //sample display
		// cout<<"Received the indexs\n";

		// cout<<"indexA: "<<index_m.indexA<<"\t indexB: "<<index_m.indexB<<"\t indexC: "<<index_m.indexC<<endl;

}

//the 4 functions below are referred from geeksforgeeks.org and modified according to the requirement of this project

//function that recursively finds out the path from dest to the source
//by traversing through the parent_node array
void getpath(int parent_node[], int p, int dest){

	if(parent_node[p] == -1)
		return;

	getpath(parent_node, parent_node[p], dest);

	if(index_m.indexC != dest){

		path_from_src1.push_back(p);
	}
	else{

		path_from_src2.push_back(p);
	}
}

//function that returns whether a path was found or not
// -1 no path; 0 path found
int check_parent_n_gap(float dist[], int parent_node[], int dest){

	// int dest = index_m.indexB;

	if(dist[dest] >= MAX || dist[dest] < 0){
		return -1;   //no path found
	}
	else { //path found; store the index of the path in reverse order
		
		// for(auto x = 0; x < numVertices; x++){
		// 	path_from_dest[x] = parent_node[dest]; 
		// }
		getpath(parent_node, dest, dest);
		return 0;
	}
	
}

//function that returns index of the lowest distance vertex from the
//nodes that are not yet added to the Minimum Spanning Tree
int Min_Distance(float dist[], bool SP_tree[], int numV){
	
	// Initialize min value
	float min = INT_MAX, min_index;

	for (int y = 0; y < numV; y++)
		if (SP_tree[y] == false && dist[y] <= min){
			min = dist[y]; min_index = y;
			// cout<<y<<'\t'<<min<<'\t'<<min_index<<endl;
		}

	return min_index;
}

//function that computes the MST and also finds the path by calling some functions
//updates the distance vector every iteration
//finally return if there was a path found or not
int dijkstra(vector <vector<float> > &v, int src, int numV, int dest){
						
	float dist[numV]; //array to store distance from src to a particular node(index)
	bool SP_tree[numV]; //array to store whether node(index) is added to minimum spanning tree or not
	int parent_node[numV];  //array to store the parent node of each index
	memset(parent_node, 0, sizeof(parent_node));
	//initialize
	for (auto x = 0; x < numV; x++)
	{
		parent_node[src] = -1;
		dist[x] = MAX;
		SP_tree[x] = false;
	}

	dist[src] = 0;

	for (auto x = 0; x < numV ; x++)
	{
		int picked = Min_Distance(dist, SP_tree, numV); //select next node outside of SPT that has the min distance

		// cout << "picked: "<<picked<<endl;
		SP_tree[picked] = true;
		
		for (auto y = 0; y < numV; y++){

			if (!SP_tree[y] && v[picked][y] && (dist[picked] + v[picked][y] < dist[y])){
				parent_node[y] = picked;
				dist[y] = dist[picked] + v[picked][y];
			}
		}
			// //sample display
			// cout<<"picked: "<<picked<<",";
			// for(auto p = 0; p < numV; p++){
			// 	cout<<"p: "<<dist[p]<<'\t';
			// }
			// cout<<endl;
	}
		// //sample display
		// cout<<"Parent node:\n";
		// for(auto s = 0; s < numV; s++){
		// 	cout<< s<<" : "<<parent_node[s]<<'\t';
		// }

	//push the source index into the first index of path
	if(index_m.indexC != dest)
		path_from_src1.push_back(src);
	else
		path_from_src2.push_back(src);
	

	//send parent node and distance array to a function that checks the path and matching gap
	int r = check_parent_n_gap(dist, parent_node, dest);

//store the distance array into a vector to be used later in the code
	for(auto q = 0; q < numV; q++){
		if(index_m.indexC != dest)
			dist_vector1.push_back(dist[q]);
		else
			dist_vector2.push_back(dist[q]);
	}

	if(r == -1)
		return -5;  //-there is no path
	else
		return 0;  //path found
}


void generate_2d_map(){

    // while(fs>>S){
	   //  	//cout<<s<<endl;
	   //  if(!m.count(S)){
	   //  	m.insert(make_pair(S,cnt));
	   //  	cnt++;
	   //  }
    // }
    	// cout << "Creating 2d map...\n";

   //creates the 2d map with index as key and the associated names and scores as values of struct
	for(auto i = 0; i < numVertices; i++){
		if(!m.count(obj[i].indexvalue)){
			if(strcmp(obj[i].names, obj_score[i].names) == 0){  
				
				det.score = obj_score[i].score_value;
				strcpy(det.names,obj[i].names);

				m.insert(make_pair(obj[i].indexvalue, det));
			}

		}  
	}

		//sample display of the 2d map thats generated
	    // map<int, details>::iterator i;
	    // for(auto x = m.begin(); x != m.end(); x++) {
	    // 	cout<<x->first<<"\t"<<(x->second).score<<"\t"<<(x->second).names<<endl;
	    // }
}

//function that calculates matching gap
float matching_gap(int i, int j){
	
	int scoreI = 0, scoreJ = 0;

//find the score for the 2 indexes in the map
	auto it = m.find(i);
	scoreI = (it->second).score;

	it = m.find(j);
	scoreJ = (it->second).score;

	float match_gap = (scoreI - scoreJ);
	if(match_gap < 0)
		match_gap *= -1;
	match_gap /= (scoreI + scoreJ);

	return match_gap;
}

//the main function which wait for data from central to process
//snippets from Beej's guide are used to bind the UDP socket
int main(){  
     
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

	printf("The ServerP is up and running using UDP on port 23716.\n");

	while(1){

		Recv_from_central();

		printf("The ServerP received the topology and score information.\n");

		//processing area
		//obj[] has the index map //obj_score has the score map

		//adj has the 1D Array
		//create a vector of size numVertices x numVertices initialized with all zeroes
		vector<vector<float>> v( numVertices , vector<float> (numVertices, 0));
		
		for(auto i = 0, k = 0; i < numVertices; i++){

			for(auto j = 0; j < numVertices; j++){
				v[i][j] = adj.adj_m[k];
				k++;
			}
		}
		// //sample display of received matrix
		// 	for(auto i = 0; i < numVertices; i++){
		// 		cout<<i<<": ";
		// 		for(auto j = 0; j < numVertices; j++)
		// 			cout<<v[i][j]<<" ";
		// 		cout<<endl;
		// 	}
		m.clear();  //clear the map before creating a new one

		//create a map with index as the key1, name and score as value
		generate_2d_map();


		//generate weighted graph
		for(auto i = 0; i < numVertices; i++){
			for(auto j = 0; j < numVertices; j++){
				if(i != j && v[i][j] != 0){
					v[i][j] = matching_gap(i, j);
				}
			}
		}


			// //sample display of weighted matrix
			// cout<<endl<<"weighted matrix\n";
			// for(auto i = 0; i < numVertices; i++){
			// 	cout<<i<<": ";
			// 	for(auto j = 0; j < numVertices; j++)
			// 		cout<<v[i][j]<<" ";
			// 	cout<<endl;
			// }

		int result1 = dijkstra(v, index_m.indexA, numVertices, index_m.indexB);

		// //sample display
		// 	if(result1 == 0){
		// 		cout<<"Path found for 1st pair: ";
		// 		// printf("condn: %d\n", path_from_src1.size() );
		// 		for(auto s = 0; s < path_from_src1.size(); s++){
		// 			cout<<path_from_src1[s]<<'\t';
		// 			// printf("%s\t", path_from_src1[s]);
		// 		}
		// 	}
		// 	else if (result1 == -5){
		// 		cout<<"Path not found for 1st pair\n";
		// 	}
		// 	printf("Reached 1\n");
		NVP1.numVinP = 0;

		if(result1 == 0){//path found
			pathfound1 = 1;
			//give the indexes to the map and get the names to send to central
			for(auto s = 0; s < path_from_src1.size(); s++){
				auto it = m.find(path_from_src1[s]);
				path_from_src_as_names1.push_back((it->second).names);	
			}
			// printf("Reached 1.1\n");
			// cout<<"\n PAth Names for first pair:";
				// for(auto s = 0; s < path_from_src_as_names1.size(); s++)	
				// 	cout<<path_from_src_as_names1[s]<<'\t';
			// printf("Reached 1.2\n");
			NVP1.numVinP = path_from_src_as_names1.size();
			NVP1.match_gap = dist_vector1[index_m.indexB];
			// printf("Reached 1.3\n");
		}
		else if (result1 == -5){ //path not found
			pathfound1 = -1;
			NVP1.numVinP = -1;
			// cout<<"Path not found\n";
			NVP1.match_gap = 0;
		}
			// printf("Reached 2\n");

		//load the names in this struct
		for(auto s = 0; s < path_from_src_as_names1.size(); s++){
			strcpy(VIP1[s].names, path_from_src_as_names1[s].c_str());
		}
		// printf("Reached 3\n");
		if(index_m.indexC != -1 && index_m.indexC != -2){
		
			int result2 = dijkstra(v, index_m.indexA, numVertices, index_m.indexC);

				// if(result2 == 0){
				// 	cout<<"Path found: ";
				// 	for(auto s = 0; s < path_from_src2.size(); s++){
				// 		cout<<path_from_src2[s]<<'\t';
				// 	}
				// }
				// else if (result2 == -5){
				// 	cout<<"Path not found\n";
				// }

			NVP2.numVinP = 0;

			if(result2 == 0){//path found
				pathfound2 = 1;
				//give the indexes to the map and get the names to send to central
				for(auto s = 0; s < path_from_src2.size(); s++){
					auto it = m.find(path_from_src2[s]);
					path_from_src_as_names2.push_back((it->second).names);	
				}
					// cout<<"\n PAth Names:";
					// for(auto s = 0; s < path_from_src_as_names2.size(); s++)	
					// 	cout<<path_from_src_as_names2[s]<<'\t';

				NVP2.numVinP = path_from_src_as_names2.size();
				NVP2.match_gap = dist_vector2[index_m.indexC];
			}
			else if (result2 == -5){ //path not found
				pathfound2 = -1;
				NVP2.numVinP = -1;
				// cout<<"Path not found\n";
				NVP2.match_gap = 0;
			}

			//load the names in this struct
			for(auto s = 0; s < path_from_src_as_names2.size(); s++){
				strcpy(VIP2[s].names, path_from_src_as_names2[s].c_str());
			}
		}

		Connect_to_Central_to_send_results();

		printf("The ServerP finished sending the results to the Central.\n");

//clear all the vectors
		path_from_src_as_names1.clear();
		path_from_src1.clear();
		dist_vector1.clear();
		path_from_src_as_names2.clear();
		path_from_src2.clear();
		dist_vector2.clear();
	}	

    return 1;
	
}

