/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-02-2024
 * Last Edited:   12-08-2024
 * Description:   This header file defines the TransactionManager class, which 
 *                acts as the master node in a distributed database system. 
 *                The TransactionManager is responsible for managing all 
 *                transaction-related information, site status, and the 
 *                serialization graph used for conflict detection. 
 * 
 *                Key responsibilities include:
 *                 - Handling transaction lifecycle (begin, read, write, end).
 *                 - Managing site failures and recoveries.
 *                 - Interfacing with the DataManager for site-level data 
 *                   operations and the SerializationGraph for dependency tracking.
 *                 - Maintaining a list of active transactions and their states.
 ****************************************************************************/

#ifndef TransactionManager_H
#define TransactionManager_H
#include "common.h"
#include "DataManager.h"
#include "graph.h"

enum TranStatus {
    active,
    committed,
    aborted,
    blocked
};

class TransactionManager {
public:
    struct Transaction {
        tran_id tranID;
        double startTime;
        TranStatus status;
        unordered_set<var_id> read;
        unordered_map<var_id, pair<int, double>> write;
    };

    TransactionManager();
    void inputHandle(const string &inputs);
    void beginTransaction(tran_id tranID);
    void readTransaction(const tran_id tranID, const var_id variable);
    void writeTransaction(const tran_id tranID, const var_id variable, const int value);
    void endTransaction(tran_id tranID);
    void fail(site_id siteID);
    void recover(site_id siteID);
    void dump();
    void queryState();

private:
    SerializationGraph tranGraph;
    unordered_map<tran_id, Transaction> transList;
    vector<DataManager*> sites;

    void abortTransaction(const tran_id tranID);
    void commitTransaction(const tran_id tranID);
    vector<tran_id> getWAWConflict(const tran_id tranID);
};

#endif