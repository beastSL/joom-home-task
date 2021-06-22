#include <iostream>
#include <fstream>
#include <random>

using namespace std;

int main() {
    cout << "Hello! Enter number of strings and maximal length of a string.\n";
    uint64_t n;
    uint32_t max_len;
    cin >> n >> max_len;
    ofstream out("data.txt");
    mt19937 rng(random_device{}());
    for (uint64_t i = 0; i < n; i++) {
        uint32_t cur_len = rng() % max_len;
        if (!cur_len) {
            cur_len = max_len;
        }
        for (uint32_t j = 0; j < cur_len; j++) {
            char c = 'a' + (rng() % 26);
            out << c;
        }
        out << '\n';
    }
}