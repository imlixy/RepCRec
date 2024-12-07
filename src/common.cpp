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