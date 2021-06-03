#pragma once
#include <vector>
#include <shared_mutex>

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
	shared_mutex* getPointerToMutexColor();
	int getId();
	void ex_lock();
	void unlock();

	bool operator==(const int& id);

private:
	int _id;
	int _degree;
	int _color;
	int _weight;
	shared_mutex _mtx;
	vector<int> _adj_nodes;
};

