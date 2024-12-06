#include "TM.h"

TransactionManager::TransactionManager() {
    sites.push_back({});
    for (site_id i = 1; i <= SITE_NUM; ++i) {
        DataManager* dm = new DataManager(i);
        sites.push_back(dm);
    }
}

void TransactionManager::inputHandle(const string& inputs) {
    if (inputs.find("begin") == 0) {
        vector<string> str = split(inputs, ',', 5);
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        beginTransaction(tranID);
    }
    else if (inputs.find("R") != string::npos) {
        vector<string> str = split(inputs, ',', 1);
        if (str.size() != 2)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        var_id varID = stoi(str[1].substr(1));
        readTransaction(tranID, varID);
    }
    else if (inputs.find("W") != string::npos) {
        vector<string> str = split(inputs, ',', 1);
        if (str.size() != 3)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        var_id varID = stoi(str[1].substr(1));
        int value = stoi(str[2]);
        writeTransaction(tranID, varID, value);
    }
    else if (inputs.find("end") == 0) {
        vector<string> str = split(inputs, ',', 3);
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        endTransaction(tranID);
    }
    else if (inputs.find("fail") == 0) {
        vector<string> str = split(inputs, ',', 4);
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int siteID = stoi(str[0]);
        fail(siteID);
    }
    else if (inputs.find("recover") == 0) {
        vector<string> str = split(inputs, ',', 7);
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int siteID = stoi(str[0]);
        recover(siteID);
    }
    else if (inputs.find("dump") == 0)
        dump();
    else if (inputs.find("queryState") == 0)
        queryState();
    else
        cout << "Invalid input command: " << inputs << endl;
}



void TransactionManager::beginTransaction(tran_id tranID) {
    if (transList.count(tranID)) {
        cout << "Transaction " << tranID << " already exists." << endl;
        return;
    }
    transList[tranID] = { tranID, currentTime(), tranStatus::active, {}, {} };
    tranGraph.addTran(tranID);
    //cout << "Transaction " << tranID << " started." << endl;
}

void TransactionManager::readTransaction(const tran_id tranID, const var_id varID) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }

    Transaction& t = transList[tranID];
    if (varID % 2 == 0) {   // is replicated variable
        for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
            if (!site->isAvailable())
                continue;
            if (site->hasVariable(varID)) {
                int val = site->read(tranID, varID, t.startTime);
                if (val != -1) {
                    t.read.insert(varID);    // add into readSet

                    // update graph
                    for (const auto& [otherID, otherTran] : transList) {
                        if (otherID != tranID && otherTran.write.count(varID))
                            tranGraph.addDependency(otherID, tranID);
                    }
                    cout << "x" << varID << ": " << val << endl;
                    return;
                }
            }
        }
    }
    else {
        site_id target = 1 + (varID % 10);
        DataManager* site = sites[target];
        if (site->isAvailable()) {
            int val = site->read(tranID, varID, t.startTime);
            if (val != -1) {
                t.read.insert(varID);    // add into readSet

                // update graph
                for (const auto& [otherID, otherTran] : transList) {
                    if (otherID != tranID && otherTran.write.count(varID))
                        tranGraph.addDependency(otherID, tranID);
                }
                cout << "x" << varID << ": " << val << endl;
                return;
            }
        }
    }
    //cout << "Read Error!" << endl;
    t.status = tranStatus::aborted;
    cout << "T" << tranID << " aborts" << endl;
    //abortTransaction(tranID);
    return;
}

void TransactionManager::writeTransaction(tran_id tranID, const var_id varID, int value) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }
    Transaction& t = transList[tranID];

    // check conflict and add edge to graph
    for (const auto& [otherID, otherTran] : transList) {
        if (otherID != tranID && otherTran.write.find(varID) != otherTran.write.end()) {
            tranGraph.addDependency(otherID, tranID);
        }
    }

    // write
    t.write[varID] = make_pair(value, currentTime());
    bool writeSuccess = false;
    if (varID % 2 == 0) {        // replicated variable
        for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
            if (!site->isAvailable())
                continue;
            if (site->hasVariable(varID))
                writeSuccess = site->write(tranID, varID, value);
        }
    }
    else {
        site_id target = 1 + (varID % 10);
        DataManager* site = sites[target];
        if (site->isAvailable())
            writeSuccess = site->write(tranID, varID, value);
    }
    
    if (!writeSuccess) {
        cout << "Write Failed" << endl;
    }
}

void TransactionManager::endTransaction(tran_id tranID) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }
    
    Transaction& t = transList[tranID];
    
    /**   abort directly   **/ 
    if (t.status == tranStatus::aborted) {
        abortTransaction(tranID);
        return;
    }

    /**   check whether write can commit   **/
    for (const auto& [varID, writeValue] : t.write) {
        // is replicated variable, check for cacheWrite consistency
        if (varID % 2 == 0) {   
            for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
                //if (!site->isAvailable())
                //   continue;
                if (writeValue.second < site->getFailTime()) {
                    abortTransaction(tranID);
                    return;
                }
            }
        }
        else {      // for non-replicated variable
            site_id targetID = 1 + (varID % 10);
            DataManager* targetSite = sites[targetID];
            if (!targetSite->isAvailable()) {
                abortTransaction(tranID);
                return;
            }
            if (writeValue.second < targetSite->getFailTime()) {
                abortTransaction(tranID);
                return;
            }
            /*
            const auto& siteWrite = targetSite->getCacheWrite(tranID);
            if (siteWrite.find(varID) == siteWrite.end()) {
                abortTransaction(tranID);
                return;
            }*/
        }
    }


    // If status is “active”, check for conflicts
    if (t.status == tranStatus::active) {
        // has cycle
        if (!tranGraph.getCycle().empty()) {
            tran_id target = findTransaction(tranID);

            if (target != -1)
                transList[target].status = tranStatus::aborted; 
        }

        // check WAW, first committer wins
        vector<tran_id> conflicts = getWAWConflict(tranID);
        if (!conflicts.empty()) {
            for (tran_id c : conflicts)
                transList[c].status = tranStatus::aborted;
        }
        
    }
    commitTransaction(tranID);
}

void TransactionManager::abortTransaction(const tran_id tranID) {
    Transaction& t = transList[tranID];
    for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
        site->abortWrite(tranID);
    }
    /*
    for (const auto& [var, _] : t.write) {
        for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
            if (site->isAvailable() && site->hasVariable(var))
                site->abortWrite(tranID, var);
        }
    }*/
    t.status = tranStatus::aborted;
    transList.erase(tranID);
    tranGraph.removeTran(tranID);
    cout << "T" << tranID << " aborts" << endl;
}

void TransactionManager::commitTransaction(const tran_id tranID) {
    Transaction& t = transList[tranID];

    for (const auto& [varID, writeValue] : t.write) {
        for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
            if (!site->isAvailable())
                continue;
            if (varID % 2 != 0 && site->getSiteID() != (1 + (varID % 10)))      // non-replicated variables only write to target site
                continue;
            site->commitWrite(tranID, varID, writeValue.first);
        }
    }
    t.status = tranStatus::committed;
    transList.erase(tranID);
    tranGraph.removeTran(tranID);
    cout << "T" << tranID << " commits" << endl;
}

int TransactionManager::findTransaction(const tran_id tranID) {
    vector<tran_id> cyclePath = tranGraph.getCycle();

    for (tran_id id : cyclePath) {
        if (id != tranID)
            return id;
    }
    return -1;
}

vector<tran_id> TransactionManager::getWAWConflict(const tran_id tranID) {
    vector<tran_id> conflict;

    const auto& writeSet = transList[tranID].write;
    for (const auto& [u, vSet] : tranGraph.getGraph()) {
        for (tran_id v : vSet) {
            if (u == tranID || v == tranID) {
                const auto& other = (u == tranID) ? transList[v].write : transList[u].write;
                for (const auto& [id, _] : writeSet) {
                    if (other.count(id)) {
                        conflict.push_back((u == tranID) ? v : u);
                        break;
                    }
                }
            }
        }
    }
    return conflict;
}

void TransactionManager::fail(site_id siteID) {
    if (siteID < 1 || siteID > SITE_NUM) {
        cout << "Invalid site ID" << endl;
        return;
    }
    
    DataManager* site = sites[siteID];
    if (!site->isAvailable()) {
        cout << "Site" << siteID << " is already failed" << endl;
        return;
    }
    site->setAvailable(false);
    site->clearCache();
    
    // debug
    cout << "site" << siteID << " fail" << endl;
}

void TransactionManager::recover(site_id siteID) {
    if (siteID < 1 || siteID > SITE_NUM) {
        cout << "Invalid site" << endl;
        return;
    }
    
    DataManager* site = sites[siteID];
    site->setAvailable(true);
    
    // debug
    cout << "site" << siteID << " recover" << endl;
}

void TransactionManager::dump() {
    for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
        site_id siteID = site->getSiteID();
        cout << "site " << siteID << " - ";

        const auto& varList = site->getVariables();
        if (varList.empty()) {
            cout << "\n";
            continue;
        }

        vector<pair<var_id, int>> sorted;
        for (const auto& [id, var] : varList) {
            sorted.emplace_back(id, var.value);
        }
        sort(sorted.begin(), sorted.end());

        for (const auto& [id, val] : sorted) {
            cout << "x" << id << ": " << val << ", ";
        }
        cout << endl;
    }
}

void TransactionManager::queryState() {
    const int siteNumber = 2;

    DataManager* site = sites[siteNumber];

    const auto& cacheWrites = site->getAllCacheWrites(); // 假设 DataManager 提供此方法
    if (cacheWrites.empty()) {
        std::cout << "  No cacheWrites found.\n";
    }
    else {
        for (const auto& [tranID, writes] : cacheWrites) {
            std::cout << "  Transaction " << tranID << ":\n";
            for (const auto& [varID, value] : writes) {
                std::cout << "    x" << varID << " = " << value << "\n";
            }
        }
    }
    cout << "============" << endl;
}