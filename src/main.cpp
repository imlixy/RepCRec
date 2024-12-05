#include "common.h"
#include "TM.h"
#include "DM.h"

TransactionManager *tm;

int main(int argc, char **argv) {
    tm = new TransactionManager();


    string line;
    if (argc > 1) {
        ifstream testFile(argv[1]);
        if (!testFile.is_open()) {
            cout << "Failed to open test file" << endl;
            return 1;
        }
        while (getline(testFile, line)) {
            tm->inputHandle(line);
            globalTime += 0.1;
        }
        testFile.close();
    }
    else {
        while (getline(cin, line)) {
            if (line.empty())
                break;
            tm->inputHandle(line);
        }
    }

    return 0;
}