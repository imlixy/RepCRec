#ifndef DM_H
#define DM_H
#include "common.h"

class DataManager {
public:
    struct Variable {
        int value;                          // Committed value
        double lastCommitTime;                  // Last commit timestamp
        map<double, int> versionHistory; // Historical versions (timestamp -> value)
    };
    struct SiteStatus {
        bool available;
        double failTime;
        double recoverTime;
    };

    DataManager(site_id id);
    int read(const tran_id tranID, const var_id variable, const double startTime);
    bool write(tran_id tranID, const var_id var, int value);
    void commitWrite(const tran_id tranID, const var_id var, const int value);
    void abortWrite(const tran_id tranID, const var_id var);
    bool isAvailable() const;
    bool hasVariable(var_id variable) const;
    site_id getSiteID() const;
    const unordered_map<var_id, Variable>& getVariables() const;
private:
    site_id siteID;
    SiteStatus status;
    unordered_map<var_id, Variable> variables;
    unordered_map<tran_id, unordered_map<var_id, int>> cacheWrites;
};

#endif