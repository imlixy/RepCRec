#ifndef __TM__
#define __TM__
#include "common.h"
#include "DM.h"
#include "graph.h"

enum tranStatus {
    active,
    committed,
    aborted
};

class TransactionManager {
public:
    struct Transaction {
        tran_id tranID;
        double startTime;
        tranStatus status;
        unordered_set<var_id> read;
        unordered_map<var_id, int> write;
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
    int findTransaction(const tran_id tranID);
};

#endif