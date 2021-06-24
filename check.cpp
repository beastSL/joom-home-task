#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    ifstream fin("log.txt");
    string filename;
    while (fin >> filename) {
        ifstream in(filename);
        string s;
        string last;
        while (in >> s) {
            if (s < last) {
                cout << "Error in file " << filename << ", wrong string: " << s << '\n';
                return 0;
            }
            last = s;
        }
    }
    cout << "Sort is succesful.\n";
}