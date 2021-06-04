#pragma once
#include "node.h"
#include "edge.h"
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <shared_mutex>
#include <atomic>
#include <queue>


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
	void JonesPlassmanColoringParallelQueueCounter();
	void JonesPlassmanColoringParallelQueueVector();
	void JonesPlassmanColoringParallelBarriers();
	void JonesPlassmanColoringParallelOneNodeThread();
	void JonesPlassmanColoringParallelVector();
	void JonesPlassmanColoringParallel();
	void LargestDegreeFirst();
	bool isColored(int n, vector<int> &_exit);
	int checkColoring();
	void printColoring();
	void cancelColors();
	void infiniteLoopThread();
	void infiniteLoopThread_();
	void infiniteLoopThreadVector(int maxThreads);
	void LargestDegreeFirstStandard();
	void SmallDegreeFirstStandard();


	
private:
	void assignRandomWeights();
	bool weightConflict(int n);
	int isLocalMaximum(node& n);
	int getMinColor(int n);
	bool colorConflict(int n);
	void checkAndColorNode(node& n);
	void checkAndColorListOfNodesQueueCounter(int from, int to);
	void checkAndColorListOfNodesVectorSum(int from, int to, int* colored);
	bool isColoredSum(int n, vector<int> &_exit);
	void checkAndColorListOfNodesVector(int from, int to, int* colored);
	void checkAndColorListOfNodesBarrier(int from, int to);
	void findNodesToColor(int from, int to);
	void ColorNodes(int from,int  to);
	void checkAndColorListOfNodesLargestDegreeFirst(int from, int to);
	void assignDegreeWeights();
	void ColorNodesLDF(int from, int to);
	void CalculateWeightsSDF();

	


private:
	map<int, node> _nodes;
	vector<edge> _edges;
	mutex _mtx, _qmtx, _mtx_colored, mutex_node_to_color;
	condition_variable _cv, _cv_colored;
	int _n_thread;
	atomic<int> _colored_nodes;
	queue<function<void()>> _q;
	bool _terminate_pool;
	atomic<bool> _barrier;
	vector<thread> Poolvector;
	vector<int> _exit_;
	vector<pair<node *,int >> NodesToColor;


};

