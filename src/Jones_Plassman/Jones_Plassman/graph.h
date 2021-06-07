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
	/*
	 * Sequential version of Jones-Plassman algorithm
	 */
	void JonesPlassmanColoringSequential();
	/*
	 * Jones-Plassman implementation with a threadpool and a counter to check stopping condition
	 * Jobs are scheduled by the threads after executing them (different iterations may overlap)
	 */
	void JonesPlassmanColoringParallelQueueCounter();
	/*
	 * Jones-Plassman implementation where stopping condition is checked on a vector of flags
	 * Jobs are scheduled by each thread after executing them, the main function also executes jobs
	 * A threadpool is created
	 */
	void JonesPlassmanColoringParallelQueueVector();
	/*
	 * Jones-Plassman implementation where threads work on a specific portion of the graph
	 * and they are synchronized after each iteration. No overlaps between different iterations.
	 * Synchronization is done by means of condition variables (a thread is launched as soon as
	 * one of the previous threads finishes)
	 */
	void JonesPlassmanColoringParallelBarriers();
	/*
	 * Jones-Plassman implementation where each thread works on a single node.
	 * Synchronization is done by means of condition variables
	 * No overlap between different iterations
	 */
	void JonesPlassmanColoringParallelOneNodeThread();
	/*
	 * Jones-Plassman implementation where stopping condition is checked by means of a vector of flags.
	 * Overlaps may occur between different iterations
	 */
	void JonesPlassmanColoringParallelVector();
	/*
	 * Standard Jones-Plassman algorithm implementation: no overlap between different iterations
	 * can occur. Threads are synchronized after each iteration.
	 * A threadpool is created and jobs are scheduled by the main function and executed by threads
	 */
	void JonesPlassmanColoringParallelStandard();
	/*
	 * LDF implementation where different iterations may overlap and jobs are
	 * scheduled by the threads after executing them
	 */
	void LargestDegreeFirst();
	bool isColored(int n, vector<int> &_exit);
	int checkColoring();
	void printColoring();
	void cancelColors();
	void infiniteLoopThread();
	void infiniteLoopThreadRescheduleJob();
	void infiniteLoopThreadVector(int maxThreads);
	/*
	 * Standard implementation of the LDF algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration
	 */

	void LargestDegreeFirstStandard();
	/*
	 * Standard implementation of the Smallest Degree Last algorithm where different iterations
	 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
	 * synchronized after each iteration. Weight assignment is parallelized.
	 */
	void SmallestDegreeLastParallelWeighing();
	/*
	 * Standard implementation of the Smallest Degree Last algorithm where different iterations
	 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
	 * synchronized after each iteration. Weight assignment is not parallelized.
	 */
	void SmallestDegreeLastStandard();
	/*
	 * Sequential greedy coloring algorithm
	 */
	void GreedySequential();

private:
	void assignRandomWeights();
	void CalculateWeightsSDFParallel();
	bool weighNodes(int from, int to);
	void findNodesToWeigh(int from, int to);
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
	mutex _mtx, _qmtx, _mtx_colored, mutex_node_to_color, _mtx_weighted;
	condition_variable _cv, _cv_colored;
	int _n_thread, _k, _i;
	atomic<int> _colored_nodes, _weighted_nodes;
	queue<function<void()>> _q;
	bool _terminate_pool;
	atomic<bool> _barrier;
	vector<int> _exit;
	vector<pair<node *,int >> _nodes_to_color;
	vector<node*> _to_weigh;

};

