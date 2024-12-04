#include "DM.h"

DataManager::DataManager(site_id id) : siteID(id) {
	status.available = true;
	status.failTime = -1.0;
	status.recoverTime = 0.0;

	for (int i = 1; i <= VAR_NUM; ++i) {
		if ((i % 2 == 0) || (1 + (i % 10) == siteID)) {
			Variable var;
			var.lastCommitTime = 0.0;
			var.value = i * 10;
			var.versionHistory[0.0] = var.value;
			variables[i] = var;
		}
	}
}

int DataManager::read(const tranID, const var_id variable, const double startTime) {
	if (variables.find(variable) == variables.end())
		return -1;	// not exsist

	Variable& var = variables[variable];

	auto it = var.versionHistory.upper_bound(startTime);
	if (it == var.versionHistory.begin())
		return -1;		// no suitable version

	--it;
	return it->second;
}

bool DataManager::write(const tran_id tranID, const var_id var, const int value) {
	if (!status.available) {
		cout << "Write Failed, site not available!" << endl;
		return false;
	}

	auto it = variables.find(var);
	if (it == variables.end()) {
		cout << "Write Failed, variable not exist!" << endl;
		return false;
	}

	cacheWrites[tranID][var_id] = value;
	return true;
}

void DataManager::commitWrite(const tran_id tranID, const var_id var, const int value) {
	if (!variables.count(var)) {
		cout << "Commit failed!" << endl;
		return;
	}
	Variable& variable = variables[var];
	variable.lastCommitTime = static_cast<double>(time(nullptr));
	variable.value = value;
	variable.versionHistory[variable.lastCommitTime] = value;

	cacheWrites[tranID].erase(var);
	if (cacheWrites[tranID].empty())
		cacheWrites.erase(tranID);
	//debug
	cout << "T" << tranID << " committed x" << var << "=" << value << endl;
	return;
}

void DataManager::abortWrite(const tran_id tranID, const var_id var) {
	if (cacheWrites.find(tranID) == cacheWrites.end()) {
		cout << "Abort Failed!" << endl;
		return;
	}

	if (cacheWrites[tranID].find(var) == cacheWrites[tranID].end()) {
		cout << "Abort Failed!" << endl;
		return;
	}

	cacheWrites[tranID].erase(var);
	if (cacheWrites[tranID].empty())
		cacheWrites.erase(tranID);
	//debug
	cout << "T" << tranID << " aborted write for x" << var << "=" << value << endl;
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

const unordered_map<var_id, Variable>& DataManager::getVariables() const {
	return variables;
}
