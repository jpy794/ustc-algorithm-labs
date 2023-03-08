#pragma once

#include "graph.h"
#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <queue>

using namespace std;

class SingleSource {
  protected:
    struct Prev {
        int vex, dist;
    };

  protected:
    std::vector<Prev> prev;
    Graph g;
    int src;

    static constexpr int NO_PREV = -1;

  public:
    static constexpr int UNREACHABLE = std::numeric_limits<int>::max();

  public:
    SingleSource(const Graph &g_, int src_) : g(g_), src(src_) {
        prev.resize(g.v_num());
    }

  protected:
    bool relax(int e_from, int e_to, int e_dist) {
        if (prev[e_from].dist == UNREACHABLE) {
            return false;
        }
        auto new_dist = prev[e_from].dist + e_dist;
        auto &prev_e_to = prev[e_to];
        if (prev_e_to.dist > new_dist) {
            prev_e_to.vex = e_from;
            prev_e_to.dist = new_dist;
            return true;
        }
        return false;
    }

    void init_prev() {
        for (int i = 0; i < g.v_num(); i++) {
            prev[i] = Prev{NO_PREV, UNREACHABLE};
        }
        prev[src] = Prev{NO_PREV, 0};
    }

  public:
    virtual void run() = 0;

    optional<pair<vector<int>, int>> shortest_path(int to) {
        if (prev[to].dist == UNREACHABLE) {
            return nullopt;
        }
        vector<int> path = {to};
        int vex = to;
        while (vex != src) {
            vex = prev[vex].vex;
            path.push_back(vex);
        }
        std::reverse(path.begin(), path.end());
        return {{path, prev[to].dist}};
    }

    optional<int> shortest_dist(int to) {
        if (prev[to].dist == UNREACHABLE) {
            return nullopt;
        }
        return prev[to].dist;
    }

  public:
    auto get_prev() const { return prev; }
};

class BellmanFord : public SingleSource {
  public:
    BellmanFord(const Graph &g_, int src_) : SingleSource(g_, src_) {}

    void run() override {
        init_prev();
        for (int i = 0; i < g.v_num() - 1; i++) {
            for (int e_from = 0; e_from < g.v_num(); e_from++) {
                for (auto &e : g.edges(e_from)) {
                    relax(e_from, e.to, e.weight);
                }
            }
        }
    }

    optional<pair<int, Edge>> get_neg_cycle_edge() {
        int v;
        bool is_neg_edge = false;
        for (int e_from = 0; e_from < g.v_num(); e_from++) {
            for (auto &e : g.edges(e_from)) {
                if (prev[e_from].dist == UNREACHABLE) {
                    continue;
                }
                auto new_dist = prev[e_from].dist + e.weight;
                auto old_dist = prev[e.to].dist;
                if (old_dist > new_dist) {
                    // neg cycle detected
                    prev[e.to].dist = new_dist;
                    prev[e.to].vex = e_from;
                    v = e.to;
                    is_neg_edge = true;
                    break;
                }
            }
        }
        if (!is_neg_edge) {
            return nullopt;
        }
        for (int i = 0; i < g.v_num(); i++) {
            v = prev[v].vex;
            assert(v != NO_PREV);
        }
        // now v is on the neg cycle
        return {{prev[v].vex, Edge{.to = v}}};
    }
};

class Dijkstra : public SingleSource {
  private:
    struct DistVex {
        int dist, vex, prev_vex;
    };
    struct CmpDist {
        bool operator()(const DistVex &lhs, const DistVex &rhs) {
            return lhs.dist > rhs.dist;
        }
    };
    priority_queue<DistVex, vector<DistVex>, CmpDist> q;
    vector<bool> is_poped;

  public:
    Dijkstra(const Graph &g_, int src_) : SingleSource(g_, src_) {}

    void init_queue() {
        for (int i = 0; i < g.v_num(); i++) {
            if (i == src) {
                q.push({0, i, NO_PREV});
            } else {
                q.push({UNREACHABLE, i, NO_PREV});
            }
        }
        is_poped.resize(g.v_num(), false);
    }

    void run() override {
        init_prev();
        init_queue();

        while (!q.empty()) {
            auto dist = q.top().dist;
            auto v = q.top().vex;
            auto prev_v = q.top().prev_vex;

            // v is done
            q.pop();
            if (is_poped[v]) {
                continue;
            }
            is_poped[v] = true;
            prev[v] = {prev_v, dist};

            // relax
            if (dist == UNREACHABLE) {
                continue;
            }
            for (auto &&e : g.edges(v)) {
                q.push({dist + e.weight, e.to, v});
            }
        }
    }
};

class Johnson {
  private:
    Graph g;
    vector<unique_ptr<Dijkstra>> dij_res;
    vector<int> bf_dist;

  public:
    Johnson(const Graph &g_) : g(g_) {}

    void run() {
        auto edges = g.all_edges();
        vector<Edge> connect_vex_edges;
        for (int i = 0; i < edges.size(); i++) {
            connect_vex_edges.push_back({i, 1});
        }
        auto connect_edges = edges;
        connect_edges.push_back(connect_vex_edges);
        Graph connect_g{connect_edges};

        auto bf = BellmanFord(connect_g, connect_g.v_num() - 1);
        bf.run();

        bf_dist.resize(g.v_num());
        for (int i = 0; i < g.v_num(); i++) {
            bf_dist[i] = bf.shortest_dist(i).value();
        }

        for (int i = 0; i < g.v_num(); i++) {
            for (auto &&e : edges[i]) {
                // all vexs shoule be reachable
                e.weight += bf_dist[i] - bf_dist[e.to];
            }
        }

        Graph pos_g{edges};
        dij_res.resize(pos_g.v_num());
        for (int i = 0; i < pos_g.v_num(); i++) {
            dij_res[i].reset(new Dijkstra{pos_g, i});
            dij_res[i]->run();
        }
    }

    optional<pair<vector<int>, int>> shortest_path(int src, int dst) {
        auto op = dij_res[src]->shortest_path(dst);
        if(!op.has_value()) {
            return nullopt;
        }
        auto [path, dist] = op.value();
        dist += bf_dist[dst] - bf_dist[src];
        return {{path, dist}};
    }
};