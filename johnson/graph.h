#pragma once

#include <functional>
#include <ostream>
#include <vector>

struct Edge {
    int to, weight;

    bool operator==(const Edge &rhs) { return to == rhs.to; }
};

class Graph {
  private:
    const std::vector<std::vector<Edge>> &m_edges;
    int m_v_num, m_e_num;

  public:
    Graph(const std::vector<std::vector<Edge>> &edges) : m_edges(edges) {
        m_v_num = m_edges.size();
        m_e_num = 0;
        for (auto &e : m_edges) {
            m_e_num += e.size();
        }
    }
    const std::vector<Edge> &edges(int vex) const { return m_edges[vex]; }
    decltype(m_v_num) v_num() const { return m_v_num; }
    decltype(m_e_num) e_num() const { return m_e_num; }
    auto all_edges() const { return m_edges; }

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph) {
        for (int i = 0; i < graph.v_num(); i++) {
            for (auto &e : graph.edges(i)) {
                os << i << ' ' << e.to << ' ' << e.weight << '\n';
            }
        }
        return os;
    }
};

std::ostream &operator<<(std::ostream &os, const Graph &graph);