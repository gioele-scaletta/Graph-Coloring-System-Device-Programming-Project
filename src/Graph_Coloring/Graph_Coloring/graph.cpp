#define _CRT_SECURE_NO_WARNINGS

#include "graph.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <random>
#include <numeric>
#include <vector>
#include <unordered_set>
#include <string>  

using namespace std;

graph::graph()
{
}

graph::~graph()
{
}

void graph::readFileDIMACS(string fileName)
{
	this->_edges.clear();
	this->_colors.clear();
	this->_weights.clear();
	this->_tmp_degree.clear();
	this->_new_colors.clear();
	this->_new_weights.clear();
	ifstream fs(fileName);
	string line;
	if (fs.is_open()) {
		getline(fs, line);
		int n_lines, n_edges;
		// ignore header if present
		try {
			n_lines = stoi(line.substr(0, line.find(" ")));
			n_edges = stoi(line.substr(line.find(" "), line.size()));
		}
		catch (const exception e) {
			getline(fs, line);
			n_lines = stoi(line);
			n_edges = stoi(line.substr(line.find(" "), line.size()));
		}
		cout << "Number of nodes: " << n_lines << " Number of edges: " << n_edges << endl;
		this->_n_nodes = n_lines;
		this->_edges.resize(n_lines);
		for (int i = 0; i < n_lines; i++) {
			getline(fs, line);
			stringstream ss;
			ss << line;
			int n2;
			string temp;
			ss >> temp;
			// All nodes are initially not colored (color -1) and with weight -1
			this->_colors.push_back(-1);
			this->_weights.push_back(-1);
			this->_tmp_degree.push_back(-1);
			this->_new_colors.push_back(-1);
			this->_new_weights.push_back(-1);
			// Loop over all other nodes and create an edge for each of them
			while (!ss.eof()) {
				ss >> temp;
				n2 = stoi(temp);
				// Add edge in the edges vector (for both nodes -> treat each graph as indirect)
				this->_edges[i].push_back(n2 - 1);
				this->_edges[n2 - 1].push_back(i);
			}
		}
		fs.close();
	}
	else {
		cout << "Error opening file!\n";
	}

}

void graph::readFileDIMACS10(string fileName)
{
	this->_edges.clear();
	this->_colors.clear();
	this->_weights.clear();
	this->_tmp_degree.clear();
	this->_new_colors.clear();
	this->_new_weights.clear();
	ifstream fs(fileName);
	string line;
	if (fs.is_open()) {
		getline(fs, line);
		int n_lines;
		// ignore header if present
		try {
			n_lines = stoi(line);
		}
		catch (const exception e) {
			getline(fs, line);
			n_lines = stoi(line);
		}
		cout << "Number of nodes: " << n_lines << endl;
		this->_n_nodes = n_lines;
		this->_edges.resize(n_lines);
		for (int i = 0; i < n_lines; i++) {
			getline(fs, line);
			stringstream ss;
			ss << line;
			int n1, n2;
			string temp;
			ss >> temp;
			// Extract first node
			n1 = stoi(temp.substr(0, temp.find(":")));
			// All nodes are initially not colored (color -1) and with weight -1
			this->_colors.push_back(-1);
			this->_weights.push_back(-1);
			this->_tmp_degree.push_back(-1);
			this->_new_colors.push_back(-1);
			this->_new_weights.push_back(-1);
			// Loop over all other nodes and create an edge for each of them
			do {
				ss >> temp;

				if (temp != "#") {
					n2 = stoi(temp);
					// Add edge in the edges vector (for both nodes -> treat each graph as indirect)
					this->_edges[n1].push_back(n2);
					this->_edges[n2].push_back(n1);
				}
			} while (temp != "#");
		}
		fs.close();
	}
	else {
		cout << "Error opening file!\n";
	}

}

void graph::JonesPlassmanColoringParallelFindAndColor(unsigned int maxThreads, int coef)
{
	while (!this->_q.empty()) _q.pop();
	const int nodes_per_thread = floor(_n_nodes / (maxThreads )) + 1;

	this->assignRandomWeights();

	vector<thread> Pool;

	_terminate_pool = false;

	/* Create threads */
	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {waitAndExecuteJobs(); }));
	}

	_all_nodes_colored = false;

	/* Loop until all nodes are colored */
	while (!_all_nodes_colored) {

		/* _n_jobs2 will be used to synchronize threads after they have all found which nodes to color,
			then _n_jobs1 will be used to synchronize them after they have all colored their nodes */
		/* Determine how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs2 = _n_nodes / nodes_per_thread;
			else
				_n_jobs2 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs2 = _n_nodes;

		/* Determine how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		_all_nodes_colored = true;

		/* Divide the nodes into portions and schedule one job for each portion */
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			/* Schedule job */
			{
				function<void()> newJob = [this, from, to] {findAndColorNodes(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for termination of the jobs */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
	}
	/* Set termination condition to true to make threads terminate */
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	/* Join all threads */
	for (thread &t : Pool)
		t.join();
	Pool.clear();
}

/*
 * Sequential version of Jones-Plassman algorithm
 */
void graph::JonesPlassmanColoringSequential()
{
// Assign a random weight to each node 
	this->assignRandomWeights();
	bool exit = false;
	

	while (!exit) {
		exit = true;
		// Select maximal independent set
		for (int i = 0; i < _n_nodes; i++) {	
			if (_colors[i] == -1) {
				int min_color = isLocalMaximum(i);
				if (min_color != -1) {
					int new_color = getMinColor(i, min_color);
					_new_colors[i] = new_color;
				}
			}
		}
		// Color maximal independent set
		for (int i = 0; i < _n_nodes; i++)
			if (_new_colors[i] == -1)
				exit = false;
			else if (_colors[i] == -1)
				_colors[i] = _new_colors[i];
	}
}

/*
 * Standard Jones-Plassman algorithm implementation: no overlap between different iterations
 * can occur. Threads are synchronized after each iteration.
 * A threadpool is created and jobs are scheduled by the main function and executed by threads
 */
void graph::JonesPlassmanColoringParallelStandard(unsigned int maxThreads, int coef)
{
	while (!this->_q.empty()) _q.pop();

	const int nodes_per_thread = floor(_n_nodes / (maxThreads * coef)) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();

	vector<thread> Pool;
	_terminate_pool = false;
	int nodes_to_color;
	int nodes_to_color_per_thread;

	/* Create threads */
	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {waitAndExecuteJobs(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		/* Calculate how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			/* Schedule jobs to find nodes to color */
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		/* Wait for termination of jobs */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
		
		/* Calculate how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Schedule jobs to color the previously selected nodes */
			{
				function<void()> newJob = [this, from , to] {ColorNodes(from, to);};

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for termination of jobs */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
	}
	/* Set termination condition to make threads terminate */
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	/* Join threads */
	for (thread &t : Pool)
		t.join();
	Pool.clear();

}

/*
 * Standard implementation of the LDF algorithm. No overlap between different iterations.
 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
 * which are synchronized after each iteration
 */
void graph::LargestDegreeFirstStandard(unsigned int maxThreads, int coef)
{
	while (!this->_q.empty()) _q.pop();
	const int nodes_per_thread = floor(_n_nodes / (maxThreads * coef)) +1;

	this->assignDegreeWeights();

	vector<thread> Pool;
	_terminate_pool = false;

	/* Create threads */
	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {waitAndExecuteJobs(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		/* Determine how many jobs are scheduled (to wait for them) */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			/* Schedule jobs to find nodes to color */
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for jobs termination */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}

		/* Determine how many jobs are scheduled (to wait for them) */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;
		_all_nodes_colored = true;


		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Schedule jobs to color previously selected nodes */
			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		/* Wait for jobs termination */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
	}
	/* Set termination condition to terminate threads */
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	for (thread &t : Pool)
		t.join();
	Pool.clear();
}



/*
 * Implementation of the LDF algorithm. No overlap between different iterations.
 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
 * which are synchronized after each iteration. The same job finds the nodes to color, it
 * places them in a local queue and it colors them after all nodes have been found
 */
void graph::LargestDegreeFirstFindAndColor(unsigned int maxThreads, int coef)
{
	while (!this->_q.empty()) _q.pop();

	const int nodes_per_thread = floor(_n_nodes / (maxThreads )) + 1;

	this->assignDegreeWeights();

	vector<thread> Pool;
	_terminate_pool = false;

	/* Create threads */
	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {waitAndExecuteJobs(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {
		
		/* _n_jobs2 will be used to synchronize threads after they have all found which nodes to color,
		then _n_jobs1 will be used to synchronize them after they have all colored their nodes */
		/* Determine how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs2 = _n_nodes / nodes_per_thread;
			else
				_n_jobs2 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs2 = _n_nodes;

		/* Determine how many jobs are scheduled (to wait for them) */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Schedule jobs */
			{
				function<void()> newJob = [this, from, to] {findAndColorNodes(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for jobs termination */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
	}
	/* Set termination condition for threads */
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	for (thread &t : Pool)
		t.join();
	Pool.clear();
}

/*
 * Sequential implementation of the Smallest Degree Last algorithm 
 */
void graph::SmallestDegreeLastSequential() 
{
	this->CalculateWeightsSDL();
	int colored_nodes = 0;
	int min_color;

	while (colored_nodes != _n_nodes) {
		for (int n = 0; n < _n_nodes; n++) {
			if (_colors[n] == -1) {
				min_color = isLocalMaximum(n);
				if (min_color != -1) {
					_colors[n] = getMinColor(n, min_color);
					colored_nodes++;
				}
			}
		}
	}
}



/*
 * Standard implementation of the Smallest Degree Last algorithm where different iterations
 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
 * synchronized after each iteration. Weight assignment is not parallelized. 
 */
void graph::SmallestDegreeLastStandard(unsigned int maxThreads, int coef)
{
	while (!this->_q.empty()) _q.pop();
	const int nodes_per_thread = ceil(_n_nodes / (maxThreads * coef)) + 1;

	this->CalculateWeightsSDL();
	vector<thread> Pool;

	_terminate_pool = false;

	/* Create threads */
	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {waitAndExecuteJobs(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		/* Determine how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Schedule jobs to find nodes to color */
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for job termination */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}

		/* Determine how many jobs are scheduled */
		if (nodes_per_thread != 1) {
			if (_n_nodes % nodes_per_thread == 0)
				_n_jobs1 = _n_nodes / nodes_per_thread;
			else
				_n_jobs1 = ceil(_n_nodes / nodes_per_thread) + 1;
		}
		else
			_n_jobs1 = _n_nodes;

		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Schedule jobs to color previously selected nodes */
			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		/* Wait for job termination */
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_jobs1 == 0; });
		}
	}
	/* Set termination condition for threads */
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	for (thread &t : Pool)
		t.join();
	Pool.clear();
}

/*
 * Sequential greedy coloring algorithm
 */
void graph::GreedySequential()
{
	// Generate a random permutation on indices of graph nodes vector
	auto engine = std::default_random_engine{};
	vector<int> indices(_n_nodes);
	iota(indices.begin(), indices.end(), 0);
	shuffle(indices.begin(), indices.end(), engine);
	// Iterate over nodes and assign the minimum available color
	for (auto i : indices)
	{
		int color = this->getMinColor(i, -1);
		this->_colors[i] = color;
	}
}

int graph::getNNodes()
{
	return _n_nodes;
}



/*
 * Jones-Plassman implementation where threads work on a specific portion of the graph
 * and they are synchronized after each iteration. No overlaps between different iterations.
 * Synchronization is done by means of condition variables (a thread is launched as soon as
 * one of the previous threads finishes)
 */
void graph::JonesPlassmanColoringParallelNoThreadpool(unsigned int maxThreads, int coef)
{
	const int nodes_per_thread = ceil(_n_nodes / (coef * maxThreads)) + 1;
	int colored_nodes = 0;
	clock_t start, end;

	start = clock();
	// Assign a random weight to each node 
	this->assignRandomWeights();
	_n_threads = 0;
	end = clock();
	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		start = clock();

		// FIRST LOOP: find nodes to color and place them in a vector
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			/* Create threads */
			thread t([this, from, to] {findNodesToColorSingleThread(from, to); });
			t.detach();
			{
				lock_guard<mutex> lck(_mtx);
				_n_jobs1++;
			}
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_threads < maxThreads; });
		}
		{
			// Wait for termination of remaining threads
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this] {return _n_threads == 0; });
		}			
		end = clock();

		start = clock();
		// SECOND LOOP: color nodes in the vector
		
		_all_nodes_colored = true;
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			thread t([this, from, to] {ColorNodesSingleThread(from, to); });
			t.detach();
			{
				lock_guard<mutex> lck(_mtx);
				_n_threads++;
			}
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_threads < maxThreads; });
		}
		{
			// Wait for termination of remaining threads
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this] {return _n_threads == 0; });
		}
	}
}


void graph::waitAndExecuteJobs()
{
	function<void()> job;
	while (true) {
		{
			/* Wait for a job or for the termination condition */
			unique_lock<mutex> lck(_qmtx);
			_cv.wait(lck, [this] {return !this->_q.empty() || this->_terminate_pool; });
			if (this->_terminate_pool && this->_q.empty())
				return;
			job = this->_q.front();
			this->_q.pop();
		}
		/* Execute job */
		job();
	}
}


void graph::findNodesToColor(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {
		
		if (_colors[n_id] == -1) {
			/* If the node is a local maximum, find the minimum available color for it */
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				_new_colors[n_id] = getMinColor(n_id, min_color);
			}
		}
	}
	{
		lock_guard<mutex> lck(_mtx);
		_n_jobs1--;
		_cv_colored.notify_all();
	}
}


void graph::findNodesToColorSingleThread(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {

		if (_colors[n_id] == -1) {
			/* If the node is a local maximum, find the minimum available color for it 
			   and save it the _new_colors vector so that the following jobs can color it */
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				_new_colors[n_id] = getMinColor(n_id, min_color);
			}
		}
	}
	lock_guard<mutex> lck(_mtx);
	_n_threads--;
	_cv.notify_all();
}


void graph::ColorNodesSingleThread(int from, int to)
{
	/* Color nodes found by previous functions */
	for (int n_id = from; n_id < to; n_id++) {
		if (_new_colors[n_id] == -1)
			_all_nodes_colored = false;
		else
			_colors[n_id] = _new_colors[n_id];
	}
	lock_guard<mutex> lck(_mtx);
	_n_threads--;
	_cv.notify_all();
}



void graph::ColorNodes(int from, int to)
{
	/* Color nodes found by previous functions */
	for (int n_id = from; n_id < to; n_id++) {
		if (_new_colors[n_id] == -1)
			_all_nodes_colored = false;
		else
			_colors[n_id] = _new_colors[n_id];
	}
	lock_guard<mutex> lck(_mtx);
	_n_jobs1--;
	_cv_colored.notify_all();
}


int graph::checkColoring()
{
	/* Check if coloring is correct (no neighboring nodes have the same color) */
	int max_color = 0;
	for (int i = 0; i < _n_nodes; i++) {
		if (this->colorConflict(i))
			return -1;
		if (this->_colors[i] >= max_color)
			max_color = this->_colors[i] + 1;
	}
	
	return max_color;
}


void graph::printColoring()
{
	for (vector<int>::iterator it = _colors.begin(); it != _colors.end(); ++it) {
		cout << *it;
	}
}

void graph::cancelColors()
{
	for (int i = 0; i < _n_nodes; i++) {
		_colors[i] = -1;
		_weights[i] = -1;
		_new_colors[i] = -1;
		_new_weights[i] = -1;
	}
}


void graph::assignRandomWeights()
{
	for (int i = 0; i < _weights.size(); i++) {
		_weights[i] = rand();
		// Avoid conflicts
		while (weightConflict(i)) {
			_weights[i] = rand() * (rand() % 100);
		}
	}
}

void graph::assignDegreeWeights()
{
	for (int i = 0; i < _n_nodes; i++) {
		_weights[i] = _edges[i].size(); // degree is size of adjacency vector
		while (weightConflict(i)) // in case of conflict, assign random weight (with 50% prob of being lower)
			_weights[i] = rand() % (2 * _edges[i].size());
	}
}

void graph::CalculateWeightsSDL()
{
	int max_degree = _n_nodes;
	int i=1, k=1;
	int weighted_nodes = 0;
	queue<int> toWeight;

	for (int n = 0; n < _n_nodes; n++) {
		// set tmp degree to actual degree of the node
		_tmp_degree[i] = _edges[i].size();
	}

	while (weighted_nodes < _n_nodes) {
		for (int n = 0; n < _n_nodes; n++) {
			if (_tmp_degree[n] <= k && _weights[n] == -1)
				toWeight.push(n);
		}
		if (toWeight.empty())
			k++;
		while (!toWeight.empty()) {
			int n = toWeight.front();
			toWeight.pop();
			_weights[n] = i;
			while (weightConflict(n))
				_weights[n] = rand() % (_weights[n] * 2) + 1;
			weighted_nodes++;
			// Decrease tmp degrees of neighboring nodes
			for (int adj_node : _edges[n])
				if(_weights[adj_node] == -1)
					_tmp_degree[adj_node]--;
		}
		i++;
	}
}

void graph::findAndColorNodes(int from, int to)
{
	// Find nodes to color and store them in a local queue
	queue<shared_ptr<pair<int, int>>> nodes_to_color;
	for (int n_id = from; n_id < to; n_id++) {
		if (_colors[n_id] == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				/* Save nodes to color in a local queue */
				nodes_to_color.push(make_shared<pair<int, int>>(n_id, getMinColor(n_id, min_color)));
			}
		}
	}
	{
		lock_guard<mutex> lck(_mtx);
		_n_jobs2--;
		_cv_colored.notify_all();
	}
	{   // wait for all other threads to find nodes 
		unique_lock<mutex> lck(_mtx);
		_cv_colored.wait(lck, [this] {return _n_jobs2 == 0;});
	}
	// Color the nodes in the local queue
	if (!nodes_to_color.empty())
		_all_nodes_colored = false;
	while (!nodes_to_color.empty()) {
		_colors[nodes_to_color.front()->first] = nodes_to_color.front()->second;
		nodes_to_color.pop();
	}
	{
		lock_guard<mutex> lck(_mtx);
		_n_jobs1--;
		_cv_colored.notify_all();
	}
}

bool graph::weightConflict(int n)
{
	int cur_weight = _weights[n];

	if (cur_weight == -1)
		return false;

	for (auto i : _edges[n])
		if (_weights[i] == cur_weight)
			return true;
	
	return false;
}

int graph::isLocalMaximum(int n)
{
	int cur_weight;
	cur_weight = _weights[n];
	
	int min_color = -1;
	if (cur_weight == -1)
		return false;

	if (_edges[n].size() == 0)
		min_color = 0;
	for (auto adj_node : _edges[n]) {
		// Check if current adjacent node has bigger weight than original node
		if (_colors[adj_node] == -1 && _weights[adj_node] >= cur_weight) {
			return -1;
		}
		// Check if current adjacent node has bigger color than min color
		if (_colors[adj_node] >= min_color)
			min_color = _colors[adj_node] + 1;
	}
	return min_color;
}


int graph::getMinColor(int n, int min_color)
{
	/* TWO OPTIONS:
	 * 1. Get min color as a parameter and use it to allocate vector
	 * 2. If min_color = -1, perform two iterations on neighboring nodes: the first one finds the highest color used,
	 * the second one looks for the minimum available color using a vector that has as many cells
	 * as the number of used colors
	 */

	int i = 0;

	
	if (min_color == -1) {
		// Iterate over all neighboring nodes to find minimum color
		for (auto adj_node : _edges[n]) {
			if (_colors[adj_node] >= min_color)
				min_color = _colors[adj_node] + 1;
		}
	}
	

	if (min_color == -1 || min_color == 0)
		return 0;

	vector<int> used_colors(min_color, 0);
	for (auto adj_node : _edges[n]) {
		if (_colors[adj_node] != -1)
			used_colors[_colors[adj_node]] = 1;
	}
	for (i = 0; i < used_colors.size(); i++)
		if (used_colors[i] == 0)
			return i;

	return i;
}


bool graph::colorConflict(int n)
{
	int cur_color = _colors.at(n);

	if (cur_color == -1)
		return true;

	for (auto i : _edges[n]) {
		if (_colors[i] == cur_color) {
			cout << "Problem with nodes: " << n << " and " << i << endl;
			return true;
		}
	}

	return false;
}

