/*****************************************************************************
 * Author:        Xinyu Li
 * Created:       12-02-2024
 * Last Edited:   12-07-2024
 * Description:   The main entry point of the project.
 *                It supports input from either a .txt file or command line.
 *                The program reads the input instructions and invokes the 
 *                TransactionManager functions to parse and execute them iteratively.
 * 
 * Inputs:        Input through a .txt file under "/test" or via command line
 * 
 * Outputs:       0 (successful execution)
 * 
 * Side Effects:  Input must strictly follow the specified requirements.
 *                The program can only handle "//..." style comments
 ****************************************************************************/


#include "common.h"
#include "TransactionManager.h"
#include "DataManager.h"

TransactionManager *tm;

int main(int argc, char **argv) {
    tm = new TransactionManager();


    string line;
    if (argc > 1) {
        string base = "../test/";
        string path = base + argv[1];
        ifstream testFile(path);
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
        cout << "Input Command: " << endl;
        while (getline(cin, line)) {
            if (line.empty())
                break;
            tm->inputHandle(line);
            globalTime += 0.1;
        }
    }

    return 0;
}