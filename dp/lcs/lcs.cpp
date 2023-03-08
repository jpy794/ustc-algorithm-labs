#include "table.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std;

Table<size_t> lcs_dp(const string &x, const string &y) {
    Table<size_t> t{x.length() + 1, y.length() + 1};
    for (auto i : views::iota(1, static_cast<int>(y.length() + 1))) {
        for (auto j : views::iota(1, static_cast<int>(x.length() + 1))) {
            if (x[i - 1] == y[j - 1]) {
                t.at(j, i) = t.at(j - 1, i - 1) + 1;
            } else {
                t.at(j, i) = max(t.at(j, i - 1), t.at(j - 1, i));
            }
        }
    }
    return t;
}

void print_rectangle(ostream &os, Table<size_t> &t) {
    for (auto i : views::iota(1, static_cast<int>(t.get_height()))) {
        for (auto j : views::iota(1, static_cast<int>(t.get_width()))) {
            os << t.at(j, i) << ' ';
        }
        if (i != t.get_height()) {
            os << '\n';
        }
    }
}

void pretty_print_solution(ostream &os, Table<size_t> &t, const string &x,
                           const string &y) {
    string res; 
    auto i = x.length();
    auto j = y.length();
    while (i != 0 && j != 0) {
        if (x[i - 1] != y[j - 1]) {
            if (t.at(j - 1, i) > t.at(j, i - 1)) {
                j = j - 1;
            } else {
                i = i - 1;
            }
        } else {
            res.push_back(x[i - 1]);
            i = i - 1;
            j = j - 1;
        }
    }
    os << string(res.rbegin(), res.rend());
}

int main() {
    ifstream is{"../input/2_2_input.txt"};
    ofstream os_t("../output/time.txt");
    while (not(is.peek(), is.eof())) {
        int cnt;
        is >> cnt; // ignore length here as we already get the string
        string x, y;
        is >> x >> y;
        typedef chrono::high_resolution_clock Clock;
        auto t1 = Clock::now();
        auto table = lcs_dp(x, y);
        auto t2 = Clock::now();
        os_t << t2 - t1 << '\n';
        print_rectangle(cout, table);
        cout << '\n';
        ofstream os("../output/result_" + to_string(cnt) + ".txt");
        os << table.at(cnt, cnt) << '\n';
        pretty_print_solution(os, table, x, y);
    }
}