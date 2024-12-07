#include "common.h"
#include "graph.h"

void SerializationGraph::addTran(const tran_id tranID) {
	if (!graph.count(tranID)) {
		graph[tranID] = {};
	}
}

void SerializationGraph::addDependency(const tran_id u, const tran_id v, EdgeType type) {
	if (!graph.count(u) || !graph.count(v)) {
		cerr << "Error add dependency" << endl;
		return;
	}
	graph[u][v] = type;
}

void SerializationGraph::removeTran(const tran_id tranID) {
	graph.erase(tranID);

	for (auto& [_, v] : graph)
		v.erase(tranID);
}

bool SerializationGraph::hasCycle(
	const tran_id tranID,
	const unordered_map<tran_id, Status>& statusList
) const {
	unordered_set<tran_id> flag;
	unordered_set<tran_id> stack;
	unordered_map<tran_id, tran_id> parent;

	for (const auto& [node, edges] : graph) {
		if (!flag.count(node)) {
			if (detectCycle(tranID, node, flag, stack, parent, statusList))
				return true;
		}
	}
	return false;
}

bool SerializationGraph::detectCycle(
	const tran_id tranID,
	const tran_id node,
	unordered_set<tran_id>& flag,
	unordered_set<tran_id>& stack,
	unordered_map<tran_id, tran_id>& parent,
	const unordered_map<tran_id, Status>& statusList
) const {
	flag.insert(node);
	stack.insert(node);

	for (const auto& [v, type] : graph.at(node)) {
		//if (type != EdgeType::RW)
		//	continue;

		// check the transaction's status on the path
		auto it = statusList.find(v);
		if (v != tranID && (it == statusList.end() || it->second != Status::commit))
			continue;
		parent[v] = node;

		// find cycle
		if (stack.count(v)) {
			// check if the cycle has target transaction and meet the requirement
			if (isValidCycle(tranID, node, v, parent, statusList))
				return true;
		}
		else if (!flag.count(v)) {
			if (detectCycle(tranID, v, flag, stack, parent, statusList))
				return true;
		}
	}

	stack.erase(node);
	return false;
}

bool SerializationGraph::isValidCycle(
	const tran_id tranID,
	const tran_id start,
	const tran_id end,
	const unordered_map<tran_id, tran_id>& parent,
	const unordered_map<tran_id, Status>& statusList
) const {
	vector<tran_id> cyclePath;
	cyclePath.push_back(end);

	tran_id cur = end;
	while (cur != start) {
		if (parent.find(cur) == parent.end())
			return false;

		cur = parent.at(cur);
		cyclePath.push_back(cur);
	}

	cyclePath.push_back(end);
	reverse(cyclePath.begin(), cyclePath.end());

	for (tran_id id : cyclePath) {
		if (id != tranID && statusList.at(id) != Status::commit)
			return false;
	}

	return true;
}

pair<unordered_map<tran_id, SerializationGraph::EdgeType>, unordered_map<tran_id, SerializationGraph::EdgeType>> 
SerializationGraph::getEdges(tran_id tranID) const {
	unordered_map<tran_id, EdgeType> outEdges;
	unordered_map<tran_id, EdgeType> inEdges;

	auto it = graph.find(tranID);
	if (it != graph.end())
		outEdges = it->second;

	for (const auto& [u, e] : graph) {
		auto eIt = e.find(tranID);
		if (eIt != e.end())
			inEdges[u] = eIt->second;
	}

	return { outEdges, inEdges };
}