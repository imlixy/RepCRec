#include "DM.h"

DataManager::DataManager(site_id id) : siteID(id) {
	status.available = true;
	status.failTime = -1.0;
	status.recoverTime = 0.0;

	if (id % 2 == 0) {		// has replicated variables and id-1 & (id-1)*10
		for (var_id i = 2; i <= VAR_NUM; i += 2) {
			Variable var;
			var.lastCommitTime = 0.0;
			var.value = i * 10;
			var.versionHistory[0.0] = var.value;
			variables[i] = var;
		}
		vector<var_id> list = { id - 1, id + 9 };
		for (var_id i : list) {
			Variable var;
			var.lastCommitTime = 0.0;
			var.value = i * 10;
			var.versionHistory[0.0] = var.value;
			variables[i] = var;
		}
	}
	else {		// only has replicated variables
		for (var_id i = 2; i <= VAR_NUM; i += 2) {
			Variable var;
			var.lastCommitTime = 0.0;
			var.value = i * 10;
			var.versionHistory[0.0] = var.value;
			variables[i] = var;
		}
	}
}

int DataManager::read(const tran_id tranID, const var_id varID, const double startTime) {
	if (variables.find(varID) == variables.end())
		return -1;	// not exsist

	Variable& var = variables[varID];

	auto it = var.versionHistory.lower_bound(startTime);
	if (it == var.versionHistory.begin() && it->first > startTime)
		return -1;		// no suitable version
	else if (it == var.versionHistory.end() || it->first > startTime)
		--it;
	if (it->first < status.failTime && startTime < status.failTime)
		return -2;
		
	// for replicated variable, must wait for a commit after fail
	if (varID % 2 == 0 && it->first < status.failTime)
		return -1;
	return it->second;
}

bool DataManager::write(const tran_id tranID, const var_id varID, const int value) {
	if (!status.available) {
		cout << "Write Failed, site not available!" << endl;
		return false;
	}

	if (!variables.count(varID)) {
		cout << "Write Failed, variable not exist!" << endl;
		return false;
	}

	cacheWrites[tranID][varID] = value;
	return true;
}

void DataManager::commitWrite(const tran_id tranID, const var_id varID, const int value) {
	if (cacheWrites[tranID].empty())
		return;
	
	if (!variables.count(varID)) {
		cout << "Commit failed!" << endl;
		return;
	}
	Variable& variable = variables[varID];
	variable.lastCommitTime = currentTime();
	variable.value = value;
	variable.versionHistory[variable.lastCommitTime] = value;

	cacheWrites[tranID].erase(varID);
	if (cacheWrites[tranID].empty())
		cacheWrites.erase(tranID);
	//debug
	cout << "T" << tranID << " committed x" << varID << " = " << value << " at site " << siteID << endl;
	return;
}

void DataManager::abortWrite(const tran_id tranID, const var_id var) {
	if (cacheWrites.find(tranID) == cacheWrites.end())
		return;

	if (cacheWrites[tranID].find(var) == cacheWrites[tranID].end())
		return;

	cacheWrites[tranID].erase(var);
	if (cacheWrites[tranID].empty())
		cacheWrites.erase(tranID);
	//debug
	//cout << "T" << tranID << " aborted write for x" << var << " at site" << siteID << endl;
}

void DataManager::abortWrite(const tran_id tranID) {
	if (cacheWrites.find(tranID) == cacheWrites.end())
		return;

	cacheWrites.erase(tranID);
	//debug
	//cout << "T" << tranID << " aborted write for x" << var << " at site" << siteID << endl;
}

bool DataManager::isAvailable() const {
	return status.available;
}

bool DataManager::hasVariable(var_id variable) const {
	return variables.find(variable) != variables.end();
}

site_id DataManager::getSiteID() const {
	return this->siteID;
}

double DataManager::getFailTime() const {
	return status.failTime;
}

unordered_map<var_id, DataManager::Variable>& DataManager::getVariables() {
	return variables;
}

void DataManager::setAvailable(bool flag) {
	status.available = flag;
	if (!flag)
		status.failTime = currentTime();
	else
		status.recoverTime = currentTime();
}

void DataManager::clearCache() {
	cacheWrites.clear();
}

const unordered_map<var_id, int>& DataManager::getCacheWrite(tran_id tranID) const {
	static const unordered_map<var_id, int> emptyCache;

	auto it = cacheWrites.find(tranID);
	if (it != cacheWrites.end()) {
		return it->second;
	}

	return emptyCache; // if the cache for tranID does not exsist, return empty
}

const std::unordered_map<tran_id, std::unordered_map<var_id, int>>& DataManager::getAllCacheWrites() const {
	return cacheWrites;
}

