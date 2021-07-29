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
	~graph();

	graph(const graph& toCopy); //non ho capito pervhè toglienmdolo non compila

	
	void readFileDIMACSCSR(string fileName);
	/*
	 * Sequential version of Jones-Plassman algorithm
	 */
	void JonesPlassmanColoringSequential();
	/*
	 * Jones-Plassman implementation where threads work on a specific portion of the graph
	 * and they are synchronized after each iteration. No overlaps between different iterations.
	 * Synchronization is done by means of condition variables (a thread is launched as soon as
	 * one of the previous threads finishes)
	 */
	void JonesPlassmanColoringParallelBarriers();
	/*
	 * Standard Jones-Plassman algorithm implementation: no overlap between different iterations
	 * can occur. Threads are synchronized after each iteration.
	 * A threadpool is created and jobs are scheduled by the main function and executed by threads
	 */
	void JonesPlassmanColoringParallelStandard();
	/*
	 * Implementation of the Jones Plassman algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration. The same job finds the nodes to color, it
	 * places them in a local queue and it colors them after all nodes have been found
	 */
	void JonesPlassmanColoringParallelFindAndColor(const unsigned int maxThreads);
	/*
	 * LDF implementation where different iterations may overlap and jobs are
	 * scheduled by the threads after executing them
	 */
	int checkColoringCSR();
	void printColoring();
	void cancelColors();
	void infiniteLoopThreadRescheduleJob();
	/*
	 * Standard implementation of the LDF algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration
	 */
	void LargestDegreeFirstStandard();
	/*
	 * Implementation of the LDF algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration. The same job finds the nodes to color, it
	 * places them in a local queue and it colors them after all nodes have been found
	 */
	void LargestDegreeFirstFindAndColor();
	/*
	 * Sequential implementation of the Smallest Degree Last algorithm
	 */
	void SmallestDegreeLastSequential();
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
	void CalculateWeightsSDLParallel();
	void weighNodes(int from, int to);
	void findNodesToWeigh(int from, int to);
	bool weightConflict(int n);
	int isLocalMaximum(int n);
	int getMinColorCSR(int n, int min_color);
	bool colorConflictCSR(int n);
	void findNodesToColor(int from, int to);
	void findNodesToColorSingleThread(int from, int to);
	void ColorNodes(int from, int to);
	void ColorNodesSingleThread(int from, int to);
	void assignDegreeWeights();
	void CalculateWeightsSDL();
	void findAndColorNodes(int from, int to);
	int isLocalMaximummod(int n);

private:
	mutex _mtx, _qmtx, _mtx_colored, mutex_node_to_color, _mtx_weighted;
	condition_variable _cv, _cv_colored;
	int _n_thread, _k, _i, _N_THREADS;
	int _colored_nodes, _weighted_nodes;
	queue<function<void()>> _q;
	bool _terminate_pool;
	shared_mutex _mtx_weights;
	vector<int> _colors, _weights, _tmp_degree, _new_colors, _new_weights;
	int _n_nodes;
	bool _all_nodes_colored, _all_nodes_weighted, _increase_k;

	/* Variation of CSR: one single array containing adjacencies for each node */
	vector<vector<int>> _edgesCSR;
};

