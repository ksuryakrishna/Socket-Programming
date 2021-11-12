// C program for Dijkstra's single
// source shortest path algorithm.
// The program is for adjacency matrix
// representation of the graph.
#include <stdio.h>
#include <limits.h>
#include <iostream>
// Number of vertices
// in the graph
#define V 9
using namespace std;
// A utility function to find the
// vertex with minimum distance
// value, from the set of vertices
// not yet included in shortest
// path tree
int minDistance(float dist[],
				bool sptSet[])
{
	
	// Initialize min value
	int min = INT_MAX, min_index;
		for(auto x = 0; x < V; x++){
			cout<<"x: "<<dist[x]<<'\t';
		}
		cout<<endl;
	for (int v = 0; v < V; v++)
		if (sptSet[v] == false && dist[v] <= min)
			{min = dist[v]; min_index = v;}
	cout<<".................minindex:"<<min_index;
	return min_index;
}

// Function to print shortest
// path from source to j
// using parent array
void printPath(int parent[], int j)
{
	
	// Base Case : If j is source
	if (parent[j] == - 1)
		return;

	printPath(parent, parent[j]);

	printf("%d ", j);
}

// A utility function to print
// the constructed distance
// array
int printSolution(float dist[], int n,
					int parent[])
{
	int src = 0;
	printf("Vertex\t Distance\tPath");
	for (int i = 1; i < V; i++)
	{
		// printf("%d  ", dist[i]);
		printf("\n%d -> %d \t\t %f\t\t%d ",
					src, i, dist[i], src);
		printPath(parent, i);
	}
}

// Function that implements Dijkstra's
// single source shortest path
// algorithm for a graph represented
// using adjacency matrix representation
void dijkstra(int graph[V][V], int src)
{
	
	// The output array. dist[i]
	// will hold the shortest
	// distance from src to i
	float dist[V];

	// sptSet[i] will true if vertex
	// i is included / in shortest
	// path tree or shortest distance
	// from src to i is finalized
	bool sptSet[V];

	// Parent array to store
	// shortest path tree
	int parent[V];

	// Initialize all distances as
	// INFINITE and stpSet[] as false
	for (int i = 0; i < V; i++)
	{
		parent[0] = -1;
		dist[i] = 1000000000;
		sptSet[i] = false;
	}

	// Distance of source vertex
	// from itself is always 0
	dist[src] = 0;

	// Find shortest path
	// for all vertices
	for (int count = 0; count < V ; count++)
	{
		// Pick the minimum distance
		// vertex from the set of
		// vertices not yet processed.
		// u is always equal to src
		// in first iteration.
		int u = minDistance(dist, sptSet);
		cout<<"u: "<<u<<",\n";

		// Mark the picked vertex
		// as processed
		sptSet[u] = true;

		// Update dist value of the
		// adjacent vertices of the
		// picked vertex.
		for (int v = 0; v < V; v++)

			// Update dist[v] only if is
			// not in sptSet, there is
			// an edge from u to v, and
			// total weight of path from
			// src to v through u is smaller
			// than current value of
			// dist[v]
			if (!sptSet[v] && graph[u][v] &&
				(dist[u] + graph[u][v] < dist[v]))
			{
				parent[v] = u;
					dist[v] = dist[u] + graph[u][v];
			}
			
		for(auto x = 0; x < V; x++){
			cout<<"x: "<<dist[x]<<'\t';
		}
		cout<<endl;
	}

	// print the constructed
	// distance array
	printSolution(dist, V, parent);
}

// Driver Code
int main()
{
	// Let us create the example
	// graph discussed above
	// int graph[V][V] = {{0, 4, 0, 0, 0, 0, 0, 8, 0},
	// 				{4, 0, 8, 0, 0, 0, 0, 11, 0},
	// 					{0, 8, 0, 7, 0, 4, 0, 0, 2},
	// 					{0, 0, 7, 0, 9, 14, 0, 0, 0},
	// 					{0, 0, 0, 9, 0, 10, 0, 0, 0},
	// 					{0, 0, 4, 0, 10, 0, 2, 0, 0},
	// 					{0, 0, 0, 14, 0, 2, 0, 1, 6},
	// 					{8, 11, 0, 0, 0, 0, 1, 0, 7},
	// 					{0, 0, 2, 0, 0, 0, 6, 7, 0}
	// 				};

	// int graph[V][V] = {{0, 4, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 8, INT_MAX},
	// 			{4, 0, 8, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 11, INT_MAX},
	// 				{INT_MAX, 8, 0, 7, INT_MAX, 4, INT_MAX, INT_MAX, 2},
	// 				{INT_MAX, INT_MAX, 7, 0, 9, 14, INT_MAX, INT_MAX, INT_MAX},
	// 				{INT_MAX, INT_MAX, INT_MAX, 9, 0, 10, INT_MAX, INT_MAX, INT_MAX},
	// 				{INT_MAX, INT_MAX, 4, INT_MAX, 10, 0, 2, INT_MAX, INT_MAX},
	// 				{INT_MAX, INT_MAX, INT_MAX, 14, INT_MAX, 2, 0, 1, 6},
	// 				{8, 11, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 1, 0, 7},
	// 				{INT_MAX, INT_MAX, 2, INT_MAX, INT_MAX, INT_MAX, 6, 7, 0}
	// 			};

	// int graph[V][V] = {{0, 4, 1000, 1000, 1000, 1000, 1000, 8, 1000},
	// 			{4, 0, 8, 1000, 1000, 1000, 1000, 11, 1000},
	// 				{1000, 8, 0, 1000, 1000, 1000, 1000, 1000, 2},
	// 				{1000, 1000, 1000, 0, 9, 14, 1000, 1000, 1000},
	// 				{1000, 1000, 1000, 9, 0, 10, 1000, 1000, 1000},
	// 				{1000, 1000, 1000, 1000, 10, 0, 1000, 1000, 1000},
	// 				{1000, 1000, 1000, 14, 1000, 1000, 0, 1, 6},
	// 				{8, 11, 1000, 1000, 1000, 1000, 1, 0, 7},
	// 				{1000, 1000, 2, 1000, 1000, 1000, 6, 7, 0}
	// 			};

	int graph[V][V] = {{0, 4, 1000000000, 1000000000, 1000000000, 1000000000, 1000000000, 8, 1000000000},
					{4, 0, 8, 1000000000, 1000000000, 1000000000, 1000000000, 11, 1000000000},
					{1000000000, 8, 0, 1000000000, 1000000000, 1000000000, 1000000000, 1000000000, 2},
					{1000000000, 1000000000, 1000000000, 0, 9, 14, 1000000000, 1000000000, 1000000000},
					{1000000000, 1000000000, 1000000000, 9, 0, 10, 1000000000, 1000000000, 1000000000},
					{1000000000, 1000000000, 1000000000, 14, 10, 0, 1000000000, 1000000000, 1000000000},
					{1000000000, 1000000000, 1000000000, 1000000000, 1000000000, 1000000000, 0, 1, 6},
					{8, 11, 1000000000, 1000000000, 1000000000, 1000000000, 1, 0, 7},
					{1000000000, 1000000000, 2, 1000000000, 1000000000, 1000000000, 6, 7, 0}
				};

	dijkstra(graph, 2);
	return 0;
}
