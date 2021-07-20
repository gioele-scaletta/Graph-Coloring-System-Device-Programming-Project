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

using namespace std;

graph::graph()
{
}

graph::graph(const graph & toCopy)
{
	this->_nodes = toCopy._nodes;
	this->_edges = toCopy._edges;
}


graph::~graph()
{
}

void graph::addEdge(int n1, int n2)
{
	if (find(_edges.begin(), _edges.end(), edge(n1, n2)) == _edges.end()) {

		if (_nodes.find(n1) == _nodes.end()) 
			_nodes.emplace(n1, node(n1));
		if (_nodes.find(n2) == _nodes.end()) 
			_nodes.emplace(n2, node(n2));

		_nodes.find(n1)->second.addAdjNode(n2);
		_nodes.find(n2)->second.addAdjNode(n1);
		
		_edges.push_back(edge(n1, n2));
	}
}

void graph::removeNode(int n)
{
	if (_nodes.find(n) != _nodes.end()) {
		vector<edge>::iterator it = _edges.begin();
		while(it != _edges.end()) {
			if (it->containsNode(n)) {
				_nodes.find(it->getOtherNode(n))->second.removeAdjNode(n);
				it = _edges.erase(it);
			}
			else
				++it;
		}
	}
}

int graph::getNodesNumber()
{
	return _nodes.size();
}

int graph::getEdgesNumber()
{
	return _edges.size();
}

void graph::readFromFile(string fileName)
{
	ifstream fs(fileName);
	string line;
	if (fs.is_open()) {
		while (getline(fs, line)) {
			int n1, n2;
			sscanf(line.c_str(), "%d %d", &n1, &n2);
			this->addEdge(n1, n2);
		}
		fs.close();
	}

}

void graph::readFileDIMACS(string fileName)

{
	this->_nodes.clear();
	this->_edges.clear();
	while(!this->_q.empty())
		_q.pop();
	ifstream fs(fileName);
	string line;
	if (fs.is_open()) {
		getline(fs, line);
		int n_lines;
		// ignore header if present
		try {
			n_lines = stoi(line);
		}
		catch(const exception e){
			getline(fs, line);
			n_lines = stoi(line);
		}
		cout << "Number of nodes: " << n_lines << endl;
		for (int i = 0; i < n_lines; i++){
			getline(fs, line);
			stringstream ss;
			ss << line;
			int n1, n2;
			string temp;
			ss >> temp;
			// Extract first node
			n1 = stoi(temp.substr(0, temp.find(":")));
			// Loop over all other nodes and create an edge for each of them
			do {
				ss >> temp;

				if (temp != "#") {
					n2 = stoi(temp);
					this->addEdge(n1, n2);
				}
			} while (temp != "#");
		}
		fs.close();
	}
	else {
		cout << "Error opening file!\n";
	}

}


void graph::readFileDIMACSCSR(string fileName)
{
	this->_edgesCSR.clear();
	this->_colors.clear();
	this->_weights.clear();
	this->_tmp_degree.clear();
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
		this->_edgesCSR.resize(n_lines);
		this->_mtx_colors.resize(n_lines);
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
					this->_edgesCSR[n1].push_back(n2);
					this->_edgesCSR[n2].push_back(n1);
				}
			} while (temp != "#");
		}
		fs.close();
		/*cout << "OFFSETS: " << endl;
		for (auto i : _offsets)
			cout << i << " ";
		cout << "\nEDGES: " << endl;
		for (auto i : _edgesCSR)
			cout << i << " ";*/

	}
	else {
		cout << "Error opening file!\n";
	}

}

void graph::JonesPlassmanColoringParallelFindAndColor(const unsigned int maxThreads)
{
	while (!this->_q.empty()) _q.pop();

	// Check how many threads can be launched concurrently depending on the hardware setup
	//const unsigned int maxThreads = std::thread::hardware_concurrency();
	//const unsigned int maxThreads = 50;
	//int n_thread = 0;
	const int nodes_per_thread = floor(_n_nodes / maxThreads) + 1;

	this->assignRandomWeights();

	vector<thread> Pool;

	_terminate_pool = false;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		_N_THREADS = ceil(_n_nodes / nodes_per_thread) + 1;
		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {findAndColorNodes(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
 * LDF implementation where different iterations may overlap and jobs are
 * scheduled by the threads after executing them
 */
void graph::LargestDegreeFirst() {

	while (!this->_q.empty())
		_q.pop();
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = ceil(_n_nodes / maxThreads) + 1;

	this->assignDegreeWeights();
	vector<thread> Pool;

	int nt = 0;
	_terminate_pool = false;
	_colored_nodes = 0;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThread(); }));
	}
	for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
		int to = from + nodes_per_thread;
		if (to > _n_nodes)
			to = _n_nodes;

		{
			function<void()> newJob = [this, from, to] {checkAndColorListOfNodesQueueCounter(from, to); };
			unique_lock<mutex> lck(_qmtx);
			_q.push(newJob);
		}
		_cv.notify_one();
	}

	{
		unique_lock<mutex> lck(_mtx_colored);
		_cv_colored.wait(lck, [this] {return _colored_nodes == _n_nodes; });
	}

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
					int new_color = getMinColorCSR(i, min_color);
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
 * Jones-Plassman implementation with a threadpool and a counter to check stopping condition
 * Jobs are scheduled by the threads after executing them (different iterations may overlap)
 */
void graph::JonesPlassmanColoringParallelQueueCounter()
{
	while (!this->_q.empty())
		_q.pop();
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();
	
	vector<thread> Pool;

	int nt = 0;
	_terminate_pool = false;
	_colored_nodes = 0;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThread();}));
	} 
	for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
		int to = from + nodes_per_thread;
		if (to > _nodes.size())
			to = _nodes.size();

		{
			function<void()> newJob = [this, from, to] {checkAndColorListOfNodesQueueCounter(from, to); };
			unique_lock<mutex> lck(_qmtx);
			_q.push(newJob);
		}
		_cv.notify_one();
	}
	
	{
		unique_lock<mutex> lck(_mtx_colored);
		_cv_colored.wait(lck, [this] {return _colored_nodes == _nodes.size();});
	}

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
 * Standard Jones-Plassman algorithm implementation: no overlap between different iterations
 * can occur. Threads are synchronized after each iteration.
 * A threadpool is created and jobs are scheduled by the main function and executed by threads
 */
void graph::JonesPlassmanColoringParallelStandard()
{
	while (!this->_q.empty()) _q.pop();
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int coef = 1; // coefficient to calculate nodes per thread
	const int nodes_per_thread = floor(_n_nodes / (maxThreads * coef)) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();

	vector<thread> Pool;
	_terminate_pool = false;
	int _total_colored_nodes = 0;
	int nodes_to_color;
	int nodes_to_color_per_thread;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {
		//_nodes_to_color.clear();
		//_colored_nodes = 0;

		_n_thread = ceil(_n_nodes / nodes_per_thread)+1;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}

		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}

		_n_thread = ceil(_n_nodes / nodes_per_thread)+1;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from , to] {ColorNodes(from, to);};

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}

		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
 * Standard implementation of the LDF algorithm. No overlap between different iterations.
 * A threadpool is created, jobs are scheduled by the main function and executed by threads,
 * which are synchronized after each iteration
 */
void graph::LargestDegreeFirstStandard()
{
	while (!this->_q.empty()) _q.pop();

	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	//int n_thread = 0;
	const int nodes_per_thread = floor(_n_nodes / maxThreads) +1;

	this->assignDegreeWeights();

	vector<thread> Pool;

	_terminate_pool = false;
	int _total_colored_nodes = 0;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}

		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
void graph::LargestDegreeFirstFindAndColor()
{
	while (!this->_q.empty()) _q.pop();

	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	//int n_thread = 0;
	const int nodes_per_thread = floor(_n_nodes / maxThreads) + 1;

	this->assignDegreeWeights();

	vector<thread> Pool;

	_terminate_pool = false;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		_N_THREADS = ceil(_n_nodes / nodes_per_thread) + 1;
		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {findAndColorNodes(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
					_colors[n] = getMinColorCSR(n, min_color);
					colored_nodes++;
				}
			}
		}
	}
}



/*
 * Standard implementation of the Smallest Degree Last algorithm where different iterations
 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
 * synchronized after each iteration. Weight assignment is parallelized.
 */
void graph::SmallestDegreeLastParallelWeighing()
{
	while (!this->_q.empty()) _q.pop();

	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = floor(_n_nodes / maxThreads) + 1;

	this->CalculateWeightsSDLParallel();
	// Solve conflicts
	for (int i = 0; i < _n_nodes; i++)
		while (weightConflict(i))
			_weights[i] = rand() % (_weights[i] * 2) + 1;
			
	vector<thread> Pool;

	_terminate_pool = false;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {
		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}

		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		_all_nodes_colored = true;
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
 * Standard implementation of the Smallest Degree Last algorithm where different iterations
 * do not overlap. Jobs are scheduled by the main function and executed by threads, which are
 * synchronized after each iteration. Weight assignment is not parallelized. 
 */
void graph::SmallestDegreeLastStandard()
{
	while (!this->_q.empty()) _q.pop();

	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int coef = 1; // coefficient to calculate nodes per thread
	const int nodes_per_thread = ceil(_n_nodes / (maxThreads * coef)) + 1;

	this->CalculateWeightsSDL();
	vector<thread> Pool;

	_terminate_pool = false;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
		_n_thread = ceil(_n_nodes / nodes_per_thread) + 1;
		_all_nodes_colored = true;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx);
			_cv_colored.wait(lck, [this] {return _n_thread == 0; });
		}
	}
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
		int color = this->getMinColorCSR(i, -1);
		this->_colors[i] = color;
	}
}


/*
 * Jones-Plassman implementation where stopping condition is checked on a vector of flags
 * Jobs are scheduled by each thread after executing them, the main function also executes jobs
 * A threadpool is created
 */
void graph::JonesPlassmanColoringParallelQueueVector()
{
	while (!this->_q.empty())
		_q.pop();
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();
	
	vector<thread> Pool;

	int nt = 0;
	_terminate_pool = false;

	for (int i = 0; i < maxThreads; i++) {
		_exit.push_back(0);
		Pool.push_back(thread([this, maxThreads] {infiniteLoopThreadVector(maxThreads); }));
	}
	for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
		int to = from + nodes_per_thread;
		if (to > _nodes.size())
			to = _nodes.size();

		int* tmp = &_exit[nt++];
		
		{
			function<void()> newJob = [this, from, to, tmp] {checkAndColorListOfNodesVectorSum(from, to, tmp); };
			unique_lock<mutex> lck(_qmtx);
			_q.push(newJob);
		}
		_cv.notify_one();
	}

	function<void()> job;
	while (true) {
		{
		if (isColoredSum(maxThreads, _exit)) {
			_terminate_pool = true;
			for (thread &t : Pool)
				t.join();
			Pool.clear();
			return;
		}

		unique_lock<mutex> lck(_qmtx);
		_cv.wait(lck, [this] { return (!this->_q.empty()); });
				
			job = this->_q.front();
			this->_q.pop();
			this->_q.push(job);
		}
		_cv.notify_one();
		job();
	}
}


/*
 * Jones-Plassman implementation where threads work on a specific portion of the graph
 * and they are synchronized after each iteration. No overlaps between different iterations.
 * Synchronization is done by means of condition variables (a thread is launched as soon as
 * one of the previous threads finishes)
 */
void graph::JonesPlassmanColoringParallelBarriers()
{
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int coef = 1; // coefficient to evaluate nodes per thread (so far, best tested is 1)
	const int nodes_per_thread = ceil(_n_nodes / (coef * maxThreads)) + 1;
	int colored_nodes = 0;
	clock_t start, end;

	start = clock();
	// Assign a random weight to each node 
	this->assignRandomWeights();
	_n_thread = 0;
	end = clock();
	//cout << "Weighing took " << double(end - start) / CLOCKS_PER_SEC << endl;
	//bool all_nodes_colored = false;
	_all_nodes_colored = false;

	while (!_all_nodes_colored) {

		start = clock();
		//_nodes_to_color.clear();

		// FIRST LOOP: find nodes to color and place them in a vector
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			thread t([this, from, to] {findNodesToColorSingleThread(from, to); });
			t.detach();
			{
				lock_guard<mutex> lck(_mtx);
				_n_thread++;
			}
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_thread < maxThreads; });
		}
		{
			// Wait for termination of remaining threads
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this] {return _n_thread == 0; });
		}			
		end = clock();
		//cout << "Finding nodes to color took " << double(end - start) / CLOCKS_PER_SEC << endl;

		start = clock();
		// SECOND LOOP: color nodes in the vector
		//int n_nodes_to_color = _nodes_to_color.size();
		//int nodes_to_color_per_thread = ceil(n_nodes_to_color / (coef * maxThreads)) + 1;
		
		//for (int i = 0; i < _n_nodes; i++)
		//	_colors[i] = _new_colors[i];
		_all_nodes_colored = true;
		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;

			thread t([this, from, to] {ColorNodesSingleThread(from, to); });
			t.detach();
			{
				lock_guard<mutex> lck(_mtx);
				_n_thread++;
			}
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_thread < maxThreads; });
		}
		{
			// Wait for termination of remaining threads
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this] {return _n_thread == 0; });
		}
		//colored_nodes += n_nodes_to_color;		
		//end = clock();
		//cout << "Coloring nodes took " << double(end - start) / CLOCKS_PER_SEC << endl;
		// Check if all nodes are colored
		/*all_nodes_colored = true;
		for (auto c : _colors)
			if (c == -1) {
				all_nodes_colored = false;
				break;
			}*/
	}

}

/*
 * Jones-Plassman implementation where each thread works on a single node.
 * Synchronization is done by means of condition variables
 * No overlap between different iterations
 */
void graph::JonesPlassmanColoringParallelOneNodeThread()
{
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();

	// Assign a random weight to each node 
	this->assignRandomWeights();
	bool exit = false;

	while (!exit) {
		exit = true;
		vector<thread> threadPool;
		_n_thread = 0;

		// Launch a different thread for each node (check if it is local max and, if so, color it)
		for (map<int, node>::iterator it = this->_nodes.begin(); it != this->_nodes.end(); ++it) {

			if (it->second.getColor() == -1) {
				exit = false;
				thread t([it, this] {checkAndColorNode(it->second); });
				t.detach();
				lock_guard<mutex> lck(_mtx);
				_n_thread++;
			}

			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_thread < maxThreads; });
		}
		// Wait for termination of remaining threads
		unique_lock<std::mutex> lck(_mtx);
		_cv.wait(lck, [this] {return _n_thread == 0; });
	}

}

/*
 * Jones-Plassman implementation where stopping condition is checked by means of a vector of flags.
 * Overlaps may occur between different iterations
 */

void graph::JonesPlassmanColoringParallelVector() {

	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();
	vector<int> _exit;

	int nt;
	for (int i = 0; i < maxThreads; i++) _exit.push_back(0);

	while (!isColored(maxThreads, _exit)) {
		vector<thread> threadPool;
		_n_thread = 0;
		nt = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();

			int* tmp = &_exit[nt++];
			thread t([this, from, to, tmp] {checkAndColorListOfNodesVector(from, to, tmp); });

			t.detach();
			{
				unique_lock<mutex> lck(_mtx);
				_n_thread++;
				if (_n_thread == maxThreads) _cv.wait(lck);
			}
		}
	}
}


void graph::infiniteLoopThread()
{
	function<void()> job;
	while (true) {
		{
			unique_lock<mutex> lck(_qmtx);
			_cv.wait(lck, [this] {return !this->_q.empty() || this->_terminate_pool; });
			if (this->_terminate_pool)
				return;
			job = this->_q.front();
			this->_q.pop();
			this->_q.push(job);
			_cv.notify_one();
		}
		job();
	}
}

void graph::infiniteLoopThreadRescheduleJob()
{
	function<void()> job;
	while (true) {
		{
			unique_lock<mutex> lck(_qmtx);
			_cv.wait(lck, [this] {return !this->_q.empty() || this->_terminate_pool; });
			if (this->_terminate_pool && this->_q.empty())
				return;
			job = this->_q.front();
			this->_q.pop();
			//_cv.notify_one();
		}
		job();
	}
}

/*Do we need this?*/
void graph::infiniteLoopThreadVector(int maxThreads)
{
	function<void()> job;
	while (true) {
		{

			if (this->_terminate_pool)
				return;

			unique_lock<mutex> lck(_qmtx);
			if (!(!this->_q.empty()))
				_cv.wait(lck);

			job = this->_q.front();
			this->_q.pop();
			this->_q.push(job);
		}
		_cv.notify_one();
		job();
	}
}



void graph::checkAndColorListOfNodesQueueCounter(int from, int to)
{	
	int min_color;
	for (int n_id = from; n_id < to; n_id++) {
		// Acquire exclusive lock on current node (the color can be modified at the end so
		// no one should read its value)
		unique_lock<shared_mutex> lck(_mtx_colors[n_id]);
		min_color = -1;
		// Check if current node is already colored
		if (_colors[n_id] == -1) {
			// Find minimum color in the neighborhood
			min_color = this->isLocalMaximum(n_id);
			
			if (min_color != -1) {
				// Assign minimum available color and update number of colored nodes
				_colors[n_id] = min_color;
				{
					lock_guard<mutex> lck_cv(_mtx_colored);
					_colored_nodes++;
					_cv_colored.notify_all();
				}
			}
		}
	}
}


void graph::checkAndColorListOfNodesLargestDegreeFirst(int from, int to)
{	
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &this->_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {

				current_node->setColor(min_color);
				for (int t : current_node->getAdjList()) {
					node *tmp = &_nodes.find(t)->second;
					tmp->ex_lock();
					tmp->decreaseWeight();
					tmp->unlock();
				}
				{
					lock_guard<mutex> lck(_mtx_colored);
					_colored_nodes++;
				}
				_cv_colored.notify_all();
			}
		}
		current_node->unlock();
	}
}

void graph::checkAndColorListOfNodesVector(int from, int to, int* colored)
{	// Assign a random weight to each node 
	bool exit = true;
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &this->_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				// Use a lock to modify the common data structure
				current_node->setColor(min_color);
				exit = false;
			}
		}
		current_node->unlock();
	}
	if (exit){
		*colored = 1;
	}
	else {
		*colored = 0;
	}
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}

/*Do we need this?*/
void graph::checkAndColorListOfNodesVectorSum(int from, int to, int* colored)
{	
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &this->_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				// Use a lock to modify the common data structure
				current_node->setColor(min_color);
				*colored = *colored + 1;
			}
		}
		current_node->unlock();
	}

	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}

void graph::checkAndColorListOfNodesBarrier(int from, int to)
{	
	bool exit = true;
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				// Use a lock to modify the common data structure
				current_node->setColor(min_color);
				exit = false;
			}
		}
		current_node->unlock();
	}
	if (!exit)
		_barrier = exit;

	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}


void graph::findNodesToColor(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {
		
		if (_colors[n_id] == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				/*{
					unique_lock<mutex> lck(mutex_node_to_color);
					_nodes_to_color.push_back(pair<int, int>(n_id, getMinColorCSR(n_id, min_color)));
				}*/
				_new_colors[n_id] = getMinColorCSR(n_id, min_color);
			}
		}
	}
	{
		lock_guard<mutex> lck(_mtx);
		_n_thread--;
		_cv_colored.notify_all();
	}
}


void graph::findNodesToColorSingleThread(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {

		if (_colors[n_id] == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				_new_colors[n_id] = getMinColorCSR(n_id, min_color);
			}
		}
	}
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}


void graph::ColorNodesSingleThread(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {
		//if (_new_colors[n_id] != -1 && _colors[n_id] == -1)
		if (_new_colors[n_id] == -1)
			_all_nodes_colored = false;
		else
			_colors[n_id] = _new_colors[n_id];
	}
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}



void graph::ColorNodes(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {
		//if (_new_colors[n_id] != -1 && _colors[n_id] == -1)
		if (_new_colors[n_id] == -1)
			_all_nodes_colored = false;
		else
			_colors[n_id] = _new_colors[n_id];
	}
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv_colored.notify_all();
}


void graph::ColorNodesLDF(int from, int to)
{
	// PROBABLY USELESS
	for (int n_id = from; n_id < to; n_id++) {

		// Color current node
		_colors[_nodes_to_color[n_id].first] = _nodes_to_color[n_id].second;
		{
			lock_guard<mutex> lck(_mtx_colored);
			_colored_nodes++;
		}
	}
	_cv_colored.notify_all();
}




//LOWER LEVEL FUNCTIONS COMMON TO ALMOST ALL JASON PLASSMAN ALGO

bool graph::isColored(int n, vector<int> &array) {
	for (int i = 0; i < n; i++) {
		if (array[i] == 0) return false;
	}
	return true;
}

bool graph::isColoredSum(int n, vector<int> &array) {
	int sum = 0;
	for (int i = 0; i < n; i++)
		sum += array[i];

	if (sum == this->_nodes.size()) {
	return true;
	printf("colored");
}
	else
		return false;
}



int graph::checkColoring()
{
	int max_color = 0;
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		if (colorConflict(it->first))
			return -1;
		if (it->second.getColor() > max_color)
			max_color = it->second.getColor();
	}

	return max_color;
}



int graph::checkColoringCSR()
{
	int max_color = 0;
	//printColoring();
	for (int i = 0; i < _n_nodes; i++) {
		if (this->colorConflictCSR(i))
			return -1;
		if (this->_colors[i] >= max_color)
			// add 1 because coloring starts from 0
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
		while (weightConflict(i))
			_weights[i] = rand();
	}
}

void graph::assignDegreeWeights()
{
	for (int i = 0; i < _n_nodes; i++) {
		_weights[i] = _edgesCSR[i].size(); // degree is size of adjacency vector
		while (weightConflict(i)) // in case of conflict, assign random weight (with 50% prob of being lower)
			_weights[i] = rand() % (2 * _edgesCSR[i].size());
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
		_tmp_degree[i] = _edgesCSR[i].size();
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
			// Decrease tmp degrees of neighboring nodes (WORKS MUCH BETTER WITHOUT THIS!)
			for (int adj_node : _edgesCSR[n])
				if(_weights[adj_node] == -1)
					_tmp_degree[adj_node]--;
		}
		i++;
	}
}

void graph::findAndColorNodes(int from, int to)
{
	// Find nodes to color and store them in a local queue
	queue<pair<int, int>> nodes_to_color;
	for (int n_id = from; n_id < to; n_id++) {
		if (_colors[n_id] == -1) {
			int min_color = this->isLocalMaximum(n_id);
			if (min_color != -1) {
				nodes_to_color.push(pair<int, int>(n_id, getMinColorCSR(n_id, min_color)));
			}
		}
	}
	{
		lock_guard<mutex> lck(_mtx);
		_N_THREADS--;
		_cv_colored.notify_all();
	}
	{   // wait for all other threads to find nodes 
		unique_lock<mutex> lck(_mtx);
		_cv_colored.wait(lck, [this] {return _N_THREADS == 0;});
	}
	// Color the nodes 
	if (!nodes_to_color.empty())
		_all_nodes_colored = false;
	while (!nodes_to_color.empty()) {
		_colors[nodes_to_color.front().first] = nodes_to_color.front().second;
		nodes_to_color.pop();
	}
	{
		lock_guard<mutex> lck(_mtx);
		_n_thread--;
		_cv_colored.notify_all();
	}
}




void graph::CalculateWeightsSDLParallel()
{
	int max_degree = _n_nodes;
	vector<thread> Pool;
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = ceil(_n_nodes / maxThreads) + 1;

	_terminate_pool = false;
	_k = 1;
	_i = 1;
	int total_weighted_nodes = 0;
	int nodes_to_weigh;
	int nodes_to_weigh_per_thread;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	for (int i = 0; i < _n_nodes; i++) {
		_tmp_degree[i] = _edgesCSR[i].size();
	}

	while (total_weighted_nodes != _n_nodes) {
		//cout << "Weighted nodes: " << total_weighted_nodes << endl;
		_to_weigh.clear();
		_colored_nodes = 0;

		for (int from = 0; from < _n_nodes; from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _n_nodes)
				to = _n_nodes;
			{
				function<void()> newJob = [this, from, to] {findNodesToWeigh(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx_weighted);
			_cv_colored.wait(lck, [this] {return _colored_nodes == _n_nodes; });
		}

		if (_to_weigh.empty())
			_k++;
		else {
			nodes_to_weigh = _to_weigh.size();
			_colored_nodes = 0;

			nodes_to_weigh_per_thread = ceil(nodes_to_weigh / maxThreads) + 1;

			for (int from = 0; from < nodes_to_weigh; from += nodes_to_weigh_per_thread) {
				int to = from + nodes_to_weigh_per_thread;
				if (to > nodes_to_weigh)
					to = nodes_to_weigh;
				{
					function<void()> newJob = [this, from, to] {weighNodes(from, to); };

					unique_lock<mutex> lck(_qmtx);
					_q.push(newJob);
				}
				_cv.notify_one();
			}
			{
				unique_lock<mutex> lck(_mtx_weighted);
				_cv_colored.wait(lck, [this, nodes_to_weigh] {return _colored_nodes == nodes_to_weigh; });
			}
			total_weighted_nodes += nodes_to_weigh;
			_i++;
		}
	}
	{
		lock_guard<mutex> lck(_qmtx);
		_terminate_pool = true;
	}
	_cv.notify_all();

	for (thread &t : Pool)
		t.join();
	Pool.clear();
}


void graph::weighNodes(int from, int to) {

	for (int n_id = from; n_id < to; n_id++) {
		int n = _to_weigh[n_id];
		// Assign weight
		_weights[n] = _i;
		// Solve all conflicts at the end
		{
			unique_lock<shared_mutex> lck(_mtx_weights);
			for (int adj_node : _edgesCSR[n])
				if (_weights[adj_node] == -1)
					_tmp_degree[adj_node]--;
		}
		{
			lock_guard<mutex> lck(_mtx_weighted);
			_colored_nodes++;
		}
		_cv_colored.notify_all();
	}
}


void graph::findNodesToWeigh(int from, int to) {

	for (int n_id = from; n_id < to; n_id++) {
		
		if (_tmp_degree[n_id] <= _k && _weights[n_id] == -1) {
			unique_lock<mutex> lck(mutex_node_to_color);
			_to_weigh.push_back(n_id);
		}
		{
			lock_guard<mutex> lck(_mtx_weighted);
			_colored_nodes++;
		}
		_cv_colored.notify_all();
	}
}


bool graph::weightConflict(int n)
{
	int cur_weight = _weights[n];

	if (cur_weight == -1)
		return false;

	for (auto i : _edgesCSR[n])
		if (_weights[i] == cur_weight)
			return true;
	
	return false;
}

int graph::isLocalMaximum(int n)
{
	int cur_weight;
	{
		// Probably this lock is not even needed (no one is writing while this is reading)
		shared_lock<shared_mutex> lck(_mtx_weights);
		cur_weight = _weights[n];
	}
	int min_color = -1;
	if (cur_weight == -1)
		return false;

	if (_edgesCSR[n].size() == 0)
		min_color = 0;
	for (auto adj_node : _edgesCSR[n]) {
		// Check if current adjacent node has bigger weight than original node
		shared_lock<shared_mutex> lck1(_mtx_weights);
		shared_lock<shared_mutex> lck2(_mtx_colors[adj_node]);
		if (_colors[adj_node] == -1 && _weights[adj_node] >= cur_weight) {
			return -1;
		}
		// Check if current adjacent node has bigger color than min color
		if (_colors[adj_node] >= min_color)
			min_color = _colors[adj_node] + 1;
	}
	return min_color;
}

int graph::getMinColor(int n)
{
	int min_color = 0;

	vector<int> adj_list = _nodes.find(n)->second.getAdjList();
	for (vector<int>::iterator it = adj_list.begin(); it != adj_list.end(); ++it) {
		int current_color = _nodes.find(*it)->second.getColor();
		if (current_color >= min_color)
			min_color = current_color + 1;
	}

	return min_color;
}


int graph::getMinColorCSR(int n, int min_color)
{
	/* TWO OPTIONS:
	 * 1. Get min color as a parameter and use it to allocate vector
	 * 2. Perform two iterations on neighboring nodes: the first one finds the highest color used,
	 * the second one looks for the minimum available color using a vector that has as many cells
	 * as the number of used colors
	 */

	int i = 0;

	if (min_color == -1) {
		// Iterate over all neighboring nodes to find minimum color
		for (auto adj_node : _edgesCSR[n]) {
			if (_colors[adj_node] >= min_color)
				min_color = _colors[adj_node] + 1;
		}
	}

	if (min_color == -1 || min_color == 0)
		return 0;

	vector<int> used_colors(min_color, 0);
	for (auto adj_node : _edgesCSR[n]) {
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
	int cur_color = _nodes.find(n)->second.getColor();

	if (cur_color == -1)
		return true;

	vector<int> adj_list = _nodes.find(n)->second.getAdjList();
	for (vector<int>::iterator it = adj_list.begin(); it != adj_list.end(); ++it)
		if (_nodes.find(*it)->second.getColor() == cur_color)
			return true;

	return false;
}


bool graph::colorConflictCSR(int n)
{
	int cur_color = _colors.at(n);

	if (cur_color == -1)
		return true;

	for (auto i : _edgesCSR[n]) {
		if (_colors[i] == cur_color) {
			cout << "Problem with nodes: " << n << " and " << i << endl;
			return true;
		}
	}

	return false;
}

void graph::checkAndColorNode(node& n)
{
	n.ex_lock();
	int min_color = 0;// this->isLocalMaximum(n);

	if (min_color != -1) {
		n.setColor(min_color);
	}
	n.unlock();
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}

