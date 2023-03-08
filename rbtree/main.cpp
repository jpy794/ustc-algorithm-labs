#include "interval_tree.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

random_device rd;
mt19937 g(rd());

int main() {
    /* read input */
    vector<Interval<int>> ints;
    ifstream input("../../input/input.txt");
    while (true) {
        int start, end;
        input >> start >> end;
        if (input.fail()) {
            break;
        }
        ints.push_back({start, end});
    }
    input.close();

    IntervalTree<int> t;
    for (auto &&i : ints) {
        t.insert(i);
        t.verify();
    }

    /* in order traverse */
    ofstream inorder("../../output/inorder.txt");
    t.in_order_traverse([&inorder](auto v) {
        inorder << v.interval.start() << ' ' << v.interval.end() << ' ' << v.max
                << '\n';
    });
    inorder.close();

    ofstream remove("../../output/delete_data.txt");
    vector<Interval<int>> remove_ints;
    for (int i = 0; i < 3;) {
        assert(ints.size() > 0);
        uniform_int_distribution<int> dist(0, ints.size() - 1);
        auto idx = dist(g);
        auto interval = ints[idx];
        if (find(remove_ints.begin(), remove_ints.end(), interval) !=
            remove_ints.end()) {
            continue;
        }
        remove_ints.push_back(interval);
        i++;
    }
    for (auto &&i : remove_ints) {
        t.remove(i);
        t.verify();
        remove << i.start() << ' ' << i.end() << '\n';
        t.in_order_traverse([&remove](auto v) {
            remove << v.interval.start() << ' ' << v.interval.end() << ' '
                   << v.max << '\n';
        });
        remove << '\n';
    }
    remove.close();

    ofstream search("../../output/search.txt");
    vector<Interval<int>> search_ints;
    uniform_int_distribution<int> dist(26, 29);
    auto start = dist(g);
    uniform_int_distribution<int> end_dist(start + 1, 30);
    auto end = end_dist(g);
    search_ints.push_back(Interval<int>(start, end));
    for (int i = 0; i < 2;) {
        uniform_int_distribution<int> dist(0, 49);
        auto start = dist(g);
        uniform_int_distribution<int> end_dist(start + 1, 50);
        auto end = end_dist(g);
        auto interval = Interval<int>(start, end);
        if (find(search_ints.begin(), search_ints.end(), interval) !=
            search_ints.end()) {
            continue;
        }
        search_ints.push_back(interval);
        i++;
    }
    for (auto &&i : search_ints) {
        auto res = t.search(i);
        search << i.start() << ' ' << i.end() << '\n';
        if (res.has_value()) {
            search << res->start() << ' ' << res->end() << '\n';
        } else {
            search << "Null\n";
        }
        search << '\n';
    }
    search.close();

    cout.flush();
}