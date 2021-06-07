#include "node.h"
#include <algorithm>


node::node()
{
	_degree = 0;
	_color = -1;
	_weight = -1;
}

node::node(const node & toCopy)
{
	this-> _id = toCopy._id;
	this->_degree = toCopy._degree;
	this->_color = toCopy._color;
	this->_weight = toCopy._weight;
	this->_adj_nodes = toCopy._adj_nodes;

}

node::node(int id)
{
	_id = id;
	_degree = 0;
	_color = -1;
	_weight = -1;
}


node::~node()
{
}

int node::getDegree()
{
	return _degree;
}

int node::getTmpDegree()
{
	//shared_lock<shared_mutex> lck(_tmp_degree_mtx);
	return _tmp_degree;
}

void node::decreaseTmpDegree()
{
	//unique_lock<shared_mutex> lck(_tmp_degree_mtx);
	_tmp_degree--;
}

void node::resetTmpDegree()
{
	_tmp_degree = _degree;
}


int node::getColor()
{
	//shared_lock<shared_mutex> lock(_color_mtx);
	return _color;
}

void node::setColor(int color)
{
	//unique_lock<shared_mutex> lock(_color_mtx);
	_color = color;
}

void node::addAdjNode(int adj_node)
{
	if (find(_adj_nodes.begin(), _adj_nodes.end(), adj_node) == _adj_nodes.end()) {
		_adj_nodes.push_back(adj_node);
		_degree++;
	}
}

void node::removeAdjNode(int n)
{
	_adj_nodes.erase(find(_adj_nodes.begin(), _adj_nodes.end(), n));
}

void node::setWeight(int w)
{
	//unique_lock<shared_mutex> lck(_weight_mtx);
	_weight = w;
}

int node::getWeight()
{
	//shared_lock<shared_mutex> lck(_color_mtx);
	return _weight;
}

void node::increaseWeight()
{
	//unique_lock<shared_mutex> lck(_weight_mtx);
	_weight++;
}


void node::decreaseWeight()
{
	//unique_lock<shared_mutex> lck(_weight_mtx);
	_weight--;
	
}

vector<int> node::getAdjList()
{
	return _adj_nodes;
}

shared_mutex * node::getPointerToMutexColor()
{
	return &_color_mtx;
}

int node::getId()
{
	return _id;
}

void node::ex_lock()
{
	_color_mtx.lock();
}

void node::unlock()
{
	_color_mtx.unlock();
}

bool node::operator==(const int& id)
{
	return this->_id == id;
}
