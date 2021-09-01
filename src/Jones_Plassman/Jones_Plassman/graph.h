#pragma once
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

	graph(const graph& toCopy); 
	
	/* Read graphs from file */
	void readFileDIMACS10(string fileName);
	void readFileDIMACS(string fileName);

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
	void JonesPlassmanColoringParallelNoThreadpool(unsigned int maxThreads, int coef);
	/*
	 * Standard Jones-Plassman algorithm implementation: no overlap between different iterations
	 * can occur. Threads are synchronized after each iteration.
	 * A threadpool is created and jobs are scheduled by the main function and executed by threads
	 */
	void JonesPlassmanColoringParallelStandard(unsigned int maxThreads, int coef);
	/*
	 * Implementation of the Jones Plassman algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration. The same job finds the nodes to color, it
	 * places them in a local queue and it colors them after all nodes have been found
	 */
	void JonesPlassmanColoringParallelFindAndColor(unsigned int maxThreads, int coef);

	/* Check if current coloring is correct */
	int checkColoring();
	/* Print current coloring */
	void printColoring();
	/* Cancel current coloring */
	void cancelColors();
	/* Thread function when using a threadpool (wait for jobs in the queue and check termination condition) */
	void waitAndExecuteJobs();
	/*
	 * Standard implementation of the LDF algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration
	 */
	void LargestDegreeFirstStandard(unsigned int maxThreads, int coef);
	/*
	 * Implementation of the LDF algorithm. No overlap between different iterations.
	 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
	 * which are synchronized after each iteration. The same job finds the nodes to color, it
	 * places them in a local queue and it colors them after all nodes have been found
	 */
	void LargestDegreeFirstFindAndColor(unsigned int maxThreads, int coef);
	/*
	 * Sequential implementation of the Smallest Degree Last algorithm
	 */
	void SmallestDegreeLastSequential();
	/*
	 * Standard implementation of the Smallest Degree Last algorithm where different iterations
	 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
	 * synchronized after each iteration. Weight assignment is not parallelized.
	 */
	void SmallestDegreeLastStandard(unsigned int maxThreads, int coef);
	/*
	 * Sequential greedy coloring algorithm
	 */
	void GreedySequential();

	int getNNodes();

private:
	void assignRandomWeights();
	/* Check if node n has a weight equal to a neighbor */
	bool weightConflict(int n);
	/* Check if node n is local maximum */
	int isLocalMaximum(int n);
	/* Get minimum available color for node n given its colored neighbors */
	int getMinColor(int n, int min_color);
	/* Check if node n has the same color as one of its neighbors */
	bool colorConflict(int n);
	/* Find nodes that can be colored according to the algorithm in the given iteration 
	  (i.e. they are not colored and they are local maxima). Used with a threadpool */
	void findNodesToColor(int from, int to);
	/* Find nodes that can be colored according to the algorithm in the given iteration
      (i.e. they are not colored and they are local maxima). Used without threadpool */
	void findNodesToColorSingleThread(int from, int to);
	/* Color nodes that have been selected by findNodesToColor. Used with threadpool */
	void ColorNodes(int from, int to);
	/* Color nodes that have been selected by findNodesToColorSingleThread. Used without threadpool */
	void ColorNodesSingleThread(int from, int to);
	/* Assign degrees of nodes as weights */
	void assignDegreeWeights();
	/* Calculate weights for SDL algorithm */
	void CalculateWeightsSDL();
	/* Find nodes to color and then color them. Used for the find and color version of the algorithms */
	void findAndColorNodes(int from, int to);

private:
	mutex _mtx, _qmtx, _mtx_colored, mutex_node_to_color, _mtx_weighted;
	condition_variable _cv, _cv_colored;
	int _n_jobs1, _k, _i, _n_jobs2, _n_threads;
	int _colored_nodes, _weighted_nodes;
	queue<function<void()>> _q;
	bool _terminate_pool;
	shared_mutex _mtx_weights;
	vector<int> _colors, _weights, _tmp_degree, _new_colors, _new_weights;
	int _n_nodes;
	bool _all_nodes_colored, _all_nodes_weighted, _increase_k;

	/* one single array containing adjacencies for each node (one vector for each node) */
	vector<vector<int>> _edges;
};

