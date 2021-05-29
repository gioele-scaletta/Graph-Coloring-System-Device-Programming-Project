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


int node::getColor()
{
	return _color;
}

void node::setColor(int color)
{
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
	_weight = w;
}

int node::getWeight()
{
	return _weight;
}

vector<int> node::getAdjList()
{
	return _adj_nodes;
}

bool node::operator==(const int& id)
{
	return this->_id == id;
}
