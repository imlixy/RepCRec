#ifndef GRAPH_H
#define GRAPH_H
#include "common.h"

class SerializationGraph {
public:
	enum EdgeType {
		WW,
		RW
	};
	void addTran(const tran_id tranID);
	void addDependency(const tran_id u, const tran_id v, EdgeType type);
	void removeTran(const tran_id tranID);
	bool hasCycle(const tran_id tranID, const unordered_map<tran_id, Status>& statusList) const;
	pair<unordered_map<tran_id, EdgeType>, unordered_map<tran_id, EdgeType>> getEdges(tran_id tranID) const;

private:
	unordered_map<tran_id, unordered_map<tran_id, EdgeType>> graph;

	bool detectCycle(
		const tran_id tranID,
		const tran_id node,
		unordered_set<tran_id>& flag,
		unordered_set<tran_id>& stack,
		unordered_map<tran_id, tran_id>& parent,
		const unordered_map<tran_id, Status>& statusList
	) const;

	bool isValidCycle(
		const tran_id tranID,
		const tran_id start,
		const tran_id end,
		const unordered_map<tran_id, tran_id>& parent,
		const unordered_map<tran_id, Status>& transactionStatuses
	) const;
};

#endif