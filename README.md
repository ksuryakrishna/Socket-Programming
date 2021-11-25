# Socket-Programming

a. Surya Krishna Kasiviswanathan

b. STUDENT ID: 9083261716

c. BONUS OPTIONAL PART is COMPLETED

	This project is based on a simple social matching service where 2 clients send different usernames to the central server to find out their matching gap and the matching path. The backend servers help in identifying the path and the matching gap between the input usernames.

d. Code files in this project:
	
	1. clientA.cpp
		- Sends 1 username to the central and waits for the results
		- Prints the path and matching gap(if available), else says not compatabile
	2. clientB.cpp
		- Sends 1/2 username(s) to the central and waits for the results
		- Prints the path(s) and matching gap(if available), else says not compatabile
	3. central.cpp
		- Receives the input from both clients
		- Sends it to backend servers, gets the results and sends it back to the clients
	4. serverT.cpp
		- Creates a map of indices and names after reading the edgelist
		- Receives the usernames from the central and returns a graph to be sent to server P
	5. serverS.cpp
		- Creates a map of scores and names
		- Receives requests from the central and sends the scores to the central
	6. serverP.cpp
		- Receives the graph and scores from central
		- Based on the data received, creates a map based on index as key and the names and scores as values
		- Uses dijkstra's algorithm to find the minimum spanning tree (MPT) for a given source node
		- finds the shortest path from the source node to the 1/2 given destination nodes
		-sends the results to central
	7. makefile
		- make all: compiles all the source cpp codes and creates object files
		- make <filename>: compiles a particular source cpp code
			eg: "make central" compiles the source code of the central server
		- make clean: deletes all the object files previously created

e. Format of messages:
	client to central:
	 - number of usernames to be sent as 1 byte char
	 - usernames sent as char array
	central to serverT:
	 - number of usernames to be sent as 1 byte char
	 - usernames sent as char array 
	serverT to central:
	 - sends the adj_matrix as 1D array and number of vertices in the graph as int
	 - sends indices of the usernames as int in a struct
	central to serverS:
	 - sends request as char array
	serverS to central:
	 - sends the score map as objects of a struct
	central to serverT:
	 - sends the received adj_matrix, index list, score map
	serverT to central:
	 - sends the results
	 	- matching gap as float
	 	- for the path:
	 		- sends the number of vertices in the path as int
	 		- sends the names of the vertices in the path as char array in a struct obj
	central to client:
	 - sends int stating if there was any path found
	 - sends the path names as char array to both clients
	 		- to clientA in actual order 
	 		- to client B in reverse order

g. Idiosyncrasy:
	- The project supports upto 255 unique usernames in the edgelist/scorelist (But works for any number of edges, ie, edge count is not an issue)

h. Some snippets were referred from 'Beej's guide for Socket Programming' and modified to establish the TCP and UDP socket communication between the clients and the servers in this project. Also, I referred https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/?ref=lbp for finding the minimum spanning tree for a given source node.



