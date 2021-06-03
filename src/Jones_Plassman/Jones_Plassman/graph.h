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
	bool isColored(int n, vector<int> &_exit);
	int checkColoring();
	void printColoring();
	void cancelColors();
	void infiniteLoopThread();
	void infiniteLoopThreadVector(int maxThreads);
	
private:
	void assignRandomWeights();
	bool weightConflict(int n);
	int isLocalMaximum(node& n);
	int getMinColor(int n);
	bool colorConflict(int n);
	void checkAndColorNode(node& n);
	void checkAndColorListOfNodesQueueCounter(int from, int to);
	void checkAndColorListOfNodesVector(int from, int to, int* colored);
	void checkAndColorListOfNodesBarrier(int from, int to);
	


private:
	map<int, node> _nodes;
	vector<edge> _edges;
	mutex _mtx, _qmtx, _mtx_colored;
	condition_variable _cv, _cv_colored;
	int _n_thread;
	atomic<int> _colored_nodes;
	queue<function<void()>> _q;
	bool _terminate_pool;
	atomic<bool> _barrier;
	vector<thread> Poolvector;
	vector<int> _exit_;
};

