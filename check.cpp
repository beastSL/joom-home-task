#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    ifstream in("result.txt");
    string s;
    string last;
    while (in >> s) {
        if (s < last) {
            cout << "Error, wrong string: " << s << '\n';
            return 0;
        }
        last = s;
    }
    cout << "Sort is succesful.\n";
}