#ifndef GRAPH_H
#define GRAPH_H
#include "common.h"

class SerializationGraph {
public:
	void addTran(const tran_id tranID);
	void addDependency(const tran_id u, const tran_id v);
	void removeTran(const tran_id tranID);
	vector<int> getCycle();

private:
	unordered_map<tran_id, unordered_set<tran_id>> graph;
	bool detectCycle(
		const tran_id node,
		unordered_set<tran_id>& flag,
		unordered_set<tran_id>& stack,
		unordered_map<tran_id, tran_id>& parent,
		vector<tran_id>& cyclePath
	);
	vector<tran_id> traceCycle(
		const tran_id start,
		const tran_id end,
		const unordered_map<tran_id, tran_id>& parent
	);
};

#endif