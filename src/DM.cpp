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

int DataManager::read(const tran_id tranID, const var_id variable, const double startTime) {
	if (variables.find(variable) == variables.end())
		return -1;	// not exsist

	Variable& var = variables[variable];

	auto it = var.versionHistory.lower_bound(startTime);
	if (it == var.versionHistory.begin() && it->first > startTime)
		return -1;		// no suitable version
	else if (it == var.versionHistory.end() || it->first > startTime)
		--it;
	if (it->first < status.failTime)
		return -1;
	return it->second;
}

bool DataManager::write(const tran_id tranID, const var_id var, const int value) {
	if (!status.available) {
		cout << "Write Failed, site not available!" << endl;
		return false;
	}

	if (!variables.count(var)) {
		cout << "Write Failed, variable not exist!" << endl;
		return false;
	}

	cacheWrites[tranID][var] = value;
	return true;
}

void DataManager::commitWrite(const tran_id tranID, const var_id var, const int value) {
	if (!variables.count(var)) {
		cout << "Commit failed!" << endl;
		return;
	}
	Variable& variable = variables[var];
	variable.lastCommitTime = currentTime();
	variable.value = value;
	variable.versionHistory[variable.lastCommitTime] = value;

	cacheWrites[tranID].erase(var);
	if (cacheWrites[tranID].empty())
		cacheWrites.erase(tranID);
	//debug
	cout << "T" << tranID << " committed x" << var << " = " << value << " at site " << siteID << endl;
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

//void DataManager::eraseCache() {

//}
