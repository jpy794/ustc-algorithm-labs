#pragma once

#include "rbtree.h"
#include <functional>
#include <optional>
#include <ostream>

template <typename K> class Interval {
  private:
    K m_start, m_end;

  public:
    Interval(const K &start, const K &end) {
        assert(start <= end);
        m_start = start;
        m_end = end;
    }

    bool intersect(const Interval &other) const {
        return !(m_end < other.m_start || m_start > other.m_end);
    }

    const K &start() const { return m_start; }
    const K &end() const { return m_end; }

    bool operator==(const Interval &other) const {
        return m_start == other.m_start && m_end == other.m_end;
    }

    friend ostream &operator<<(ostream &os, const Interval &interval) {
        os << '[' << interval.m_start << ", " << interval.m_end << ']';
        return os;
    }
};

template <typename K> struct Value {
    Interval<K> interval;
    K max;
};

template <typename K> class IntervalTree : protected RbTree<K, Value<K>> {
    using RbTreeK = RbTree<K, Value<K>>;
    using IntervalK = Interval<K>;
    using ValueK = Value<K>;

    inline void update_max(typename RbTreeK::NodePtr node) {
        auto end = [](typename RbTreeK::NodePtr node) -> const K & {
            return node->value().interval.end();
        };
        auto max = [](typename RbTreeK::NodePtr node) -> K & {
            return node->kv->second.max;
        };

        max(node) = end(node);
        if (!RbTreeK::is_nil(node->lchild)) {
            max(node) = std::max(max(node), max(node->lchild));
        }
        if (!RbTreeK::is_nil(node->rchild)) {
            max(node) = std::max(max(node), max(node->rchild));
        }
    }

    virtual void post_left_rotate(typename RbTreeK::NodePtr node) final {
        update_max(node);
        /* actually we can ignore checking if parent's lchild is nil as it's
         * node */
        update_max(node->parent_shr());
    }

    virtual void post_right_rotate(typename RbTreeK::NodePtr node) final {
        update_max(node);
        update_max(node->parent_shr());
    }

  public:
    /* insertion of interval with the same start is an undefined behaivor */
    void insert(const IntervalK &interval) {
        auto key = interval.start();
        auto max = interval.end();

        auto fix_node = RbTreeK::insert_node({key, ValueK{interval, max}});

        /* bottom up fix */
        auto node = fix_node->parent_shr();
        while (!RbTreeK::is_nil(node)) {
            /* insert will only increse max */
            if (max > node->value().max) {
                node->kv->second.max = max;
            }
            node = node->parent_shr();
        }

        RbTreeK::insert_fixup(fix_node);
    }

    void remove(const IntervalK &interval) {
        auto [_, node] = RbTreeK::search_node(interval.start());
        if (!RbTreeK::is_nil(node)) {
            if (node->value().interval == interval) {
                auto [is_lost_black, fix_node] = RbTreeK::remove_node(node);

                /* bottom up update max */
                auto node = fix_node->parent_shr();
                /* node could now be root (nil at the same time)*/
                while (!RbTreeK::is_nil(node)) {
                    update_max(node);
                    node = node->parent_shr();
                }

                if (is_lost_black) {
                    RbTreeK::remove_fixup(fix_node);
                }
            }
        }
    }

    optional<IntervalK> search(const IntervalK &interval) {
        auto current = RbTreeK::root;
        while (!RbTreeK::is_nil(current) &&
               !current->value().interval.intersect(interval)) {
            if (!RbTreeK::is_nil(current->lchild) &&
                current->lchild->value().max >= interval.start()) {
                current = current->lchild;
            } else {
                current = current->rchild;
            }
        }
        if (RbTreeK::is_nil(current)) {
            return nullopt;
        } else {
            return {current->kv->second.interval};
        }
    }

    friend ostream &operator<<(ostream &os, IntervalTree &t) {
        t.pre_order_depth(
            [&os](size_t depth, typename RbTreeK::NodePtr node) {
                os << string(depth, '\t') << node->value().interval << ' '
                   << node->value().max << ' ';
                if (node->red()) {
                    os << 'r';
                } else {
                    os << 'b';
                }
                os << '\n';
            },
            0, t.root);
        return os;
    }

    void in_order_traverse(function<void(ValueK)> f) {
        in_order_traverse_node(RbTreeK::root, f);
    }

    void verify() {
        RbTreeK::verify();
        verify_max(RbTreeK::root);
    }

  private:
    void verify_max(typename RbTreeK::NodePtr node) {
        if (!RbTreeK::is_nil(node)) {
            auto max = node->value().interval.end();
            if (!RbTreeK::is_nil(node->lchild)) {
                max = std::max(max, node->lchild->value().max);
            }
            if (!RbTreeK::is_nil(node->rchild)) {
                max = std::max(max, node->rchild->value().max);
            }
            assert(max == node->value().max);
            verify_max(node->lchild);
            verify_max(node->rchild);
        }
    }

    void in_order_traverse_node(typename RbTreeK::NodePtr node,
                                function<void(ValueK)> f) {
        if (!RbTreeK::is_nil(node)) {
            in_order_traverse_node(node->lchild, f);
            f(node->value());
            in_order_traverse_node(node->rchild, f);
        }
    }
};