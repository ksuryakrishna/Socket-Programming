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
#include <stdio.h>
#include <limits.h>
#include <float.h>

using namespace std;

int main(){

	int x = INT_MAX;

	if(x+1<INT_MAX){
		cout<<"good";
		cout<<x+1;
	}
	else{
		cout<<"overflow";
	}
	// cout << x+1;
	// cout<<INT_MAX<<endl<<FLT_MAX;
	cout<<"floatmax: "<<FLT_MAX;
	return 0;

}