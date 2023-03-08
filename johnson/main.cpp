#include "graph.h"
#include "johnson.h"
#include <fstream>
#include <chrono>

using namespace std;

int main() {
    auto time = ofstream("../../output/time.txt", ofstream::out);
    for (int cnt = 0; cnt < 8; cnt++) {
        auto suffix = to_string(cnt / 2 + 1) + to_string(cnt % 2 + 1);
        auto file = "../../input/input" + suffix + ".txt";
        ifstream input(file);
        auto out_file = "../../output/result" + suffix + ".txt";
#ifdef BELLMAN_FORD
        out_file.append(".bf");
#endif
        ofstream output(out_file, ofstream::out);

        vector<vector<Edge>> edges;
        // assume edge_from is in ascending order
        int e_from, e_to, e_dist;
        while (input.peek(), !input.eof()) {
            input >> e_from >> e_to >> e_dist;
            // remove \n
            input.get();
            if (edges.size() <= e_from) {
                edges.push_back({});
            }
            edges[e_from].push_back(Edge{e_to, e_dist});
        }

        Graph g{edges};
        Johnson john{g};
        
        typedef chrono::high_resolution_clock Clock;
        auto t1 = Clock::now();
        john.run();
        auto t2 = Clock::now();
        time << t2 - t1 << '\n';

        for (int src = 0; src < g.v_num(); src++) {
#ifdef BELLMAN_FORD
            BellmanFord bf{g, src};
            bf.run();
#endif
            for (int dst = 0; dst < g.v_num(); dst++) {
                if (dst == src) {
                    continue;
                }
                auto p = john.shortest_path(src, dst);
#ifdef BELLMAN_FORD
                p = bf.shortest_path(dst);
#endif
                if (p.has_value()) {
                    auto [path, dist] = p.value();
                    output << '(';
                    for (auto v : path) {
                        output << v;
                        if (v != path.back()) {
                            output << ',';
                        }
                    }
                    output << ' ' << dist << ")\n";
                } else {
                    output << '(' << src << ',' << dst << " unreachable)\n";
                }
            }
        }
    }
}