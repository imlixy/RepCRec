/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-04-2024
 * Last Edited:   12-05-2024
 * Description:   [Brief description of the functionality defined in this file.
 *                 For example, "This file declares the utility functions and
 *                 data structures used for processing transactions."]
 ****************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <stack>
#include <algorithm>
#include <regex>

using namespace std;

#define SITE_NUM 10
#define VAR_NUM 20
typedef int var_id;
typedef int site_id;
typedef int tran_id;

extern double globalTime;

enum Status {
    commit,
    running
};

double currentTime();
vector<string> split(const string& line, char delimiter, int start);

#endif