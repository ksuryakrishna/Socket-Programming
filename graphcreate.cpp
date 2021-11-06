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

struct convert_map_to_struct{
	int keyvalue;
	char names[512];
}obj[400];

int main(){

	string s;
	string file_name="edgelist.txt";
    map<string,int> m; 
    
    fstream fs(file_name);   
    int cnt = 0, Vertno = 0; 
    while(fs>>s){
	    	//cout<<s<<endl;
	    if(!m.count(s)){
	    	m.insert(make_pair(s,cnt));
	    	cnt++;
	    }
    }
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
	while(ifs>>s){
		c++;
		if(c==1){
			f = s;
		}
		if(c==2){
			//edge exists between r and s
			//So call add edge here
			auto it = m.find(f);
			a = it->second;
			it = m.find(s);
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
	//sample display
	for(Vertno = 0; Vertno < cnt; Vertno++){
		cout<<obj[Vertno].keyvalue<<"\t";
		printf("%s\n",obj[Vertno].names);
	}



    return 1;
}