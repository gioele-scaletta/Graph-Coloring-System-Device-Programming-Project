#pragma once
#include "node.h"
#include "edge.h"
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <mutex>

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
	bool checkColoring();
	void printColoring();
	void cancelColors();
	
private:
	void assignRandomWeights();
	bool weightConflict(int n);
	bool isLocalMaximum(int n);
	int getMinColor(int n);
	bool colorConflict(int n);
	void checkAndColorNode(int n, graph* V);

private:
	map<int, node> _nodes;
	vector<edge> _edges;
	mutex _mtx, _cv_m;
	condition_variable _cv;
};

