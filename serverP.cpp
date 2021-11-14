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

#define MAXBUFLEN 100

#define MAX 1000

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

    // map<int, map<int, string> > m; 
    // map<int, map<int, string> >::iterator itr1;
    // map<int, string>::iterator itr2;

    map<int, struct details> m;
    
    // int numVertices = 0;
	int Vertno = 0, pathfound = 0;  //-1 - path not found, 1- path found

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

	struct index_matrix{
		int indexA;
		int indexB;
		int indexC;
	}index_m;

	struct score_map{
		int score_value;
		char names[512];
	}obj_score[400];

	struct numVP{
		int numVinP;
		float match_gap;
	}NVP;

	struct vert_in_path{
		char names[512];
	}VIP[400];

    int numVertices = 0; //get this from Central through numV struct
    vector<int> path_from_src;  //vector to store the path from destination to src if it exists
    vector<string> path_from_src_as_names;
    vector<float> dist_vector;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


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
		// obj.a = 1;
		// obj.b = 2;
		// strcpy(obj.names, "Amma");
		// for(auto p = 0;p<10;p++)
		// 	for(auto q = 0; q<3;q++)
		// 		obj.arr[p][q] = p;

	//send numverticesin path
		if ((numbytes = sendto(sockfd_to_central, (char*) &NVP, sizeof(NVP), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		printf("talker: sent %d bytes to central\n", numbytes);
		
	//send vertices as structs
		if(pathfound != -1){
			for (auto x = 0; x < NVP.numVinP; x++){
				if ((numbytes = sendto(sockfd_to_central, (char*) &VIP[x], sizeof(vert_in_path), 0,
					 p->ai_addr, p->ai_addrlen)) == -1) {
					perror("talker: sendto");
					exit(1);
				}
				printf("talker: sent %d bytes to central\n", numbytes);
			}
		}
		freeaddrinfo(servinfo);

		// printf("talker: sent %d bytes to central\n", numbytes);
		close(sockfd_to_central);


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
		cout<<x<<": \t" <<obj[x].indexvalue<<"  "<<obj[x].names<<endl;
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

	//index from central
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_binded, (char*) &index_m, sizeof(index_matrix)/*MAXBUFLEN-1*/, 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}	
	//sample display
	cout<<"Received the indexs\n";

	cout<<"indexA: "<<index_m.indexA<<"\t indexB: "<<index_m.indexB<<endl;

}


void getpath(int parent_node[], int p){

	if(parent_node[p] == -1)
		return;

	getpath(parent_node, parent_node[p]);

	path_from_src.push_back(p);

}

int check_parent_n_gap(float dist[], int parent_node[]){

	int dest = index_m.indexB;

	if(dist[dest] >= MAX || dist[dest] < 0){
		return -1;   //no path found
	}
	else { //path found; store the index of the path in reverse order
		
		// for(auto x = 0; x < numVertices; x++){
		// 	path_from_dest[x] = parent_node[dest]; 
		// }
		getpath(parent_node, dest);
		return 0;
	}
	
}

int Min_Distance(float dist[], bool SP_tree[], int numV){
	
	// Initialize min value
	int min = INT_MAX, min_index;

	for (int y = 0; y < numV; y++)
		if (SP_tree[y] == false &&	dist[y] <= min)
			min = dist[y], min_index = y;

	return min_index;
}

int dijkstra(vector <vector<float> > &v, int src, int numV){
						
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

		SP_tree[picked] = true;

		for (auto y = 0; y < numV; y++){

			if (!SP_tree[y] && v[picked][y] && (dist[picked] + v[picked][y] < dist[y])){
				parent_node[y] = picked;
				dist[y] = dist[picked] + v[picked][y];
			}
		}
		//sample display
		cout<<"picked: "<<picked<<",";
		for(auto p = 0; p < numV; p++){
			cout<<"p: "<<dist[p]<<'\t';
		}
		cout<<endl;
	}
	//sample display
	cout<<"Parent node:\n";
	for(auto s = 0; s < numV; s++){
		cout<< s<<" : "<<parent_node[s]<<'\t';
	}
	// print the constructed
	// distance array
	// printSolution(dist, numV, parent_node);
	path_from_src.push_back(src);
	//send parent node and distance array to a function that checks the path and matching gap
	int r = check_parent_n_gap(dist, parent_node);

	for(auto q = 0; q < numV; q++){
		dist_vector.push_back(dist[q]);
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
    cout << "Creating 2d map...\n";

   //creates the 2d map with index as key and the associated names and scores as values of struct
	for(auto i = 0; i < numVertices; i++){
		if(!m.count(obj[i].indexvalue)){
			if(strcmp(obj[i].names, obj_score[i].names) == 0){   //check this if it works
				
				det.score = obj_score[i].score_value;
				strcpy(det.names,obj[i].names);

				m.insert(make_pair(obj[i].indexvalue, det));
			}

		}  
	}

	//sample display of the 2d map thats generated
    map<int, details>::iterator i;
    for(auto x = m.begin(); x != m.end(); x++) {
    	cout<<x->first<<"\t"<<(x->second).score<<"\t"<<(x->second).names<<endl;
    }
}
//function that calculates matching gap
float matching_gap(int i, int j){
	
	int scoreI = 0, scoreJ = 0;

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

	while(1){

		printf("listener: waiting to recvfrom CENTRAL SERVER...\n");

		Recv_from_central();

		//processing area
		//obj[] has the index map //obj_score has the score map

		//adj has the 1D Array
		vector<vector<float>> v( numVertices , vector<float> (numVertices, 0));
		
		for(auto i = 0, k = 0; i < numVertices; i++){

			for(auto j = 0; j < numVertices; j++){
				v[i][j] = adj.adj_m[k];
				k++;
			}
		}
	//sample display of received matrix
		for(auto i = 0; i < numVertices; i++){
			cout<<i<<": ";
			for(auto j = 0; j < numVertices; j++)
				cout<<v[i][j]<<" ";
			cout<<endl;
		}

		//create a map with index as the key1, name and score as value
		generate_2d_map();


		//generate weighted graph
		for(auto i = 0; i < numVertices; i++){
			for(auto j = 0; j < numVertices; j++){
				if(i != j && v[i][j] != 0){
					v[i][j] = matching_gap(i, j);
				}
				// else if(i != j && v[i][j] == 0){
				// 	v[i][j] = MAX;
				// }
			}
		}


		//sample display of weighted matrix
		cout<<endl<<"weighted matrix\n";
		for(auto i = 0; i < numVertices; i++){
			cout<<i<<": ";
			for(auto j = 0; j < numVertices; j++)
				cout<<v[i][j]<<" ";
			cout<<endl;
		}

		int result = dijkstra(v, index_m.indexA, numVertices);


	//sample display
		if(result == 0){
			cout<<"Path found: ";
			for(auto s = 0; s < path_from_src.size(); s++){
				cout<<path_from_src[s]<<'\t';
			}
		}
		else if (result == -5){
			cout<<"Path not found";
		}

		NVP.numVinP = 0;

		if(result == 0){//path found
			pathfound = 1;
			//give the indexes to the map and get the names to send to central
			for(auto s = 0; s < path_from_src.size(); s++){
				auto it = m.find(path_from_src[s]);
				path_from_src_as_names.push_back((it->second).names);	
			}
			cout<<"\n PAth Names:";
			for(auto s = 0; s < path_from_src_as_names.size(); s++)	
				cout<<path_from_src_as_names[s]<<'\t';

			NVP.numVinP = path_from_src_as_names.size();
			NVP.match_gap = dist_vector[index_m.indexB];
		}
		else if (result == -5){ //path not found
			pathfound = -1;
			NVP.numVinP = -1;
			cout<<"Path not found";
		}

		//load the names in this struct
		for(auto s = 0; s < path_from_src_as_names.size(); s++){
			strcpy(VIP[s].names, path_from_src_as_names[s].c_str());
		}

		Connect_to_Central_to_send_results();

		path_from_src_as_names.clear();
		path_from_src.clear();
		dist_vector.clear();
	}	

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
	//close(sockfd_binded);  //finally remove this, dont close because server should be 
	//continously listening on the binded socket


	



    return 1;
	
}

