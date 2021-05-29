#include "edge.h"



edge::edge()
{
}

edge::edge(const edge & toCopy)
{
	this->_n1 = toCopy._n1;
	this->_n2 = toCopy._n2;
}

edge::edge(int n1, int n2)
{
	_n1 = n1;
	_n2 = n2;
}


edge::~edge()
{
}

bool edge::containsNode(int n)
{
	return (_n1 == n || _n2 == n);
}

int edge::getOtherNode(int n)
{
	if (_n1 == n)
		return _n2;
	else if (_n2 == n)
		return _n1;
	else
		return -1;
}

int edge::getFirst()
{
	return _n1;
}

int edge::getSecond()
{
	return _n2;
}

bool edge::operator==(const edge & toCompare1)
{
	// Since graphs are not directed, order of nodes in edge is not relevant
	return (toCompare1._n1 == this->_n1 && toCompare1._n2 == this->_n2) || 
		(toCompare1._n1 == this->_n2 && toCompare1._n2 == this->_n1);
}
