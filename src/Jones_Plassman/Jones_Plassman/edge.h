#pragma once
#include "node.h"

using namespace std;

class edge
{
public:
	edge();
	edge(const edge& toCopy);
	edge(int n1, int n2);
	~edge();

	bool containsNode(int n);
	int getOtherNode(int n);
	int getFirst();
	int getSecond();

	bool operator==(const edge& toCompare1);

private:
	int _n1, _n2;
};

