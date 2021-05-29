#pragma once
#include <vector>

using namespace std;

class node
{
public:
	node();
	node(const node& toCopy);
	node(int id);
	~node();

	int getDegree();
	int getColor();
	void setColor(int color);
	void addAdjNode(int adj_node);
	void removeAdjNode(int n);
	void setWeight(int w);
	int getWeight();
	vector<int> getAdjList();

	bool operator==(const int& id);

private:
	int _id;
	int _degree;
	int _color;
	int _weight;
	vector<int> _adj_nodes;
};

