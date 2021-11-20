/*
** listener.c -- a datagram sockets "server" demo
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
#include <sstream>

using namespace std; 

#define MYPORT "21716"	// the port users will be connecting to

#define CENTRAL_PORT "24716" //the port T uses to connect to central

#define MAXBUFLEN 512

	int sockfd_binded, sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

    // struct sample{
    // 	int a;
    // 	int b;
    // 	char names[50];
    // 	int arr[10][3];
    // }obj;

    struct numV {	
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

//declaration for map generation
	string S;
	string file_name="edgelist.txt";
    map<string,int> m; 
    
    fstream fs(file_name);   
    int cnt = 0, Vertno = 0;

    char clientA_Name[512], clientB_Name1[512], clientB_Name2[512];

    char numUsernames[] = "1"; 

class Graph {
   private:
  // int** adjMatrix;
  int numVertices;

   public:
  int** adjMatrix;
  // Initialize the matrix to zero
  Graph(int numVertices) {
    this->numVertices = numVertices;
    adjMatrix = new int*[numVertices];
    for (int i = 0; i < numVertices; i++) {
      adjMatrix[i] = new int[numVertices];
      for (int j = 0; j < numVertices; j++)
        adjMatrix[i][j] = 0;
    }
  }

  void toString() {
    for (int i = 0; i < numVertices; i++) {
      cout << i << " : ";
      for (int j = 0; j < numVertices; j++)
        cout << adjMatrix[i][j] << " ";
      cout << "\n";
    }
  }
  // void return_matrix(int** matrix_to_send){
  // 	for (int i = 0; i <numVertices; i++){
  // 		for(int j = 0; j <numVertices; j++)
  // 			matrix_to_send[i][j] = adjMatrix[i][j];
  // 	}
  // }
  // Add edges
  void addEdge(int i, int j) {
    adjMatrix[i][j] = 1;
    adjMatrix[j][i] = 1;
  }
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void Connect_to_Central_to_send_graph(){
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
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
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

	//send numvertices
		if ((numbytes = sendto(sockfd, (char*) &numobj, sizeof(numobj), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		printf("talker: sent %d bytes to central\n", numbytes);
		
	//send map as struct objs
		for (auto x = 0; x < cnt; x++){
			if ((numbytes = sendto(sockfd, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}
		}
		printf("talker: sent %d bytes to central\n", numbytes);
			
	//send adjacency matrix
		cout<<"Going to send matrix\n";
		if ((numbytes = sendto(sockfd, (char*) &adj, 65000, 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
	//send index of the given clients
		cout<<"going to send index_M\n";
		if ((numbytes = sendto(sockfd, (char*) &index_m, sizeof(index_matrix), 0,
				 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		printf("talker: sent %d bytes to central\n", numbytes);
		freeaddrinfo(servinfo);

		printf("talker: sent %d bytes to central\n", numbytes);
		close(sockfd);

}

void generate_map(){

    while(fs>>S){
	    	//cout<<s<<endl;
	    if(!m.count(S)){
	    	m.insert(make_pair(S,cnt));
	    	cnt++;
	    }
    }
    //sample output
    map<string,int>::iterator i;
    for(i=m.begin();i!=m.end();i++) {
    	cout<<i->first<<"\t"<<i->second<<endl;
    }

    Graph g(cnt);
   // g.toString();
    fs.close();

    //reopen the file again to read the names again to add edges in the graph
	ifstream ifs(file_name); 
	int c = 0;
	// char[] temp_name_read = "";
	
	string f;
	int a = 0, b = 0;  //a=first index, b= second index
	while(ifs>>S){
		c++;
		if(c==1){
			f = S;
		}
		if(c==2){
			//edge exists between r and s
			//So call add edge here
			auto it = m.find(f);
			a = it->second;
			it = m.find(S);
			b = it->second;
			 //cout<<a<<" "<<b<<endl;
			g.addEdge(a,b);
			c = 0;
		}
	}
	ifs.close();
	g.toString();

	//add the map values on to a struct obj - total numVertices
	for(i=m.begin(); i!=m.end(), Vertno < cnt; Vertno++, i++){
		obj[Vertno].indexvalue = i->second;
		strcpy(obj[Vertno].names, i->first.c_str());
	}
	//insert numVertices(cnt) into numstruct
	numobj.numstruct = cnt;
	cout << "numobj.numstruct (cnt) = "<<numobj.numstruct<<endl;
	//sample display
	for(Vertno = 0; Vertno < cnt; Vertno++){
		cout<<obj[Vertno].indexvalue<<"\t";
		printf("%s\n",obj[Vertno].names);
	}
	int index = 0;
	for(auto x = 0; x < cnt; x++){
		for(auto y = 0; y < cnt; y++){
			adj.adj_m[index] = g.adjMatrix[x][y];
			index+=1;
		}
	}
	//sample display
	for(auto x = 0; x < index; x++){
		cout<<adj.adj_m[x]<<" ";
	}
	cout<<endl;	
}
int main(void)
{
	generate_map();

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

	printf("The ServerT is up and running using UDP on port 21716.\n");

	while(1){

		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd_binded, buf, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		numUsernames[0] = buf[0];

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("numUsernames: \"%s\"\n", numUsernames);

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
		printf("listener: packet contains clientA_Name: \"%s\"\n", buf);
		
		strcpy(clientA_Name, buf);

	//look up index for clientA and clientB and send to central
		stringstream ss1,ss2,ss3;
		string temp;
		ss1 << clientA_Name;
	    ss1 >> temp; 
		// cout << "temp: "<<temp<<endl;
		auto it = m.find(temp);
		if(m.count(temp)){
		    it = m.find(temp);
		    index_m.indexA = it->second;
		}
		else
			index_m.indexA = -2;

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
		printf("listener: packet contains clientB_Name1: \"%s\"\n", buf);	

		strcpy(clientB_Name1, buf);

		ss2 << clientB_Name1;
	    ss2 >> temp; 
		// cout << "temp: "<<temp<<endl;
		if(m.count(temp)){
			it = m.find(temp);
	    	index_m.indexB = it->second;   //send these to central	
		}
		else
			index_m.indexB = -2;


		index_m.indexC = -1;
		
		if(numUsernames[0] == '2'){

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
			printf("listener: packet contains clientB_Name2 \"%s\"\n", buf);	

			strcpy(clientB_Name2, buf);	
			// printf("test of clientB_Name2: %s\n", clientB_Name2);
			ss3 << clientB_Name2;
		    ss3 >> temp; 
		    // cout << "temp2: "<< temp <<endl;
		    if(m.count(temp)){
			    it = m.find(temp);
			    // cout<<"it->second:"<<it->second;
			    index_m.indexC = it->second;
			}
			else
				index_m.indexC = -2;

		}

		printf("The ServerT received a request from Central to get the topology.\n");
 
		//sample display
	    cout << "indexA: " << index_m.indexA << endl << "indexB: " << index_m.indexB <<endl<< "indexC: " 
	    			<< index_m.indexC << endl;
	// received the two usernames up until this point


		Connect_to_Central_to_send_graph();

		printf("The ServerT finished sending the topology to Central.\n");
	}
	
	return 0;
}
