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

/*
 * LDF implementation where different iterations may overlap and jobs are
 * scheduled by the threads after executing them
 */
void graph::LargestDegreeFirst() {

	while (!this->_q.empty())
		_q.pop();
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;

	this->assignDegreeWeights();
	vector<thread> Pool;

	int nt = 0;
	_terminate_pool = false;
	_colored_nodes = 0;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThread(); }));
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
		_cv_colored.wait(lck, [this] {return _colored_nodes == _nodes.size(); });
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

		for (map<int, node>::iterator it = this->_nodes.begin(); it != this->_nodes.end(); ++it) {
			
			if (it->second.getColor() == -1) {
				exit = false;
				int min_color = this->isLocalMaximum(it->second);
				if (min_color != -1) {
					// Use a lock to modify the common data structure
					it->second.setColor(min_color);
				}
			}
		}
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
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
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

	while (_total_colored_nodes != _nodes.size()) {
		_nodes_to_color.clear();
		_colored_nodes = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();

			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}

		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this] {return _colored_nodes == _nodes.size(); });
		}
		
		nodes_to_color = _nodes_to_color.size();
		_colored_nodes=0;

		nodes_to_color_per_thread = ceil(nodes_to_color / maxThreads)+1;

		for (int from = 0; from < nodes_to_color; from += nodes_to_color_per_thread) {
			int to = from + nodes_to_color_per_thread;
			if (to > nodes_to_color)
				to = nodes_to_color;

			{
				function<void()> newJob = [this, from , to] {ColorNodes(from, to);};

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}

		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this, nodes_to_color] {return _colored_nodes == nodes_to_color; });
		}
		_total_colored_nodes += nodes_to_color;
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
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) +1;

	this->assignDegreeWeights();

	vector<thread> Pool;

	_terminate_pool = false;
	int _total_colored_nodes = 0;

	int nodes_to_color;
	int nodes_to_color_per_thread;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	while (_total_colored_nodes != _nodes.size()) {

		_nodes_to_color.clear();
		_colored_nodes = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();

			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this] {return _colored_nodes == _nodes.size(); });
		}

		nodes_to_color = _nodes_to_color.size();
		_colored_nodes = 0;

		nodes_to_color_per_thread = ceil(nodes_to_color / maxThreads)+1 ;

		for (int from = 0; from < nodes_to_color; from += nodes_to_color_per_thread) {
			int to = from + nodes_to_color_per_thread;
			if (to > nodes_to_color)
				to = nodes_to_color;
			{
				function<void()> newJob = [this, from, to] {ColorNodesLDF(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this, nodes_to_color] {return _colored_nodes == nodes_to_color; });
		}
		_total_colored_nodes += nodes_to_color;
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
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;

	this->CalculateWeightsSDF();
	vector<thread> Pool;

	_terminate_pool = false;
	int _total_colored_nodes = 0;
	int nodes_to_color;
	int nodes_to_color_per_thread;

	for (int i = 0; i < maxThreads; i++) {
		Pool.push_back(thread([this] {infiniteLoopThreadRescheduleJob(); }));
	}

	while (_total_colored_nodes != _nodes.size()) {

		_nodes_to_color.clear();
		_colored_nodes = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();
			{
				function<void()> newJob = [this, from, to] {findNodesToColor(from, to); };
				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);
			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this] {return _colored_nodes == _nodes.size(); });
		}

		nodes_to_color = _nodes_to_color.size();
		_colored_nodes = 0;
		nodes_to_color_per_thread = ceil(nodes_to_color / maxThreads) + 1;

		for (int from = 0; from < nodes_to_color; from += nodes_to_color_per_thread) {
			int to = from + nodes_to_color_per_thread;
			if (to > nodes_to_color)
				to = nodes_to_color;

			{
				function<void()> newJob = [this, from, to] {ColorNodes(from, to); };

				unique_lock<mutex> lck(_qmtx);
				_q.push(newJob);

			}
			_cv.notify_one();
		}
		{
			unique_lock<mutex> lck(_mtx_colored);
			_cv_colored.wait(lck, [this, nodes_to_color] {return _colored_nodes == nodes_to_color; });
		}
		_total_colored_nodes += nodes_to_color;
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
	vector<int> indices(_nodes.size());
	iota(indices.begin(), indices.end(), 0);
	shuffle(indices.begin(), indices.end(), engine);
	// Iterate over nodes and assign the minimum available color
	for (auto i : indices)
	{
		int color = this->getMinColor(i);
		_nodes.find(i)->second.setColor(color);
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
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();

	while (!_barrier) {
		_barrier = true;
		vector<thread> threadPool;
		_n_thread = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();

			thread t([this, from, to] {checkAndColorListOfNodesBarrier(from, to); });
			t.detach();
			{
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
		}
		_cv.notify_one();
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
			
		}
		_cv.notify_one();
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
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &this->_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(*current_node);
			if (min_color != -1) {
				
				current_node->setColor(min_color);
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


void graph::checkAndColorListOfNodesLargestDegreeFirst(int from, int to)
{	
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &this->_nodes.find(n_id)->second;
		current_node->ex_lock();
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(*current_node);
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
			int min_color = this->isLocalMaximum(*current_node);
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
			int min_color = this->isLocalMaximum(*current_node);
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
			int min_color = this->isLocalMaximum(*current_node);
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
		node *current_node = &this->_nodes.find(n_id)->second;
		
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(*current_node);
			if (min_color != -1) {
				{
					unique_lock<mutex> lck(mutex_node_to_color);
					_nodes_to_color.push_back(pair<node*, int>(current_node, min_color));
				}
			}
		}
		{
			lock_guard<mutex> lck(_mtx_colored);
			_colored_nodes++;
		}
		_cv_colored.notify_all();
	}
}

void graph::ColorNodes(int from, int to)
{
	for (int n_id = from; n_id < to; n_id++) {
		_nodes_to_color[n_id].first->setColor(_nodes_to_color[n_id].second);
		{
			lock_guard<mutex> lck(_mtx_colored);
			_colored_nodes++;
		}
	}
	_cv_colored.notify_all();
}


void graph::ColorNodesLDF(int from, int to)
{
	int t;
	for (int n_id = from; n_id < to; n_id++) {

		_nodes_to_color[n_id].first->setColor(_nodes_to_color[n_id].second);

		vector<int> adj_list = _nodes_to_color[n_id].first->getAdjList();
		for (int i = 0; i < adj_list.size(); i++) {
			node *tmp = &_nodes.find(adj_list[i])->second;
		
			if(tmp->getColor()!=-1)
			tmp->decreaseWeight();
			
		}
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

void graph::printColoring()
{
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		cout << "Node: " << it->first << " Color: " << it->second.getColor() << endl;
	}
}

void graph::cancelColors()
{
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it)
		it->second.setColor(-1);
}


void graph::assignRandomWeights()
{
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		it->second.setWeight(rand());
		while (weightConflict(it->first))
			it->second.setWeight(rand());
	}
}

void graph::assignDegreeWeights()
{
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		it->second.setWeight(it->second.getDegree());
		while (weightConflict(it->first))
			it->second.setWeight(rand()%(it->second.getDegree()*2));
	}
}


void graph::CalculateWeightsSDF()
{
	int max_degree = _nodes.size();
	int i=0, k=0;
	queue<node*> toWeight;

	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		it->second.resetTmpDegree();
		it->second.setWeight(-1);

	}

	while (k < max_degree) {
		for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
			if (it->second.getTmpDegree() <= k && it->second.getWeight()==-1)
				toWeight.push(&it->second);
		}
		if (toWeight.empty())
			k++;
		while (!toWeight.empty()) {
			node* tmp = toWeight.front();
			toWeight.pop();
			tmp->setWeight(i);
			while (weightConflict(tmp->getId()))
				tmp->setWeight(rand() % (tmp->getDegree() * 2));
			for (int t : tmp->getAdjList())
				if(_nodes.find(t)->second == -1)
					_nodes.find(t)->second.decreaseTmpDegree();
		}
		i++;
	}
}

bool graph::weightConflict(int n)
{
	int cur_weight = _nodes.find(n)->second.getWeight();

	if (cur_weight == -1)
		return false;

	vector<int> adj_list = _nodes.find(n)->second.getAdjList();
	for (vector<int>::iterator it = adj_list.begin(); it != adj_list.end(); ++it)
		if (_nodes.find(*it)->second.getWeight() == cur_weight)
			return true;

	return false;
}

int graph::isLocalMaximum(node& n)
{
	int cur_weight = n.getWeight();
	int min_color = -1;
	if (cur_weight == -1)
		return false;

	vector<int> adj_list = n.getAdjList();
	if (adj_list.size() == 0)
		min_color = 0;
	for (vector<int>::iterator it = adj_list.begin(); it != adj_list.end(); ++it) {
		// Check if current adjacent node has bigger weight than original node
		node adj_node = this->_nodes.find(*it)->second;
		if (adj_node.getColor() == -1 && adj_node.getWeight() >= cur_weight) {
			return -1;
		}
		// Check if current adjacent node has bigger color than min color
		int cur_color = _nodes.find(*it)->second.getColor();
		if (cur_color >= min_color)
			min_color = cur_color + 1;
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


void graph::checkAndColorNode(node& n)
{
	n.ex_lock();
	int min_color = this->isLocalMaximum(n);

	if (min_color != -1) {
		n.setColor(min_color);
	}
	n.unlock();
	lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}

