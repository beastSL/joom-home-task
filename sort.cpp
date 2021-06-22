#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <queue>
#include <tuple>
#include <cstdio>
#include <cctype>

using namespace std;

// assume we have 3GB of RAM available for string storage
constexpr const uint32_t RAM_BYTES = 2.8 * 1024 * 1024 * 1024;
constexpr const uint32_t BLOCK_BYTES = 1024 * 1024;
constexpr const uint32_t BLOCKS_PER_LAYER = RAM_BYTES / BLOCK_BYTES;
uint32_t BLOCKS = 0;

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

    in.seekg(-next_str.size(), ios_base::seekdir::_S_cur);
    return true;
}

void write_buf(const vector<string>& buf, ofstream& out) {
    for (auto &s : buf) {
        out << s << '\n';
    }
}

void split_into_blocks(const string& filename) {
    ifstream in(filename);
    bool flag_cont = true;
    string next_str;
    while (flag_cont) {
        string cur_filename = "b" + to_string(BLOCKS++);
        vector<string> buf;
        ofstream out(cur_filename);

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
    vector<bool> opened(BLOCKS_PER_LAYER, true);
    vector<uint32_t> indexes(BLOCKS_PER_LAYER);  
    for (uint32_t i = 0; i < BLOCKS_PER_LAYER; i++) {
        borders.push_back(l + (r - l) * i / BLOCKS_PER_LAYER);
        if (i > 0) {
            string source = outer_sort(borders[i - 1], borders[i]);
            if (!source.empty()) {
                ifstream in(source);
                sources.push_back(move(in));
                filenames.push_back(source);
            }
        }
    }
    
    vector<vector<string>> bufs(BLOCKS_PER_LAYER);
    priority_queue<pair<string, int>> merge;
    for (uint32_t i = 0; i < BLOCKS_PER_LAYER; i++) {
        opened[i] = read_buf(bufs[i], sources[i]);
        if (opened[i]) {
            merge.push({bufs[i][indexes[i]++], i});
        }
    }

    vector<string> res_buf;
    string out_filename = to_string(l) + "-" + to_string(r);
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
            if (indexes[curr_ind] == bufs[curr_ind].size() && opened[curr_ind]) {
                opened[curr_ind] = read_buf(bufs[curr_ind], sources[curr_ind]);
                indexes[curr_ind] = 0;
            } else if (!opened[curr_ind]) {
                continue;
            }
            merge.push({bufs[curr_ind][indexes[curr_ind]++], curr_ind});
        }
        write_buf(res_buf, out);
        res_buf.clear();
    }

    for (auto &filename : filenames) {
        remove(filename.c_str());
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
}