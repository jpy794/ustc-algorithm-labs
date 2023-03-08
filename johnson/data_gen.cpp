#include <algorithm>
#include <fstream>
#include <random>
#include <vector>
#include <iostream>

#include "graph.h"
#include "johnson.h"

using namespace std;

random_device rd;
mt19937 g(rd());

struct VertexGenSet {
    int v_num;
    VertexGenSet(size_t v_num_) : v_num(v_num_) {}
    vector<pair<int, int>> gen_set() {
        return {{v_num, log2(v_num) / log2(5)}, {v_num, log2(v_num) / log2(7)}};
    }
};

int gen_weight() {
    std::uniform_int_distribution<int> w_gen{-10, 50};
    return w_gen(g);
}

vector<Edge> gen_edges_from(int from, int to_max, int count) {
    vector<Edge> res;
    uniform_int_distribution<int> to_dist{0, to_max};
    for (int i = 0; i < count; i++) {
        auto edge = Edge{to_dist(g), gen_weight()};
        while (edge.to == from || find(res.begin(), res.end(), edge) != res.end()) {
            edge = Edge{to_dist(g), gen_weight()};
        }
        res.push_back(edge);
    }
    return res;
}

int main() {
    vector<pair<int, int>> gen_set;
    for (auto i : vector<VertexGenSet>{27, 81, 243, 729}) {
        auto current_set = i.gen_set();
        gen_set.insert(gen_set.end(), current_set.begin(), current_set.end());
    }

    int cnt = 0;
    for (auto [v_num, e_num] : gen_set) {
        auto suffix = to_string(cnt / 2 + 1) + to_string(cnt % 2 + 1);
        auto file = "../../input/input" + suffix + ".txt";
        ofstream input(file, ofstream::out);
        
        vector<vector<Edge>> edges;
        for(int i = 0; i < v_num; i++) {
            auto gen_edges = gen_edges_from(i, v_num - 1, e_num);
            edges.emplace_back(move(gen_edges));
        }

        cout << "original\n" << Graph{edges};
        // remove neg cycles
        while (true) {
            auto connect_edges = edges;
            vector<Edge> connect_vex_edges;
            for(int i = 0; i < edges.size(); i++) {
                connect_vex_edges.push_back({i, 1});
            }
            connect_edges.push_back(connect_vex_edges);
            Graph connect_g{connect_edges};
            BellmanFord bf{connect_g, connect_g.v_num() - 1};
            bf.run();
            auto neg_edge = bf.get_neg_cycle_edge();
            if(!neg_edge.has_value()) {
                break;
            }
            auto [v, e] = neg_edge.value();
            cout << "remove edge " << v << ' ' << e.to << '\n';
            vector<int> a;
            erase(edges[v], e);
        }
        Graph graph{edges};

        input << graph;
        input.close();
        cnt++;
    }
}