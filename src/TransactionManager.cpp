/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-02-2024
 * Last Edited:   12-08-2024
 * Description:   This file implements the TransactionManager class, which is 
 *                the central component of the project. It handles input 
 *                commands and orchestrates all transactional operations 
 *                including:
 *                 - Starting and ending transactions (begin, end).
 *                 - Reading and writing variables (read, write).
 *                 - Managing site failures and recoveries (fail, recover).
 *                 - Querying and dumping the state of the system (dump, queryState).
 * 
 *                The TransactionManager is responsible for coordinating with 
 *                the DataManager and SerializationGraph classes. It manages 
 *                site-level data and uses the serialization graph to track 
 *                transaction dependencies and detect conflicts. 
 * 
 *                Note: DataManager and SerializationGraph cannot invoke 
 *                TransactionManager functions. Information flow is unidirectional.
 ****************************************************************************/

#include "TransactionManager.h"
#include "common.h"

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
        return;
}



void TransactionManager::beginTransaction(tran_id tranID) {
    if (transList.count(tranID)) {
        cout << "Transaction " << tranID << " already exists." << endl;
        return;
    }
    transList[tranID] = { tranID, currentTime(), TranStatus::active, {}, {} };
    tranGraph.addTran(tranID);
    //cout << "Transaction " << tranID << " started." << endl;
}

void TransactionManager::readTransaction(const tran_id tranID, const var_id varID) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }

    Transaction& t = transList[tranID];
    vector<site_id> wait;
    if (varID % 2 == 0) {   // is replicated variable
        for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
            auto [flag, val] = site->read(varID, t.startTime);
            if (flag) {
                t.read.insert(varID);    // add into readSet

                // update graph
                for (const auto& [otherID, otherTran] : transList) {
                    if (otherID != tranID && otherTran.write.count(varID))
                        tranGraph.addDependency(tranID, otherID, SerializationGraph::RW);
                }
                cout << "x" << varID << ": " << val << endl;
                return;
            }
            else {
                if (val != -1) {
                    wait.push_back(site->getSiteID());
                    t.read.insert(varID);
                }
            }
        }
        
    }
    else {
        site_id target = 1 + (varID % 10);
        DataManager* site = sites[target];
        auto [flag, val] = site->read(varID, t.startTime);
        if (flag) {
            t.read.insert(varID);    // add into readSet

            // update graph
            for (const auto& [otherID, otherTran] : transList) {
                if (otherID != tranID && otherTran.write.count(varID))
                    tranGraph.addDependency(tranID, otherID, SerializationGraph::RW);
            }
            cout << "x" << varID << ": " << val << endl;
            return;
        }
        else {
            if (val != -1) {
                wait.push_back(site->getSiteID());
                t.read.insert(varID);
            }
        }
    }
    if (wait.empty()) {
        t.status = TranStatus::aborted;
        abortTransaction(tranID);
    }
    else {  // should wait for recover
        for (site_id id : wait)
            cout << "T" << tranID << " waits for site " << id << endl;
        t.status = TranStatus::blocked;
    }
    return;
}

void TransactionManager::writeTransaction(tran_id tranID, const var_id varID, int value) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }
    Transaction& t = transList[tranID];

    // check WAW conflict and add edge to graph
    for (const auto& [otherID, otherTran] : transList) {
        if (otherID != tranID && otherTran.write.count(varID)) {
            tranGraph.addDependency(otherID, tranID, SerializationGraph::WW);
        }
    }

    // check RW conflict
    for (const auto& [otherID, otherTran] : transList) {
        if (otherID != tranID && otherTran.read.count(varID)) {
            tranGraph.addDependency(otherID, tranID, SerializationGraph::RW);
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
        //cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }
    
    Transaction& t = transList[tranID];
    
    /**   abort directly   **/ 
    if (t.status == TranStatus::aborted || t.status == TranStatus::blocked) {
        abortTransaction(tranID);
        return;
    }

    /**   check whether write can commit   **/
    for (const auto& [varID, writeValue] : t.write) {
        // is replicated variable, check for cacheWrite consistency
        if (varID % 2 == 0) {   
            for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
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
        }
    }

    /**   detect cycle   **/ 
    unordered_map<tran_id, Status> statusList;
    for (const auto& [id, tran] : transList) {
        if (tran.status == TranStatus::committed)
            statusList[id] = Status::commit;
        else if (tran.status == TranStatus::active)
            statusList[id] = Status::running;
    }
    if (tranGraph.hasCycle(tranID, statusList)) {
        abortTransaction(tranID);
        return;
    }


    /**   check WAW, first committer wins   **/
    vector<tran_id> conflicts = getWAWConflict(tranID);
    if (!conflicts.empty()) {
        for (tran_id c : conflicts)
            transList[c].status = TranStatus::aborted;
    }

    commitTransaction(tranID);
}

void TransactionManager::abortTransaction(const tran_id tranID) {
    Transaction& t = transList[tranID];
    for (DataManager* site : vector<DataManager*>(sites.begin() + 1, sites.end())) {
        site->abortWrite(tranID);
    }
    t.status = TranStatus::aborted;
    transList.erase(tranID);
    tranGraph.removeTran(tranID);
    cout << "T" << tranID << " aborts" << endl;
    cout << endl;
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
    t.status = TranStatus::committed;
    cout << "T" << tranID << " commits" << endl;
    cout << endl;
}

vector<tran_id> TransactionManager::getWAWConflict(const tran_id tranID) {
    unordered_set<tran_id> conflictSet;

    const auto& [outEdges, inEdges] = tranGraph.getEdges(tranID);

    for (const auto& [v, type] : outEdges) {
        if (type == SerializationGraph::WW)
            conflictSet.insert(v);
    }

    for (const auto& [u, type] : inEdges) {
        if (type == SerializationGraph::WW)
            conflictSet.insert(u);
    }
    return vector<tran_id>(conflictSet.begin(), conflictSet.end());
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
    // cout << "site" << siteID << " fail" << endl;
}

void TransactionManager::recover(site_id siteID) {
    if (siteID < 1 || siteID > SITE_NUM) {
        cout << "Invalid site" << endl;
        return;
    }
    
    DataManager* site = sites[siteID];
    site->setAvailable(true);
    const auto targetSet = site->getVariables();

    for (auto& [tranID, tran] : transList) {
        if (tran.status != TranStatus::blocked)
            continue;

        for (var_id varID : tran.read) {
            if (site->hasVariable(varID)) {
                auto [flag, val] = site->read(varID, tran.startTime);
                if (flag) {
                    cout << "T" << tranID << " unblocked" << endl;
                    cout << "x" << varID << ": " << val << endl;
                    // update graph
                    for (const auto& [otherID, otherTran] : transList) {
                        if (otherID != tranID && otherTran.write.count(varID))
                            tranGraph.addDependency(tranID, otherID, SerializationGraph::RW);
                    }
                    tran.status = TranStatus::active;
                }
            }
        }
    }
    
    // debug
    // cout << "site" << siteID << " recover" << endl;
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

    const auto& cacheWrites = site->getAllCacheWrites();
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