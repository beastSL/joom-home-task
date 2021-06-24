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
#include <ctime>
#include <memory>

using namespace std;

// assume we have 4MB of RAM available for string storage
constexpr const uint32_t RAM_BYTES = 4 * 1024 * 1024 * 0.95;
//must be more than two
constexpr const uint32_t BLOCKS_PER_LAYER = 2;
constexpr const uint32_t BLOCK_BYTES = RAM_BYTES / BLOCKS_PER_LAYER;
uint32_t BLOCKS = 0;
// ofstream log("log.txt");

class my_ifstream {
private:
    char buf[BLOCK_BYTES];
public:
    ifstream stream;
    my_ifstream() {}
    my_ifstream(const string& filename) {
        stream = ifstream(filename);
        stream.rdbuf()->pubsetbuf(buf, sizeof(buf));
    }
    my_ifstream(const string& filename, ios::openmode om) {
        stream = ifstream(filename, om);
        stream.rdbuf()->pubsetbuf(buf, sizeof(buf));
    }
};

class my_ofstream {
private:
    char buf[BLOCK_BYTES];
public:
    ofstream stream;
    my_ofstream() {}
    my_ofstream(const string& filename) {
        stream = ofstream(filename);
        stream.rdbuf()->pubsetbuf(buf, sizeof(buf));
    }
    my_ofstream(const string& filename, const ios::openmode om) {
        stream = ofstream(filename, om);
        stream.rdbuf()->pubsetbuf(buf, sizeof(buf));
    }
};

bool check_correct(const string& filename) {
    my_ifstream in(filename);
    return in.stream && in.stream.peek() != ifstream::traits_type::eof(); 
}

bool read_buf(vector<string>& buf, my_ifstream& in) {
    uint32_t cur_bytes = 0;
    string next_str;
    while (cur_bytes <= BLOCK_BYTES) {
        if (!next_str.empty()) {
            buf.push_back(next_str);
        }
        while (isspace(in.stream.peek())) {
            in.stream.get();
        }
        if (in.stream.peek() == EOF) {
            return false;
        }
        in.stream >> next_str;
        cur_bytes += sizeof(next_str) + next_str.size();
    }

    in.stream.seekg(-(off_t)next_str.size(), ios::cur);
    return true;
}

void write_buf(const vector<string>& buf, my_ofstream& out) {
    for (auto &s : buf) {
        out.stream << s << '\n';
    }
}

void split_into_blocks(const string& filename) {
    my_ifstream in(filename, ios::binary);
    bool flag_cont = true;
    while (flag_cont) {
        string cur_filename = "b" + to_string(BLOCKS++);
        vector<string> buf;
        my_ofstream out(cur_filename);
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
    vector<my_ifstream> sources;
    vector<string> filenames;
    for (uint32_t i = 0; i <= BLOCKS_PER_LAYER; i++) {
        borders.push_back(l + (r - l) * i / BLOCKS_PER_LAYER);
        if (i > 0) {
            string source = outer_sort(borders[i - 1], borders[i]);
            if (!source.empty()) {
                my_ifstream in(source, ios::binary);
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
    my_ofstream out(out_filename);
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
                    bufs[curr_ind].clear();
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
        sources[i].stream.close();
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
    cout << "Parameters:\nRAM_BYTES = " << RAM_BYTES << ", BLOCK_BYTES = " << BLOCK_BYTES ;
    cout << ", BLOCKS_PER_LAYER = " << BLOCKS_PER_LAYER << endl;
    double start = clock();
    if (!check_correct(filename)){
        cout << "Done.\n";
        return 0;
    }
    split_into_blocks(filename);
    rename(outer_sort(0, BLOCKS).c_str(), "result.txt");
    cout << "Done.\n";
    cout << "On BPL=" << BLOCKS_PER_LAYER << " time is " << (clock() - start) / CLOCKS_PER_SEC; 
    cout << '\n';
}