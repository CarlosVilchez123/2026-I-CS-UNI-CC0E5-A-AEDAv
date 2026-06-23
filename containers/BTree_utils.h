#ifndef __BTREE_UTILS_H__
#define __BTREE_UTILS_H__

#include "../types.h"
#include "Vector.h"

namespace btree_utils {

template <typename Container, typename Item>
void insertAt(Container& c, const Item& item, size pos) {
    sindex n = asSIndex(c.numElems());
    for (sindex i = n - 2; i >= asSIndex(pos); --i)
        c[asSize(i + 1)] = c[asSize(i)];
    c[pos] = item;
}

template <typename Container>
void removeAt(Container& c, size pos) {
    size n = c.numElems();
    for (size i = pos + 1; i < n; ++i)
        c[i - 1] = c[i];
}

template <typename EntryVec, typename Value>
size lowerBound(const EntryVec& keys, size keyCount, const Value& key) {
    size lo = 0, hi = keyCount;
    while (lo < hi) {
        size mid = (lo + hi) / 2;
        if (key > keys[mid].m_data) lo = mid + 1;
        else                         hi = mid;
    }
    return lo;
}

} // namespace btree_utils

#endif // __BTREE_UTILS_H__