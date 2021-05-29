#define _CRT_SECURE_NO_WARNINGS

#include "graph.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

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

	graph U = graph(*this);

	// Assign a random weight to each node 
	U.assignRandomWeights();

	while (U.getNodesNumber() > 0) {
		map<int, node>::iterator it = U._nodes.begin();
		while (it != U._nodes.end()) {
			if (U.isLocalMaximum(it->first)) {
				this->_nodes.find(it->first)->second.setColor(this->getMinColor(it->first));
				U.removeNode(it->first);
				it = U._nodes.erase(it);
			}
			else
				++it;
		}
	}

}


void graph::JonesPlassmanColoringParallel()
{

	graph U = graph(*this);
	// Check how many threads can be launched concurrently depending on the hardware setup
	const unsigned int maxThreads = std::thread::hardware_concurrency();
	int n_thread = 0;

	// Assign a random weight to each node 
	U.assignRandomWeights();

	while (U.getNodesNumber() > 0) {
		vector<thread> threadPool;
		vector<int> keys;
		n_thread = 0;
		// Collect keys of nodes
		for (map<int, node>::iterator it = U._nodes.begin(); it != U._nodes.end(); ++it)
			keys.push_back(it->first);
		// Launch a different thread for each node (check if it is local max and, if so, color it and remove it from the list)
		for (vector<int>::iterator it = keys.begin(); it != keys.end(); ++it) {
			threadPool.emplace_back([it, &U, this] {checkAndColorNode(*it, &U);});
			n_thread++;
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

bool graph::checkColoring()
{
	for (map<int, node>::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		if (colorConflict(it->first))
			return false;
	}
	
	return true;
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

bool graph::isLocalMaximum(int n)
{
	int cur_weight = _nodes.find(n)->second.getWeight();

	if (cur_weight == -1)
		return false;

	vector<int> adj_list = _nodes.find(n)->second.getAdjList();
	for (vector<int>::iterator it = adj_list.begin(); it != adj_list.end(); ++it)
		if (_nodes.find(*it)->second.getWeight() >= cur_weight)
			return false;

	return true;
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

void graph::checkAndColorNode(int n, graph* U)
{
	if (U->isLocalMaximum(n)) {
		int min_color = this->getMinColor(n);
		// Use a lock to modify the common data structure
		unique_lock<mutex> lck(_mtx);
		this->_nodes.find(n)->second.setColor(min_color);
		U->removeNode(n);
		U->_nodes.erase(n);
	}
}
