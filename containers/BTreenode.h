#ifndef __BTREE_NODE_H__
#define __BTREE_NODE_H__

#include <iostream>
#include "Vector.h"
#include "../types.h"
#include "traits.h"

enum class bt_code {
    ok,
    overflow,
    underflow,
    duplicate,
    notFound,
    rootMerged
};

template <typename Value>
struct BTreeEntry {
    using value_type = Value;

    Value m_data;
    Ref   m_ref;
    size  m_useCount;

    BTreeEntry()
        : m_data(Value{}), m_ref(Ref{}), m_useCount(0) {}

    BTreeEntry(const Value& data, Ref ref)
        : m_data(data), m_ref(ref), m_useCount(0) {}

    operator const Value&() const { return m_data; }

    size touch()          { return ++m_useCount; }
    size useCount() const { return m_useCount;   }

    flag operator< (const BTreeEntry& o) const { return m_data <  o.m_data; }
    flag operator> (const BTreeEntry& o) const { return m_data >  o.m_data; }
    flag operator==(const BTreeEntry& o) const { return m_data == o.m_data; }

    friend std::ostream& operator<<(std::ostream& os, const BTreeEntry& e) {
        return os << e.m_data << ":" << e.m_ref;
    }
    friend std::istream& operator>>(std::istream& is, BTreeEntry& e) {
        char sep{};
        return is >> e.m_data >> sep >> e.m_ref;
    }
};

template <typename Trait, typename _Child>
struct BTreeNode {
    using value_type = typename Trait::value_type;
    using Entry      = BTreeEntry<value_type>;
    using Child      = _Child;                   // tipo real del hijo

    std::vector<Entry>   m_keys;
    std::vector<Child*>  m_children;             // ya tipado — sin cast
    size                 m_keyCount;
    size                 m_maxKeys;
    size                 m_childMaxKeys;
    flag                 m_unique;

    explicit BTreeNode(size maxKeys, flag unique = true)
        : m_keyCount(0)
        , m_maxKeys(maxKeys)
        , m_childMaxKeys(maxKeys)
        , m_unique(unique)
    {
        m_keys.assign(m_maxKeys + 1, Entry{});
        m_children.assign(m_maxKeys + 2, nullptr);
    }

    BTreeNode(const BTreeNode&)            = delete;
    BTreeNode& operator=(const BTreeNode&) = delete;

    ~BTreeNode() {
        for (size i = 0; i <= m_keyCount; ++i)
            delete m_children[i];
        m_children.assign(m_children.size(), nullptr);
    }

    flag isLeaf()      const { return m_children[0] == nullptr; }
    flag isFull()      const { return m_keyCount >= m_maxKeys;  }
    flag isOverflow()  const { return m_keyCount >  m_maxKeys;  }
    size minKeys()     const { return (2 * m_maxKeys) / 3;      }
    flag isUnderflow() const { return m_keyCount < minKeys();   }
    size freeCells()   const { return m_maxKeys - m_keyCount;   }
    flag isRoot()      const { return m_childMaxKeys != m_maxKeys; }

    void clearKeys() { m_keyCount = 0; }

    void releaseChildren() {
        for (size i = 0; i <= m_keyCount; ++i) {
            delete m_children[i];
            m_children[i] = nullptr;
        }
    }
};

#endif // __BTREE_NODE_H__