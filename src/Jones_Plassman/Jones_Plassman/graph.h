#pragma once
#include "node.h"
#include "edge.h"
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <shared_mutex>
#include <atomic>


using namespace std;

class graph
{
public:
	graph();
	graph(const graph& toCopy);
	~graph();

	void addEdge(int n1, int n2);
	void removeNode(int n);
	int getNodesNumber();
	int getEdgesNumber();
	void readFromFile(string fileName);
	void readFileDIMACS(string fileName);
	void JonesPlassmanColoring();
	void JonesPlassmanColoringParallel();
	int checkColoring();
	void printColoring();
	void cancelColors();
	
private:
	void assignRandomWeights();
	bool weightConflict(int n);
	int isLocalMaximum(node& n);
	int getMinColor(int n);
	bool colorConflict(int n);
	void checkAndColorNode(node& n);
	void checkAndColorListOfNodes(int from, int to);

private:
	map<int, node> _nodes;
	vector<edge> _edges;
	mutex _mtx;
	condition_variable _cv;
	atomic<int> _n_thread;
	atomic<bool> _exit;
};

