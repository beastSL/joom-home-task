#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <queue>
#include <tuple>
#include <cstdio>
#include <cctype>
#include <functional>
#include <cerrno>
#include <cstring>

using namespace std;

// assume we have 4MB of RAM available for string storage
constexpr const uint32_t RAM_BYTES = 4 * 1024 * 1024 * 0.95;
constexpr const uint32_t BLOCK_BYTES = 8 * 1024;
constexpr const uint32_t BLOCKS_PER_LAYER = RAM_BYTES / BLOCK_BYTES;
uint32_t BLOCKS = 0;
// ofstream log("log.txt");

bool check_correct(const string& filename) {
    ifstream in(filename);
    return in && in.peek() != ifstream::traits_type::eof(); 
}

bool read_buf(vector<string>& buf, ifstream& in) {
    uint32_t cur_bytes = 0;
    string next_str;
    while (cur_bytes <= BLOCK_BYTES) {
        if (!next_str.empty()) {
            buf.push_back(next_str);
        }
        while (isspace(in.peek())) {
            in.get();
        }
        if (in.peek() == EOF) {
            return false;
        }
        in >> next_str;
        cur_bytes += sizeof(next_str) + next_str.size();
    }

    in.seekg(-(off_t)next_str.size(), ios::cur);
    return true;
}

void write_buf(const vector<string>& buf, ofstream& out) {
    for (auto &s : buf) {
        out << s << '\n';
    }
}

void split_into_blocks(const string& filename) {
    ifstream in(filename, ios::binary);
    bool flag_cont = true;
    while (flag_cont) {
        string cur_filename = "b" + to_string(BLOCKS++);
        vector<string> buf;
        ofstream out(cur_filename);
        // log << cur_filename << '\n';

        string str;
        flag_cont = read_buf(buf, in);
        sort(buf.begin(), buf.end());
        write_buf(buf, out);
    }
}

string outer_sort(uint64_t l, uint64_t r) {
    if (l == r) {
        return "";
    }
    if (l == r - 1) {
        return "b" + to_string(l);
    }
    vector<uint32_t> borders;
    vector<ifstream> sources;
    vector<string> filenames;
    for (uint32_t i = 0; i <= BLOCKS_PER_LAYER; i++) {
        borders.push_back(l + (r - l) * i / BLOCKS_PER_LAYER);
        if (i > 0) {
            string source = outer_sort(borders[i - 1], borders[i]);
            if (!source.empty()) {
                ifstream in(source, ios::binary);
                sources.push_back(move(in));
                filenames.push_back(source);
            }
        }
    }

    uint32_t real_bpl = sources.size();
    
    vector<bool> opened(real_bpl, true);
    vector<vector<string>> bufs(real_bpl);
    vector<uint32_t> indexes(real_bpl);
    priority_queue<pair<string, int>, vector<pair<string, int>>, greater<pair<string, int>>> merge;
    for (uint32_t i = 0; i < real_bpl; i++) {
        opened[i] = read_buf(bufs[i], sources[i]);
        merge.push({bufs[i][indexes[i]++], i});
    }

    vector<string> res_buf;
    string out_filename = to_string(l) + "-" + to_string(r);
    // log << out_filename << '\n';
    ofstream out(out_filename);
    while (!merge.empty()) {
        uint32_t bytes = 0;
        while (!merge.empty() && 
               bytes + sizeof(merge.top().first) + merge.top().first.size() <= BLOCK_BYTES) {
            string curr_s;
            uint32_t curr_ind;
            tie(curr_s, curr_ind) = merge.top();
            merge.pop();
            res_buf.push_back(curr_s);
            bytes += sizeof(merge.top().first) + merge.top().first.size();
            if (indexes[curr_ind] == bufs[curr_ind].size()) {
                if (opened[curr_ind]) {
                    opened[curr_ind] = read_buf(bufs[curr_ind], sources[curr_ind]);
                    indexes[curr_ind] = 0;
                } else {
                    continue;
                }
            }
            merge.push({bufs[curr_ind][indexes[curr_ind]++], curr_ind});
        }
        write_buf(res_buf, out);
        res_buf.clear();
    }

    for (int i = 0; i < real_bpl; i++) {
        sources[i].close();
        int result = remove(filenames[i].c_str());
        if (result) {
            cout << strerror(errno);
            exit(0);
        }
    }
    return out_filename;
}

int main() {
    cout << "Hello! Enter the filename to sort.\n";
    string filename;
    cin >> filename;
    if (!check_correct(filename)){
        cout << "Done.\n";
        return 0;
    }
    split_into_blocks(filename);
    rename(outer_sort(0, BLOCKS).c_str(), "result.txt");
    cout << "Done.\n";
}