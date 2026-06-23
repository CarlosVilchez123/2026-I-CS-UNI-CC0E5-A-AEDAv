#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>
#include <sstream>
#include "../types.h"

template <typename Container>
void Print(Container& c, std::ostream& os) {
    os << c << std::endl;
}

template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args&&... args) {
    for (auto it = begin; it != end; ++it)
        func(*it, std::forward<Args>(args)...);
}

template <typename Container, typename Func, typename... Args>
void ForEach(Container& container, Func func, Args&&... args) {
    ForEach(container.begin(), container.end(), func, std::forward<Args>(args)...);
}

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
        if (key > keys[mid]) lo = mid + 1;
        else                  hi = mid;
    }
    return lo;
}

}

template <typename Container>
void testIO(Container& c, std::ostream& os) {
    os << "\n=== operator<< / operator>> ===\n";
    std::ostringstream oss;
    oss << c;
    os << "serializado:   " << oss.str() << "\n";
    Container c2;
    std::istringstream iss(oss.str());
    iss >> c2;
    os << "deserializado: " << c2 << "\n";
}

template <typename Container, typename InsertFn>
void testCopyMove(Container& original, InsertFn insertExtra, std::ostream& os) {
    os << "\n=== Copy / Move ===\n";
    Container copia(original);
    insertExtra(copia);
    os << "original : size=" << original.numKeys() << "  " << original << "\n";
    os << "copia    : size=" << copia.numKeys()    << "  " << copia    << "\n";
    Container movida(std::move(copia));
    os << "movida   : size=" << movida.numKeys() << "  " << movida << "\n";
    os << "fuente tras move: size=" << copia.numKeys() << "\n";
}

#endif