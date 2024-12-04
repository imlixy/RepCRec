#include "common.h"
#include "graph.h"

void SerializationGraph::addTran(const tran_id tranID) {
	if (graph.find(tranID) == graph.end()) {
		graph[tranID] = {};
	}
}

void SerializationGraph::addDependency(const tran_id u, const tran_id v) {
	if (graph.find(u) == graph.end() || graph.find(v) == graph.end()) {
		cerr << "Error add dependency" << endl;
		return;
	}
	graph[u].insert(v);
}

void SerializationGraph::removeTran(const tran_id tranID) {
	graph.erase(tranID);

	for (auto& [_, v] : graph)
		v.erase(tranID);
}

vector<tran_id> SerializationGraph::getCycle() {
	vector<tran_id> cyclePath;
	unordered_set<tran_id> flag;
	unordered_set<tran_id> stack;
	unordered_map<tran_id, tran_id> parent;

	for (const auto& [node, _] : graph) {
		if (flag.find(node) == flag.end()) {
			if (detectCycle(node, flag, stack, parent, cyclePath))
				return cyclePath;
		}
	}
	return {};
}

bool SerializationGraph::detectCycle(
	const tran_id node, 
	unordered_set<tran_id>& flag, 
	unordered_set<tran_id>& stack,
	unordered_map<tran_id, tran_id>& parent,
	vector<tran_id>& cyclePath
) {
	flag.insert(node);
	stack.insert(node);

	for (tran_id v : graph[node]) {
		if (stack.find(v) != stack.end()) {
			cyclePath = traceCycle(v, node, parent);
			return true;
		}
			
		if (flag.find(v) == flag.end()) {
			parent[v] = node;	// record father node
			if (detectCycle(v, flag, stack, parent, cyclePath))
				return true;
		}
	}

	stack.erase(node);
	return false;
}

vector<tran_id> SerializationGraph::traceCycle(
	const tran_id start, const tran_id end, 
	const unordered_map<tran_id, tran_id>& parent
) {
	vector<tran_id> cyclePath;
	cyclePath.push_back(end);

	tran_id cur = end;
	while (cur != start) {
		cur = parent[cur];
		cyclePath.push_back(cur);
	}

	cyclePath.push_back(end);
	reverse(cyclePath.begin(), cyclePath.end());
	return cyclePath;
}