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
	int getTmpDegree();
	void decreaseTmpDegree();
	void resetTmpDegree();
	int getColor();
	void setColor(int color);
	void addAdjNode(int adj_node);
	void removeAdjNode(int n);
	void setWeight(int w);
	void increaseWeight();
	void decreaseWeight();
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
	int _tmp_degree;
	int _color;
	int _weight;
	shared_mutex _mtx;
	mutex _weight_mutex;
	vector<int> _adj_nodes;
};

