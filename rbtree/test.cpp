#include "interval_tree.h"
#include "rbtree.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <ostream>
#include <random>

using namespace std;

void rbtree_test() {
    vector<int> ins;
    for (auto i = 0; i < 20; i++) {
        ins.push_back(i);
    }
    random_device rd;
    mt19937 g(rd());
    RbTree<int, int> t;

    // test insert
    shuffle(ins.begin(), ins.end(), g);
    for (auto i : ins) {
        cout << "inserting (" << i << ", " << i << ")\n";
        t.insert({i, i});
        t.verify();
    }
    cout << t;

    // test search
    shuffle(ins.begin(), ins.end(), g);
    for (auto i : ins) {
        cout << "searching " << i << "\n";
        auto res = t.search(i);
        assert(res);
    }

    // search elem not in rbtree
    cout << "searching " << 100 << "\n";
    assert(t.search(100) == nullptr);
    cout << "searching " << 666 << "\n";
    assert(t.search(666) == nullptr);

    // test remove
    shuffle(ins.begin(), ins.end(), g);
    for (auto &&i : ins) {
        cout << "removing " << i << "\n";
        t.remove(i);
        t.verify();
    }
    cout << t;
}

void interval_tree_test() {
    random_device rd;
    mt19937 g(rd());
    uniform_int_distribution<int> dist(0, 30);
    vector<Interval<int>> intervals;
    for (auto i = 0; i < 20; i++) {
        auto start = dist(g);
        uniform_int_distribution<int> end_dist(start, 30);
        auto end = end_dist(g);
        auto interval = Interval<int>(start, end);
        if (find_if(intervals.begin(), intervals.end(),
                    [&interval](auto inter) {
                        return inter.start() == interval.start();
                    }) == intervals.end()) {
            intervals.push_back(interval);
        }
    }

    IntervalTree<int> t;

    for (auto &&i : intervals) {
        cout << "inserting " <<  i << '\n';
        t.insert(i);
        t.verify();
    }
    cout << t;

    // test remove
    shuffle(intervals.begin(), intervals.end(), g);
    for (auto &&i : intervals) {
        cout << "removing " << i << '\n';
        t.remove(i);
        t.verify();
    }
    cout << t;
}

int main() {
    rbtree_test();
    cout << "rbtree test passed\n";
    interval_tree_test();
    cout << "interval tree test passed\n";
    cout.flush();
}