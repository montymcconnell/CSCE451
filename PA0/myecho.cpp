/****************************************************************
 *
 * Author: Monty McConnell
 * Title: myecho.cpp
 * Date: Wednesday, January 19, 2022
 * Description: Shell echo implemented in C++
 *
 ****************************************************************/

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    for(int i = 1; i < argc; i++) {
        cout << argv[i] << ' ';
    }
    cout << endl;
    return 0;
}