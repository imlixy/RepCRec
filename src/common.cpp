/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-05-2024
 * Last Edited:   12-06-2024
 * Description:   [Brief description of the functionality implemented in this file.
 *                 For example, "This file implements the main logic for processing
 *                 user input and managing system states."]
 * Inputs:        [List all external inputs that the functions in this file may take.
 *                 For example, "User inputs through console" or "Data from a file".]
 * Outputs:       [List all external outputs produced by the functions in this file.
 *                 For example, "Console messages" or "Generated reports".]
 * Side Effects:  [Mention any side effects of the functions, if applicable.
 *                 For example, "Modifies global state" or "Writes to disk".]
 ****************************************************************************/

#include "common.h"

double globalTime = 0.0;

double currentTime() {
    //return static_cast<double>(time(nullptr));
    return globalTime;
}

vector<string> split(const string& line, char delimiter, int start) {
    string str = regex_replace(line, regex("//.*"), "");    // remove the comment
    str = regex_replace(str, regex("^\\s+|\\s+$"), "");     // remove space before and after
    str = str.substr((1 + start), str.size() - start - 2);
    vector<string> tokens;
    string token;
    for (char ch : str) {
        if (ch == delimiter || ch == ' ') {
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