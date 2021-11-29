# Socket-Programming

a. Surya Krishna Kasiviswanathan

b. STUDENT ID: 9083261716

	 ASSUMPTIONS:
	  - Every name present in the scores.txt file is present atleast once in the edgelist.txt.
	  - Similarly every name on the edgelist.txt should have one entry in the scores.txt.
	  - Servers are run before the clients
	  - When an invalid input username is given by any of the clients, an error message is thrown by both the clients and the central server. The programs get terminated.

	 TESTING DONE:
	  - The project theoritically supports upto 255 unique usernames and more than 1000 edges.
	  - The maximum that was actually tested was 120 usernames and 500 edges, and was found to work. It is expected to work for higher numbers too.
	  - While running the project in the VM with long edgelist.txt and scores.txt I realized that due to the slowness of the VM, some of the UDP packets were not received on time because the receiver was slow in completing memory store operations. So I have added sufficient delay for the servers to receive packets comfortably. So please wait atleast 5 seconds for the output to be visible on the client terminals.

	  STEPS TO RUN THE PROJECT:
	 - Add the 'edgelist.txt' and 'scores.txt' in the same location as the source code .cpp files
	 - Open a terminal and run 'make all' to generate the object/executable files
	 - Open 6 terminals in total and run each program in the following order: (eg: For servers run './serverT', For clients run './clientA <input>')
	 	1. serverC
	 	2. serverT
	 	3. serverS
	 	4. serverP
	 	5. clientA & clientB (Any order)

	 - Any client can send the request first, no particular order. Only requirement is all the servers need to be running before the clients start to run.
	 - Please wait for atleast 5 seconds for the output to be visible on the client terminals. 

c. BONUS OPTIONAL PART IS COMPLETED

	This project is based on a simple social matching service where 2 clients send different usernames to the central server to find out their matching gap and the matching path. The backend servers help in identifying the path and the matching gap between the input usernames.

d. Code files in this project:
	
		All the codes in the project use the loopback IP address -'127.0.0.1' which is hardcoded in each of them.

	1. clientA.cpp
		- Sends 1 username to the central and waits for the results
		- Prints the path and matching gap(if available), else says not compatible
		- Closes the TCP socket after receving the results
	2. clientB.cpp
		- Sends 1/2 username(s) to the central and waits for the results
		- Prints the path(s) and matching gap(if available), else says not compatible
		- Closes the TCP socket after receving the results
	3. central.cpp
		- Receives the input from both clients
		- Sends it to backend servers, gets the results and sends it back to the clients (to A in correct order and to B in reverse order)
		- This server always keeps 2 TCP parent sockets live to listen to incoming requests from the clients, but closes the child sockets after sending out the results
		- Keeps 1 UDP socket binded and open for any server to send its results and closes the UDP sockets that it uses to send to the servers
	4. serverT.cpp
		- Creates a map of indices and names after reading the edgelist
		- Receives the usernames from the central and returns graphs to be sent to server P
		- Closes the socket
	5. serverS.cpp
		- Creates a map of scores and names
		- Receives requests from the central and sends the scores to the central
		- Closes the socket
	6. serverP.cpp
		- Receives the graph and scores from central
		- Based on the data received, creates a map based on index as key and the names and scores as values
			- to look it up while sending the results to central as path names
		- Creates weighted graph based on the matching gap between each node using the formula
			Matching Gap between (S1, S2) = |S1 - S2| / (S1 + S2)
		- Uses Dijkstra's algorithm to find the minimum spanning tree (MPT) for a given source node
		- Finds the shortest path from the source node to the 1/2 given destination nodes
		- Identifies the path by storing the parent node information of each node
		- Recursively traverses the parent node to get the path
		- Sends the results to central
		- Closes the socket
	7. makefile
		- make all: compiles all the source cpp codes and creates object files
		- make clean: deletes all the object files previously created

e. Format of messages:
	
	Since the sockets send, receive and interpret everything as a stream of bytes, I have used struct objects. The struct objects help in taking the stream of bytes and appropriately put it in the correct containers so that they can be used by the receiving programs. Typecasting from struct to char* (a stream of bytes) is the only requirement.

	1. client to central:
		 - number of usernames to be sent as 1 byte char
		 - usernames sent as char array
	2. central to serverT:
		 - number of usernames to be sent as 1 byte char
		 - usernames sent as char array 
	3. serverT to central:
		 - sends the adj_matrix as 1D array and number of vertices in the graph as int
		 - sends indices of the usernames as int in a struct
	4. central to serverS:
	 	- sends the map received from T as request
	5. serverS to central:
	 	- sends the score map as objects of a struct
	6. central to serverT:
		 - sends the received adj_matrix, index list, score map
	7. serverT to central:
		 - sends the results
		 	- matching gap as float
		 	- for the path:
		 		- sends the number of vertices in the path as int
		 		- sends the names of the vertices in the path as char array in a struct obj
	8. central to client:
		 - sends int stating if there was any path found
		 - sends the path names as char array to both clients
		 		- to clientA in actual order 
		 		- to client B in reverse order

g. Idiosyncrasy (Maximum Limit):

	- The project supports upto 255 unique usernames in the edgelist/scorelist.
	- Every name present in the scores.txt file should be present atleast once in the edgelist.txt. That is, every name on the scores.txt should be a node in one of the graphs. Otherwise the project fails. 
	- Similarly every name on the edgelist.txt should have one entry in the scores.txt.

h. References:

	- Some snippets were referred from 'Beej's guide for Socket Programming' and modified to establish the TCP and UDP socket communication between the clients and the servers in this project. 
	- Also, I referred https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/?ref=lbp for finding the minimum spanning tree for a given source node. The following 4 functions in serverP.cpp are modifications from the one from 'geeksforgeeks': dijkstra(), check_parent_n_gap(), getpath(), Min_Distance(). 



