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
int numVertices = 0;
int Vertno = 0;

struct score_map{
	int score_value;
	char names[512];
}obj_score[400];

int main(){

	string st1, st2;
	string file_name = "scores.txt";
    map<string,int> m; 
    
    fstream fs(file_name);   
     
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



    return 1;
	
}