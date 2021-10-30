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

#define PORT1 "3490"  // the port users will be connecting to

#define PORT2 "5490"

#define BACKLOG 10	 // how many pending connections queue will hold

using namespace std;

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

int main(void)
{
	int sockfd1, sockfd2, new_fd1, new_fd2;  // listen on sock_fd1 and sockfd2, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;

	char s[INET6_ADDRSTRLEN];
	char clientA_Name[200], clientB_Name[200];
	int yes=1;			// for setsockopt() SO_REUSEADDR, below
	int i, j, rv;

	//for select inclusion
	fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    char buf[512];    // buffer for client data
    int nbytes;

    //end - select inclusion declaration

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT1, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
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
	//For second listener to bind on port 2 for client B
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT2, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
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

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

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

		// run through the existing parent sockets to find if there is any client is waiting for connection
		for (i = 0; i <= fdmax; i++){
			if(FD_ISSET(i, &read_fds)){
				if(i == sockfd1){
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
							cout << "clientA socket closed with ID: "<<i;
						}else{
							perror("recv");
						}
						close(new_fd1);
						FD_CLR(new_fd1, &master);
					} else {
						//data received from client
						//store value in buf as clientA;
						strcpy(clientA_Name, buf);
						printf("%s\n",clientA_Name);
					}

				}
				else if(i == sockfd2){
					//process clientB request
					cout<<"Reached sockfd2";
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
					if (nbytes = recv(new_fd1, buf, sizeof buf, 0) <= 0){
						// if 0 socket closed other end, -1 is error
						if(nbytes == 0){
							cout << "clientA socket closed with ID: "<<i;
						}else{
							perror("recv");
						}
						close(new_fd1);
						FD_CLR(new_fd1, &master);
					} else {
						//data received from client
						//store value in buf as clientA;
						strcpy(clientB_Name, buf);
						printf("%s\n",clientB_Name);
					}					
				}
			}
		}
	}
	while(1) {  // main accept() loop
		// sin_size = sizeof their_addr;
		// new_fd = accept(sockfd1, (struct sockaddr *)&their_addr, &sin_size);
		// if (new_fd == -1) {
		// 	perror("accept");
		// 	continue;
		// }

		// inet_ntop(their_addr.ss_family,
		// 	get_in_addr((struct sockaddr *)&their_addr),
		// 	s, sizeof s);
		// printf("server: got connection from %s\n", s);

		// if (!fork()) { // this is the child process
		// 	close(sockfd1); // child doesn't need the listener
		// 	if (send(new_fd, "Hello, world1!", 13, 0) == -1)
		// 		perror("send");
		// 	close(new_fd);
		// 	exit(0);
		// }
		// close(new_fd);  // parent doesn't need this
	}

	return 0;
}

