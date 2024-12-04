#include "TM.h"

vector<string> split(const string& line, char delimiter) {
    string str = line.substr(2, line.size() - 3);
    vactor<string> tokens;
    string token;
    for (char ch : str) {
        if (ch == delimiter) {
            if (!token.empty())
                tokens.push_back(token);
            token.clear();
        }
        else
            token += ch;
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

TransactionManager::TransactionManager() {
    for (int i = 1; i <= SITE_NUM; ++i) {
        DataManager* dm = new DataManager(i);
        sites.push_back(dm);
    }
}

void TransactionManager::inputHandle(const string& inputs) {
    if (inputs.find("begin") == 0) {
        vector<string> str = split(inputs, ',');
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        beginTransaction(tranID);
    }
    else if (inputs.find("R") != string::npos) {
        vector<string> str = split(input, ',');
        if (str.size() != 2)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        var_id variable = stoi(str[1].substr(1));
        readTransaction(tranID, variable);
    }
    else if (inputs.find("W") != string::npos) {
        vector<string> str = split(input, ',');
        if (str.size() != 3)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        var_id variable = stoi(str[1].substr(1));
        int value = stoi(str[2].substr(1));
        writeTransaction(tranID, variable, value);
    }
    else if (inputs.find("end") == 0) {
        vector<string> str = split(inputs, ',');
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int tranID = stoi(str[0].substr(1));
        endTransaction(tranID);
    }
    else if (inputs.find("fail") == 0) {
        vector<string> str = split(inputs, ',');
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int siteID = stoi(str[0].substr(1));
        fail(siteID);
    }
    else if (inputs.find("recover") == 0) {
        vector<string> str = split(inputs, ',');
        if (str.size() != 1)
            cout << "Invalid input command: " << inputs << endl;

        int siteID = stoi(str[0].substr(1));
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

void TransactionManager::readTransaction(const tran_id tranID, const var_id variable) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }

    Transaction& t = transList[tranID];
    site_id target = 1 + (variable % 10);
    for (DataManager* site : sites) {
        if (!site->isAvailable())
            continue;
        if (variable % 2 == 0 && site->getSiteID() != target)
            continue;

        if (site->hasVariable(variable)) {
            int val = site->read(tranID, variable, t.startTime);
            if (val != -1) {
                t.read.insert(variable);    // add into readSet

                // update graph
                for (const auto& [otherID, otherTran] : transList) {
                    if (otherID != tranID && otherTran.write.count(variable))
                        tranGraph.addDependency(otherID, tranID);
                }
            }
            cout << "x" << variable << ": " << val << endl;
            return;
        }
    }
    cout << "Read Error!" << endl;
    return;
}

void TransactionManager::writeTransaction(tran_id tranID, const var_id var, int value) {
    if (!transList.count(tranID)) {
        cout << "Transaction " << tranID << " does not exist." << endl;
        return;
    }
    Transaction& t = transList[tranID];

    // check conflict and add edge to graph
    for (const auto& [otherID, otherTran] : transList) {
        if (otherID != tranID && otherTran.write.find(var) != otherTran.write.end()) {
            tranGraph.addDependency(otherID, tranID);
        }
    }

    // write
    t.write[var] = value;
    bool isEven = (var % 2 == 0);
    bool writeSuccess = false;
    for (DataManager* site : sites) {
        if (!site->isAvailable())
            continue;
        if (isEven || site->getSiteID() == 1 + (var % 10)) {
            if (site->hasVariable(var)) {
                writeSuccess = site->write(tranID, var, value);
            }
        }
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
    
    // 1. abort directly
    if (t.status == tranStatus::aborted) {
        abortTransaction(tranID);
        return;
    }

    // 2. If status is ¡°active¡±, check for conflicts
    if (t.status == tranStatus::active) {
        // has cycle
        if (!tranGraph.getCycle().empty()) {
            tran_id target = findTransaction(tranID);

            if (target != -1)
                transList[target].status = tranStatus::aborted;
        }
        commitTransaction(tranID);
    }

}

void TransactionManager::abortTransaction(const tran_id tranID) {
    Transaction& t = transList[tranID];
    for (const auto& [var, _] : t.write) {
        for (DataManager* site : sites) {
            if (site->isAvailable() && site->hasVariable(var))
                site->abortWrite(tranID, var);
        }
    }
    t.status = tranStatus::aborted;
    transList.erase(tranID);
    tranGraph.removeTran(tranID);
    cout << "T" << tranID << " aborts" << endl;
}

void TransactionManager::commitTransaction(const tran_id tranID) {
    Transaction& t = transList[tranID];

    for (const auto& [var, value] : t.write) {
        for (DataManager* site : sites) {
            if (site->isAvailable() && site->hasVariable(var))
                site->commitWrite(tranID, var, value);
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

void TransactionManager::fail(site_id siteID) {
    if (siteID < 1 || siteID > 10) {
        cout << "Invalid site ID: " << siteID << endl;
        return;
    }
    //siteStatus[siteID] = false;
    //cout << "Site " << siteID << " failed." << endl;
}

void TransactionManager::recover(site_id siteID) {
    if (siteID < 1 || siteID > 10) {
        cout << "Invalid site ID: " << siteID << endl;
        return;
    }
    //siteStatus[siteID] = true;
    //cout << "Site " << siteID << " recovered." << endl;
}

void TransactionManager::dump() {
    for (DataManager* site : sites) {
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
        cout << "\b\b \n";
    }
}

void TransactionManager::queryState() {
    cout << "Transaction Manager State:" << endl;
    for (const auto& entry : transList) {
        cout << "Transaction ID: " << entry.first << ", Status: " << entry.second.status << endl;
    }
    cout << "Site Status:" << endl;
    for (const auto& entry : siteStatus) {
        cout << "Site " << entry.first << ": " << (entry.second ? "Up" : "Down") << endl;
    }
}