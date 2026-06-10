#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <fstream>
#include "vector.h"
#include <functional>
#include "../types.h"
#include "traits.h"
using namespace std;

//HeapNode
template<typename T>
struct HeapNode {
    using value_type = T;
    T   m_data;
    Ref m_ref;
    HeapNode() : m_data(T{}), m_ref(Ref{}) {}
    HeapNode(T data, Ref ref) : m_data(data), m_ref(ref) {}

    friend ostream& operator<<(ostream& os, const HeapNode& n) {
        return os << "(" << n.m_data << "," << n.m_ref << ")";
    }
};

template<typename Trait>
class Heap {
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using Node       = typename Trait::Node;
    using MySelf     = Heap<Trait>;

private:
    Node *m_data;
    size_t m_size;
    size_t m_capacity;
    Comp                 m_comp;
    mutable shared_mutex m_mtx;

    void resize() {
        m_capacity = m_capacity * 2;
        auto* nd   = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) nd[i] = m_data[i];
        delete[] m_data;
        m_data = nd;
    }

    //indices
    size_t parent(size_t i) const { return (i - 1) / 2; }
    size_t left(size_t i)   const { return 2 * i + 1;   }
    size_t right(size_t i)  const { return 2 * i + 2;   }

    //swap
    void swap(size_t i, size_t j) {
        std::swap(m_data[i], m_data[j]);
    }

    //heapifyUp
    void heapifyUp(size_t i) {
        while (i > 0 && m_comp(m_data[i].m_data, m_data[parent(i)].m_data)) {
            swap(i, parent(i));
            i = parent(i);
        }
    }

    //heapifyDown
    void heapifyDown(size_t i) {
        auto best = i;
        auto l    = left(i);
        auto r    = right(i);
        if (l < m_size && m_comp(m_data[l].m_data, m_data[best].m_data)) best = l;
        if (r < m_size && m_comp(m_data[r].m_data, m_data[best].m_data)) best = r;
        if (best != i) { swap(i, best); heapifyDown(best); }
    }

    //toString arbol
    string treeToString() const {
        if (m_size == 0) return "  (vacio)\n";
        ostringstream oss;
        auto level_start = size_t{0};
        auto level_size  = size_t{1};
        while (level_start < m_size) {
            oss << "  ";
            auto end = min(level_start + level_size, m_size);
            for (auto i = level_start; i < end; ++i)
                oss << m_data[i].m_data << " ";
            oss << "\n";
            level_start += level_size;
            level_size  *= 2;
        }
        return oss.str();
    }

public:
    //constructores
    Heap(size_t capacity = 16)
        : m_data(new Node[capacity]), m_size(0), m_capacity(capacity) {}

    //copy constructor
        Heap(const Heap& other) : m_data(nullptr), m_size(0), m_capacity(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_data     = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
    }
    //move constructor
    Heap(Heap&& other) : m_data(nullptr), m_size(0), m_capacity(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_capacity = exchange(other.m_capacity, 0);
        m_size     = exchange(other.m_size,     0);
        m_data     = exchange(other.m_data,     nullptr);
    }
    //copy assignment
    Heap& operator=(const Heap& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            shared_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_data;
            m_capacity = other.m_capacity;
            m_size     = other.m_size;
            m_data     = new Node[m_capacity];
            for (size_t i = 0; i < m_size; ++i) m_data[i] = other.m_data[i];
        }
        return *this;
    }
    //move assignment
    Heap& operator=(Heap&& other) {
        if (this != &other) {
            unique_lock<shared_mutex> lock(m_mtx);
            unique_lock<shared_mutex> olock(other.m_mtx);
            delete[] m_data;
            m_capacity = exchange(other.m_capacity, 0);
            m_size     = exchange(other.m_size,     0);
            m_data     = exchange(other.m_data,     nullptr);
        }
        return *this;
    }

    virtual ~Heap() { delete[] m_data; }

    //insert
    void insert(value_type value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == m_capacity) resize();
        m_data[m_size] = Node(value, ref);
        heapifyUp(m_size);
        ++m_size;
    }

    //extrac
    tuple<value_type, Ref> extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) throw runtime_error("heap vacio");
        auto result  = make_tuple(m_data[0].m_data, m_data[0].m_ref);
        m_data[0]    = m_data[m_size - 1];
        --m_size;
        if (m_size > 0) heapifyDown(0);
        return result;
    }

    //peek
    tuple<value_type, Ref> peek() const {
        shared_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) throw runtime_error("heap vacio");
        return make_tuple(m_data[0].m_data, m_data[0].m_ref);
    }

    bool   isEmpty() const { shared_lock<shared_mutex> lock(m_mtx); return m_size == 0; }
    size_t size()    const { shared_lock<shared_mutex> lock(m_mtx); return m_size; }

    //forEach
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        shared_lock<shared_mutex> lock(m_mtx);
        for (size_t i = 0; i < m_size; ++i)
            func(m_data[i].m_data, forward<Args>(args)...);
    }

    Node* begin() { return m_data; }
    Node* end()   { return m_data + m_size; }
    
    //toString
    string toString() const {
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "Array: [";
        bool first = true;
        for (size_t i = 0; i < m_size; ++i) {
            if (!first) oss << ",";
            oss << m_data[i];
            first = false;
        }
        oss << "]\nTree:\n" << treeToString();
        return oss.str();
    }

    // operator<<
    friend ostream& operator<<(ostream& os, const Heap& h) {
        return os << h.toString();
    }

    //operator>>
    friend istream& operator>>(istream& is, Heap& h) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.clear(ios_base::failbit); return is; }
        value_type val; Ref ref; char comma, paren;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> paren)
                    if (comma == ',' && paren == ')')
                        h.insert(val, ref);
        return is;
    }
};

#endif // __HEAP_H__