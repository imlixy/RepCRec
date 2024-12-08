/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-02-2024
 * Last Edited:   12-07-2024
 * Description:   This header file defines the DataManager class and its 
 *                associated structures. The DataManager is responsible for 
 *                managing site-level data in a distributed database system. 
 *                Key components include:
 *                 - The site identifier (siteID).
 *                 - The site status, including availability and failure times.
 *                 - A list of stored variables with their version history.
 *                 - A local write cache for uncommitted transaction data.
 ****************************************************************************/

#ifndef DataManager_H
#define DataManager_H
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
        double failTime = -1.0;
        double recoverTime;
    };

    DataManager(site_id id);
    pair<bool, int> read(const tran_id tranID, const var_id variable, const double startTime);
    bool write(tran_id tranID, const var_id var, int value);
    void commitWrite(const tran_id tranID, const var_id var, const int value);
    void abortWrite(const tran_id tranID);
    bool isAvailable() const;
    bool hasVariable(var_id variable) const;
    site_id getSiteID() const;
    unordered_map<var_id, Variable>& getVariables();
    double getFailTime() const;
    void setAvailable(bool flag);
    void clearCache();
    const unordered_map<var_id, int>& getCacheWrite(tran_id tranID) const;
    const std::unordered_map<tran_id, std::unordered_map<var_id, int>>& getAllCacheWrites() const;

private:
    site_id siteID;
    SiteStatus status;
    unordered_map<var_id, Variable> variables;
    unordered_map<tran_id, unordered_map<var_id, int>> cacheWrites;
};

#endif