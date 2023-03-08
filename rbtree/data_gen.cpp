#include "interval_tree.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

random_device rd;
mt19937 g(rd());

auto intervals_gen() {
    uniform_int_distribution<int> start_dist(0, 49);
    vector<Interval<int>> intervals;
    int count = 0;
    while (count < 40) {
        auto start = start_dist(g);
        uniform_int_distribution<int> end_dist(start + 1, 50);
        auto end = end_dist(g);
        auto interval = Interval<int>(start, end);
        /* drop interval that has the same low bound */
        if (find_if(intervals.begin(), intervals.end(),
                    [&interval](auto inter) {
                        return inter.start() == interval.start();
                    }) != intervals.end()) {
            continue;
        }
        /* drop interval that intersect with [26, 29] */
        if (interval.intersect(Interval<int>(26, 29))) {
            continue;
        }
        intervals.push_back(interval);
        count++;
    }
    return intervals;
}

int main() {
    auto intervals = intervals_gen();
    ofstream input("../../input/input.txt", ofstream::out);
    for(auto &&i: intervals) {
        input << i.start() << ' ' << i.end() << '\n';
    }
    input.close();
}