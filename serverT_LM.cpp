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

using namespace std; 

#define MYPORT "21716"	// the port users will be connecting to

#define CENTRAL_PORT "24716" //the port T uses to connect to central
#define MAXBUFLEN 100

	int sockfd;
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
		int keyvalue;
		char names[512];
	}obj[400];    
//declaration for map generation
	string S;
	string file_name="edgelist.txt";
    map<string,int> m; 
    
    fstream fs(file_name);   
    int cnt = 0, Vertno = 0;

class Graph {
   private:
  int** adjMatrix;
  int numVertices;

   public:
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
	// obj.a = 1;
	// obj.b = 2;
	// strcpy(obj.names, "Amma");
	// for(auto p = 0;p<10;p++)
	// 	for(auto q = 0; q<3;q++)
	// 		obj.arr[p][q] = p;

	if ((numbytes = sendto(sockfd, (char*) &numobj, sizeof(numobj), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	
	for (auto x = 0; x < cnt; x++){
		if ((numbytes = sendto(sockfd, (char*) &obj[x], sizeof(convert_map_to_struct), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
	}

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
		obj[Vertno].keyvalue = i->second;
		strcpy(obj[Vertno].names, i->first.c_str());
	}
	//insert numVertices(cnt) into numstruct
	numobj.numstruct = cnt;
	cout << "numobj.numstruct (cnt) = "<<numobj.numstruct<<endl;
	//sample display
	for(Vertno = 0; Vertno < cnt; Vertno++){
		cout<<obj[Vertno].keyvalue<<"\t";
		printf("%s\n",obj[Vertno].names);
	}


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
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
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

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
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

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
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

	
	//sample msg sent as reply to central server
	// if ((numbytes = sendto(sockfd, a, strlen(a), 0,
	// 		 p->ai_addr, p->ai_addrlen)) == -1){
	// 	perror("T sends to central error: sendto");
	// 	exit(1);
	// }	
	close(sockfd);


	Connect_to_Central_to_send_graph();

	return 0;
}
