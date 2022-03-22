/****************************************************************
 *
 * Author: Monty McConnell
 * Title: mycat.cpp
 * Date: Wednesday, January 19, 2022
 * Description: Opens, reads, and prints file to terminal
 *
 ****************************************************************/

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char** argv) {
    string line;
    for(int i = 1; i < argc; i++) {
        ifstream file;
        file.open(argv[i]);
        while(getline(file,line)) {
            cout << line << endl;
        }
        file.close();
        
    }

    return 0;
}