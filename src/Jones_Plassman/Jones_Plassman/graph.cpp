#define _CRT_SECURE_NO_WARNINGS

#include "graph.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>
#include <condition_variable>

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

}

void graph::JonesPlassmanColoring()
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
void graph::JonesPlassmanColoringParallel()
{
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	int n_thread = 0;

	// Assign a random weight to each node 
	this->assignRandomWeights();
	bool exit = false;

	while (!exit) {
		exit = true;
		vector<thread> threadPool;
		n_thread = 0;
		
		// Launch a different thread for each node (check if it is local max and, if so, color it)
		for (map<int, node>::iterator it = this->_nodes.begin(); it != this->_nodes.end(); ++it) {
			
			if (it->second.getColor() == -1) {
				exit = false;
				threadPool.emplace_back([it, this] {checkAndColorNode(it->second);});
				n_thread++;
			}
			if (n_thread == maxThreads) {
				// Wait for termination of all threads
				for (auto& t : threadPool)
					t.join();
				threadPool.clear();
				n_thread = 0;
			}
		}
		// Wait for termination of remaining threads
		for (auto& t : threadPool)
			t.join();
	}
}

*/

/*
void graph::JonesPlassmanColoringParallel()
{
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	//int n_thread = 0;

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
				thread t([it, this] {checkAndColorNode(it->second);});
				t.detach();
				lock_guard<mutex> lck(_mtx);
				_n_thread++;
			}
			
			unique_lock<mutex> lck(_mtx);
			_cv.wait(lck, [this, maxThreads] {return _n_thread < maxThreads;});
		}
		// Wait for termination of remaining threads
		unique_lock<std::mutex> lck(_mtx);
		_cv.wait(lck, [this] {return _n_thread == 0;});
	}

}
*/

bool graph::isColored(int n, vector<int> &array) {
	for (int i = 0; i < n; i++) {
		if (array[i]==0) return false;
	}
	return true;
}

void graph::JonesPlassmanColoringParallel()
{
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	//int n_thread = 0;
	const int nodes_per_thread = floor(_nodes.size() / maxThreads) + 1;
	// Assign a random weight to each node 
	this->assignRandomWeights();
	vector<int> _exit;
	
	int nt;
	for (int i = 0; i < maxThreads; i++) _exit.push_back(0);

	while (!isColored(maxThreads, _exit)) {
		//_exit = true;
		vector<thread> threadPool;
		_n_thread = 0;
		nt = 0;

		for (int from = 0; from < _nodes.size(); from += nodes_per_thread) {
			int to = from + nodes_per_thread;
			if (to > _nodes.size())
				to = _nodes.size();

			//if (_exit.at(nt)==1) continue;

			int* tmp = &_exit[nt++];
			thread t([this, from, to, tmp] {checkAndColorListOfNodes(from, to, tmp); });

			t.detach();
			{
				unique_lock<mutex> lck(_mtx);
				_n_thread++;
				if (_n_thread == maxThreads) _cv.wait(lck);
			}
		
			
		}

	}

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
		while(weightConflict(it->first))
			it->second.setWeight(rand());
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
/*
void graph::checkAndColorNode(node& n)
{
	int min_color = this->isLocalMaximum(n);
	if (min_color != -1) {
		n.setColor(min_color);
	}
}
*/
void graph::checkAndColorNode(node& n)
{
	int min_color = this->isLocalMaximum(n);
	if (min_color != -1) {
		n.setColor(min_color);
	}
	//lock_guard<mutex> lck(_mtx);
	_n_thread--;
	_cv.notify_all();
}

void graph::checkAndColorListOfNodes(int from, int to, int* colored)
{	// Assign a random weight to each node 
	bool exit = true;
	for (int n_id = from; n_id < to; n_id++) {
		node *current_node = &_nodes.find(n_id)->second;
		if (current_node->getColor() == -1) {
			int min_color = this->isLocalMaximum(*current_node);
			if (min_color != -1) {
				// Use a lock to modify the common data structure
				current_node->setColor(min_color);
				exit = false;
				//cout << "Colored node" << current_node->getId() << " with color " << min_color << endl;
			}
		}
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


