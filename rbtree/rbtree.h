#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <ostream>

using namespace std;

template <typename K, typename V> class RbTree {
  protected:
    using KvPtr = shared_ptr<pair<K, V>>;
    struct Node {
        KvPtr kv;
        weak_ptr<Node> parent;
        shared_ptr<Node> lchild, rchild;
        bool is_black;

        const K &key() const { return kv->first; }
        const V &value() const { return kv->second; }
        bool red() const { return !is_black; }
        bool black() const { return is_black; }
        shared_ptr<Node> parent_shr() const { return parent.lock(); }
    };
    using NodePtr = shared_ptr<Node>;

    /* nil->parent is invalid, but may be changed */
    NodePtr nil{new Node{.is_black = true}};
    NodePtr root{nil};

  protected:
    NodePtr new_node(NodePtr parent, bool is_black, const pair<K, V> &kv) {
        auto node = NodePtr{new Node};
        node->kv.reset(new pair<K, V>{kv});
        node->parent = parent;
        node->lchild = nil;
        node->rchild = nil;
        node->is_black = is_black;
        return node;
    }
    bool is_nil(const NodePtr node) const { return node == nil; }
    bool is_root(const NodePtr node) const { return node == root; }
    void pre_order_depth(function<void(size_t, NodePtr)> f, size_t depth,
                         NodePtr node) {
        if (!is_nil(node)) {
            f(depth, node);
            pre_order_depth(f, depth + 1, node->lchild);
            pre_order_depth(f, depth + 1, node->rchild);
        }
    }

  protected:
    virtual void post_left_rotate(NodePtr node) {}
    virtual void post_right_rotate(NodePtr node) {}

  private:
    void left_rotate(NodePtr node) {
        assert(node->rchild);
        auto rchild = node->rchild;
        /* move the grand child */
        auto moved_child = rchild->lchild;
        node->rchild = moved_child;
        if (!is_nil(moved_child)) {
            moved_child->parent = node;
        }
        /* move node and rchild */
        transplant(node, rchild);
        node->parent = rchild;
        rchild->lchild = node;
        post_left_rotate(node);
    }

    void right_rotate(NodePtr node) {
        assert(node->lchild);
        auto lchild = node->lchild;
        /* connect grandchild to node */
        auto moved_child = lchild->rchild;
        node->lchild = moved_child;
        if (!is_nil(moved_child)) {
            moved_child->parent = node;
        }
        /* connect lchild to grandparent */
        transplant(node, lchild);
        /* connect node to lchild */
        node->parent = lchild;
        lchild->rchild = node;
        post_right_rotate(node);
    }

  protected:
    void insert_fixup(NodePtr node) {
        auto parent = node->parent_shr();
        while (parent->red()) {
            /* root does not have grand, but its parent is black */
            auto grand = parent->parent_shr();
            if (parent == grand->lchild) {
                auto uncle = grand->rchild;
                if (uncle->red()) {
                    /* both parent and uncle are red */
                    uncle->is_black = true;
                    parent->is_black = true;
                    grand->is_black = false;
                    node = grand;
                } else /* uncle->black() */ {
                    if (node == parent->rchild) {
                        /* turn parent into lchild of node */
                        left_rotate(parent);
                        /* rotate should not invalidate NodePtr */
                        parent.swap(node);
                    }
                    /* grand must be black, as parent is red */
                    grand->is_black = false;
                    parent->is_black = true;
                    right_rotate(grand);
                    /* loop break */
                }
            } else /*parent == grand->rchild */ {
                auto uncle = grand->lchild;
                if (uncle->red()) {
                    /* both parent and uncle are red */
                    uncle->is_black = true;
                    parent->is_black = true;
                    grand->is_black = false;
                    node = grand;
                } else /* uncle->black() */ {
                    if (node == parent->lchild) {
                        /* turn parent into lchild of node */
                        right_rotate(parent);
                        /* rotate should not invalidate NodePtr */
                        parent.swap(node);
                    }
                    /* grand must be black, as parent is red */
                    grand->is_black = false;
                    parent->is_black = true;
                    left_rotate(grand);
                }
            }
            /* update parent based on node */
            parent = node->parent_shr();
        }
        root->is_black = true;
    }

    void remove_fixup(NodePtr node) {
        while (!is_root(node) && node->black()) {
            auto parent = node->parent_shr();
            if (node == parent->lchild) {
                auto sibling = parent->rchild;
                if (sibling->red()) {
                    /* parent and children of sibling must be black */
                    left_rotate(parent);
                    parent->is_black = false;
                    sibling->is_black = true;
                    /* now parent must have a black rchild, which is node's new
                     * sibling */
                    sibling = parent->rchild;
                }
                if (sibling->black()) {
                    if (sibling->lchild->black() && sibling->rchild->black()) {
                        sibling->is_black = false;
                        node = parent;
                    } else {
                        if (sibling->rchild->black()) /* lchild->red() */ {
                            /* try make rchild red */
                            sibling->lchild->is_black = true;
                            sibling->is_black = false;
                            right_rotate(sibling);
                            sibling = parent->rchild;
                        }
                        /* now rchild of sibling must be red */
                        sibling->is_black = parent->is_black;
                        parent->is_black = true;
                        sibling->rchild->is_black = true;
                        left_rotate(parent);
                        /* node is black, it's ok to just break */
                        break;
                    }
                }
            } else /* node == parent->rchild */ {
                auto sibling = parent->lchild;
                if (sibling->red()) {
                    /* parent and children of sibling must be black */
                    right_rotate(parent);
                    parent->is_black = false;
                    sibling->is_black = true;
                    /* now parent must have a black lchild, which is node's new
                     * sibling */
                    sibling = parent->lchild;
                }
                if (sibling->black()) {
                    if (sibling->lchild->black() && sibling->rchild->black()) {
                        sibling->is_black = false;
                        node = parent;
                    } else {
                        if (sibling->lchild->black()) /* rchild->red() */ {
                            /* try make rchild red */
                            sibling->rchild->is_black = true;
                            sibling->is_black = false;
                            left_rotate(sibling);
                            sibling = parent->lchild;
                        }
                        /* now lchild of sibling must be red */
                        sibling->is_black = parent->is_black;
                        parent->is_black = true;
                        sibling->lchild->is_black = true;
                        right_rotate(parent);
                        break;
                    }
                }
            }
        }
        node->is_black = true;
        nil->parent.reset();
    }

    /* src will replace dest */
    void transplant(NodePtr dest, NodePtr src) {
        if (is_root(dest)) {
            root = src;
        } else {
            auto parent = dest->parent_shr();
            if (dest == parent->lchild) {
                parent->lchild = src;
            } else /* node is rchild */ {
                parent->rchild = src;
            }
        }
        /* FIXME: do we need to avoid setting nil's parent? */
        if (!is_nil(src)) {
            src->parent = dest->parent;
        }
    }

  public:
    void insert(const pair<K, V> &kv) {
        auto fix_node = insert_node(kv);
        insert_fixup(fix_node);
    }

  protected:
    NodePtr insert_node(const pair<K, V> &kv) {
        auto &[key, value] = kv;

        auto [pre, current] = search_node(key);
        assert(is_nil(current));
        /* insert the new node */
        auto node = new_node(pre, false, kv);
        if (is_nil(pre)) {
            /* inserting the first node as root */
            root = node;
        } else {
            if (key < pre->key()) {
                pre->lchild = node;
            } else {
                pre->rchild = node;
            }
        }

        /* fix_node*/
        return node;
    }

  public:
    void remove(const K &key) {
        auto [_, node] = search_node(key);
        if (is_nil(node)) {
            return;
        }
        auto [is_lost_black, fix_node] = remove_node(node);
        if (is_lost_black) {
            remove_fixup(fix_node);
        }
    }

  protected:
    pair<bool, NodePtr> remove_node(NodePtr node) {
        /* node / next_node */
        bool is_lost_black = false;
        /* the only child of node / the rchild of next_node(can be nil) */
        NodePtr fix_node;
        if (is_nil(node->lchild)) {
            transplant(node, node->rchild);
            fix_node = node->rchild;
            is_lost_black = node->is_black;
            /* in case node is leaf, thus fix_node is nil */
            fix_node->parent = node->parent;
        } else if (is_nil(node->rchild)) {
            transplant(node, node->lchild);
            fix_node = node->lchild;
            is_lost_black = node->is_black;
        } else {
            /* node's rchild's left-most child */
            auto next_node = node->rchild;
            while (!is_nil(next_node->lchild)) {
                next_node = next_node->lchild;
            }
            is_lost_black = next_node->is_black;
            fix_node = next_node->rchild;

            if (next_node != node->rchild) {
                /* if next_node is node's rchild, then fix_node is next_node's
                   only rchild. when transplanting next_node to node, fix_node
                   will be move up together. */
                transplant(next_node, fix_node);
                /* in case fix_node is nil */
                fix_node->parent = next_node->parent;
                /* if next_node is not node's rchild, we need to tansplant
                   node's rchild to next_node first. otherwise right sub-tree of
                   node will be transplanted by transplanting next_node */
                next_node->rchild = node->rchild;
                /* node's children both aren't nil */
                node->rchild->parent = next_node;
            } else {
                /* in case fix_node is nil */
                fix_node->parent = next_node;
            }
            transplant(node, next_node);
            /* connect node's lchild to next_node */
            next_node->lchild = node->lchild;
            node->lchild->parent = next_node;
            /* next_node inherits node's color */
            next_node->is_black = node->is_black;
        }
        /* node (or next_node that replaces node) has only one child, which is
           fix_node. we then transplant fix_node to node and add a extra black
           to it. note that fix_node can be nil when next_node is a leaf. */
        return {is_lost_black, fix_node};
    }

  protected:
    /* return pre and current */
    pair<NodePtr, NodePtr> search_node(const K &key) const {
        /* find the insertion point */
        /* we can't get nil's parent, need pre */
        auto pre = nil;
        auto current = root;
        while (!is_nil(current)) {
            /* inserting duplicate keys is an undefined behavior */
            if (key < current->key()) {
                pre = current;
                current = current->lchild;
            } else if (key == current->key()) {
                return {pre, current};
            } else {
                pre = current;
                current = current->rchild;
            }
        }
        return {pre, current};
    }

  public:
    V *search(const K &key) {
        auto [_, node] = search_node(key);
        if (!is_nil(node)) {
            return &node->kv->second;
        } else {
            return nullptr;
        }
    }

    void verify() const {
        assert(root->black());
        assert(nil->black());
        verify_black_height(root);
        verify_red_child(root);
    }

  private:
    size_t verify_black_height(const NodePtr node) const {
        if (is_nil(node)) {
            return 1;
        }
        auto lbh = verify_black_height(node->lchild);
        auto rbh = verify_black_height(node->rchild);
        assert(lbh == rbh);
        return node->is_black ? lbh + 1 : lbh;
    }

    void verify_red_child(const NodePtr node) const {
        if (is_nil(node)) {
            return;
        }
        if (node->red()) {
            assert(node->lchild->black() && node->rchild->black());
        }
        verify_red_child(node->lchild);
        verify_red_child(node->rchild);
    }

  public:
    friend ostream &operator<<(ostream &os, RbTree &t) {
        t.pre_order_depth(
            [&os](size_t depth, NodePtr node) {
                os << string(depth, '\t') << node->key() << ' ';
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
};