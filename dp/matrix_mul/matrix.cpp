#include <cassert>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <ranges>
#include <vector>
#include <chrono>
#include "table.h"

using namespace std;

pair<Table<long long>, Table<size_t>> mat_mul_dp(const vector<size_t> &mats) {
    auto mat_num = mats.size() - 1;
    Table<long long> table(mat_num, mat_num);
    Table<size_t> choice(mat_num, mat_num);

    // [i, i]
    for (size_t i = 0; i < mat_num; i++) {
        table.at(i, i) = 0;
    }

    // [i, j]
    for (size_t len = 1; len < mat_num; len++) {
        for (size_t i = 0; i < mat_num - len; i++) {
            size_t j = i + len;

            // [i, k], [k + 1, j]
            size_t min_cost = std::numeric_limits<long long>::max();
            size_t min_cost_k;
            for (size_t k = i; k < j; k++) {
                size_t cost = table.at(i, k) + table.at(k + 1, j) +
                              mats[i] * mats[k + 1] * mats[j + 1];
                if (cost < min_cost) {
                    min_cost = cost;
                    min_cost_k = k;
                }
            }
            table.at(i, j) = min_cost;
            choice.at(i, j) = min_cost_k;
        }
    }
    return {table, choice};
}

template<typename T>
void print_triangle(ostream &os, Table<T> &t) {
    for (auto i : views::iota(0, static_cast<int>(t.get_width()))) {
        for (auto j :
                views::iota(i, static_cast<int>(t.get_width())) | views::reverse) {
            os << t.at(i, j) << ' ';
        }
        if (i != t.get_width() - 1) {
            os << '\n';
        }
    }
}

void pretty_print_solution(ostream &os, Table<size_t> &solv, size_t i = 0,
                           size_t j = numeric_limits<size_t>::max()) {
    if (j == numeric_limits<size_t>::max()) {
        j = solv.get_width() - 1;
    }
    if (i == j) {
        os << i;
        return;
    }
    os << '(';
    auto k = solv.at(i, j);
    pretty_print_solution(os, solv, i, k);
    os << ',';
    pretty_print_solution(os, solv, k + 1, j);
    os << ')';
}

int main() {
    ifstream is{"../input/2_1_input.txt"};
    ofstream os("../output/result.txt");
    ofstream os_t("../output/time.txt");
    while (not(is.peek(), is.eof())) {
        int cnt;
        is >> cnt;
        vector<size_t> mats;
        for (auto _ : views::iota(0, cnt + 1)) {
            size_t mat;
            is >> mat;
            mats.push_back(mat);
        }
        typedef chrono::high_resolution_clock Clock;
        auto t1 = Clock::now();
        auto [table, choice] = mat_mul_dp(mats);
        auto t2 = Clock::now();
        os_t << t2 - t1 << '\n';
        print_triangle(cout, table);
        cout << '\n';
        print_triangle(cout, choice);
        cout << '\n';
        os << table.at(0, choice.get_width() - 1) << '\n';
        pretty_print_solution(os, choice);
        os << '\n';
    }
}