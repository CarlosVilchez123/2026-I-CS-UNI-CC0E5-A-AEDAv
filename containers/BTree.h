#ifndef __BTREE_H__
#define __BTREE_H__

#include <stdexcept>
#include <utility>
#include "../types.h"
#include "traits.h"
#include "BTreePage.h"
#include "Vector.h"

template <typename Trait>
class BTree {
public:
    using value_type            = typename Trait::value_type;
    using Comp                  = typename Trait::Comp;
    static constexpr size Order = Trait::Order;
    using Page                  = BTreePage<Trait>;
    using Entry                 = typename Page::Entry;
    using Result                = KVResult<value_type, Ref>;

private:
    Page*          m_root;
    level          m_height;
    flag           m_unique;
    size           m_numKeys;
    mutable SMutex m_mtx;

    Page* cloneSubtree(const Page* src) const {
        if (!src) return nullptr;
        auto* dst           = new Page(src->m_maxKeys, src->m_unique);
        dst->m_childMaxKeys = src->m_childMaxKeys;
        dst->m_keyCount     = src->m_keyCount;
        dst->m_keys         = src->m_keys;
        for (size i = 0; i <= src->m_keyCount; ++i)
            dst->m_children[i] = cloneSubtree(src->m_children[i]);
        return dst;
    }

public:
    explicit BTree(flag unique = true)
        : m_root(new Page(2 * Order + 1, unique))
        , m_height(1), m_unique(unique), m_numKeys(0)
    {
        m_root->m_childMaxKeys = Order;
    }

    BTree(const BTree& other)
        : m_root(nullptr), m_height(1), m_unique(true), m_numKeys(0)
    {
        SLock<> lock(other.m_mtx);
        m_root    = cloneSubtree(other.m_root);
        m_height  = other.m_height;
        m_unique  = other.m_unique;
        m_numKeys = other.m_numKeys;
    }

    BTree& operator=(const BTree& other) {
        if (this == &other) return *this;
        ULock<> lkSelf(m_mtx);
        SLock<> lkOther(other.m_mtx);
        delete m_root;
        m_root    = cloneSubtree(other.m_root);
        m_height  = other.m_height;
        m_unique  = other.m_unique;
        m_numKeys = other.m_numKeys;
        return *this;
    }

    BTree(BTree&& other) noexcept
        : m_root(nullptr), m_height(1), m_unique(true), m_numKeys(0)
    {
        ULock<> lock(other.m_mtx);
        m_root    = std::exchange(other.m_root,    nullptr);
        m_height  = std::exchange(other.m_height,  1);
        m_unique  = other.m_unique;
        m_numKeys = std::exchange(other.m_numKeys, 0);
    }

    BTree& operator=(BTree&& other) noexcept {
        if (this == &other) return *this;
        ULock<> lkSelf(m_mtx);
        ULock<> lkOther(other.m_mtx);
        delete m_root;
        m_root    = std::exchange(other.m_root,    nullptr);
        m_height  = std::exchange(other.m_height,  1);
        m_unique  = other.m_unique;
        m_numKeys = std::exchange(other.m_numKeys, 0);
        return *this;
    }

    ~BTree() { delete m_root; }

    flag insert(const value_type& key, Ref ref = Ref{}) {
        ULock<> lock(m_mtx);
        bt_code result = m_root->insert(key, ref);
        if (result == bt_code::duplicate) return false;
        ++m_numKeys;
        if (result == bt_code::overflow) { m_root->splitRoot(); ++m_height; }
        return true;
    }

    Result remove(const value_type& key) {
        ULock<> lock(m_mtx);
        value_type outKey{}; Ref outRef{};
        bt_code result = m_root->remove(key, outKey, outRef);
        if (result == bt_code::notFound)
            throw std::runtime_error("BTree::remove - clave no encontrada");
        --m_numKeys;
        if (result == bt_code::rootMerged) --m_height;
        return {outKey, outRef};
    }

    Result search(const value_type& key) const {
        SLock<> lock(m_mtx);
        value_type outKey{}; Ref outRef{};
        if (!m_root->search(key, outKey, outRef))
            throw std::runtime_error("BTree::search - clave no encontrada");
        return {outKey, outRef};
    }

    size  numKeys() const { SLock<> l(m_mtx); return m_numKeys; }
    level height()  const { SLock<> l(m_mtx); return m_height;  }
    size  order()   const { return Order; }
    flag  empty()   const { SLock<> l(m_mtx); return m_numKeys == 0; }

    template <typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        SLock<> lock(m_mtx);
        m_root->forEach(0, func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    Entry* firstThat(Func func, Args&&... args) {
        SLock<> lock(m_mtx);
        return m_root->firstThat(0, func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void forEachPage(Func func, Args&&... args) {
        SLock<> lock(m_mtx);
        m_root->forEachPage(0, func, std::forward<Args>(args)...);
    }

    class Iterator {
        Vector<VectorTrait<Page*>> m_stack;
        const BTree*       m_owner;

        void descendLeft(Page* node, size idx) {
            while (node && node->m_keyCount > 0) {
                m_stack.push_back(node, static_cast<Ref>(idx));
                node = node->m_children[idx];
                idx  = 0;
            }
        }

    public:
        Iterator() : m_owner(nullptr) {}

        explicit Iterator(Page* root, const BTree* owner) : m_owner(owner) {
            if (root) descendLeft(root, 0);
        }

        Entry& operator*() const {
            Page* page = m_stack.back();
            size  idx  = asSize(m_stack.backNode().getRef());
            return page->m_keys.node(idx).getDataRef();
        }

        Entry* operator->() const { return &operator*(); }

        Iterator& operator++() {
            Page* page = m_stack.back();
            size  idx  = asSize(m_stack.backNode().getRef());
            m_stack.pop_back();
            if (idx + 1 < page->m_keyCount)
                m_stack.push_back(page, static_cast<Ref>(idx + 1));
            Page* right = page->m_children.node(idx + 1).getDataRef();
            if (right) descendLeft(right, 0);
            return *this;
        }

        flag operator==(const Iterator& o) const { return m_stack == o.m_stack; }
        flag operator!=(const Iterator& o) const { return !(*this == o); }
    };

    Iterator begin() const { SLock<> l(m_mtx); return Iterator(m_root, this); }
    Iterator end()   const { return Iterator(); }

    String toString() const {
        OSStream oss;
        oss << "[";
        flag first = true;
        for (const auto& e : *this) {
            if (!first) oss << ", ";
            oss << "(" << e << ")";
            first = false;
        }
        oss << "]";
        return oss.str();
    }

    friend OStream& operator<<(OStream& os, const BTree& t) {
        return os << t.toString();
    }

    friend IStream& operator>>(IStream& is, BTree& t) {
        char ch{};
        if (!(is >> ch) || ch != '[') { is.setstate(std::ios_base::failbit); return is; }
        Entry e; char paren{};
        while (is >> ch && ch != ']')
            if (ch == '(' && (is >> e >> paren))
                t.insert(e.m_data, e.m_ref);
        return is;
    }
};

#endif // __BTREE_H__