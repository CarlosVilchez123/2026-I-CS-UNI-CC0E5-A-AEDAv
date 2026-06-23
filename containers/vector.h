#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <sstream>
#include <utility>
#include "../types.h"

template <typename T>
struct VectorNode {
    using value_type = T;
    T m_data;

    VectorNode() : m_data(T{}) {}
    explicit VectorNode(const T &data) : m_data(data) {}
    T       &getDataRef()       { return m_data; }
    const T &getDataRef() const { return m_data; }
};

template <typename Container, typename Derived>
class VectorIteratorBase {
public:
    using Node = typename Container::Node;

protected:
    Node *m_pNode;

public:
    VectorIteratorBase(Node *p) : m_pNode(p) {}

    typename Container::value_type &operator*() { return m_pNode->getDataRef(); }

    friend flag operator==(const Derived &a, const Derived &b) { return a.m_pNode == b.m_pNode; }
    friend flag operator!=(const Derived &a, const Derived &b) { return a.m_pNode != b.m_pNode; }
};

template <typename Container>
class VectorForwardIterator
    : public VectorIteratorBase<Container, VectorForwardIterator<Container>> {
    using Base = VectorIteratorBase<Container, VectorForwardIterator<Container>>;
public:
    using Base::Base;
    VectorForwardIterator &operator++() { ++this->m_pNode; return *this; }
};

template <typename Container>
class VectorBackwardIterator
    : public VectorIteratorBase<Container, VectorBackwardIterator<Container>> {
    using Base = VectorIteratorBase<Container, VectorBackwardIterator<Container>>;
public:
    using Base::Base;
    VectorBackwardIterator &operator++() { --this->m_pNode; return *this; }
};

template <typename T>
class Vector {
public:
    using value_type        = T;
    using Node              = VectorNode<T>;
    using forward_iterator  = VectorForwardIterator<Vector<T>>;
    using backward_iterator = VectorBackwardIterator<Vector<T>>;

private:
    Node  *m_data;
    size   m_sz;
    size   m_capacity;

    void grow() {
        size newCap = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
        Node *tmp = new Node[newCap];
        for (size i = 0; i < m_sz; ++i)
            tmp[i] = m_data[i];
        delete[] m_data;
        m_data     = tmp;
        m_capacity = newCap;
    }

public:
    explicit Vector(size capacity = 10)
        : m_data(new Node[capacity]), m_sz(0), m_capacity(capacity) {}

    Vector(const Vector &other)
        : m_data(new Node[other.m_capacity])
        , m_sz(other.m_sz)
        , m_capacity(other.m_capacity)
    {
        for (size i = 0; i < m_sz; ++i)
            m_data[i] = other.m_data[i];
    }

    Vector(Vector &&other) noexcept
        : m_data(other.m_data), m_sz(other.m_sz), m_capacity(other.m_capacity)
    {
        other.m_data     = nullptr;
        other.m_sz       = 0;
        other.m_capacity = 0;
    }

    Vector &operator=(const Vector &other) {
        if (this == &other) return *this;
        delete[] m_data;
        m_capacity = other.m_capacity;
        m_sz       = other.m_sz;
        m_data     = new Node[m_capacity];
        for (size i = 0; i < m_sz; ++i)
            m_data[i] = other.m_data[i];
        return *this;
    }

    Vector &operator=(Vector &&other) noexcept {
        if (this == &other) return *this;
        delete[] m_data;
        m_data           = other.m_data;
        m_sz             = other.m_sz;
        m_capacity       = other.m_capacity;
        other.m_data     = nullptr;
        other.m_sz       = 0;
        other.m_capacity = 0;
        return *this;
    }

    ~Vector() { delete[] m_data; }

    void push_back(const T &val) {
        if (m_sz == m_capacity) grow();
        m_data[m_sz++] = Node(val);
    }

    void assign(size n, const T &val) {
        if (n > m_capacity) {
            delete[] m_data;
            m_capacity = n;
            m_data     = new Node[m_capacity];
        }
        for (size i = 0; i < n; ++i)
            m_data[i] = Node(val);
        m_sz = n;
    }

    T       &operator[](size i)       { return m_data[i].m_data; }
    const T &operator[](size i) const { return m_data[i].m_data; }

    T       &back()       { return m_data[m_sz - 1].m_data; }
    const T &back() const { return m_data[m_sz - 1].m_data; }

    void pop_back() { if (m_sz > 0) --m_sz; }

    size numElems()  const { return m_sz;       }
    size capacity()  const { return m_capacity; }
    flag empty()     const { return m_sz == 0;  }

    flag operator==(const Vector &o) const {
        if (m_sz != o.m_sz) return false;
        for (size i = 0; i < m_sz; ++i)
            if (!(m_data[i].m_data == o.m_data[i].m_data)) return false;
        return true;
    }
    flag operator!=(const Vector &o) const { return !(*this == o); }

    forward_iterator  begin()  { return forward_iterator (m_data);        }
    forward_iterator  end()    { return forward_iterator (m_data + m_sz); }
    backward_iterator rbegin() { return backward_iterator(m_data + m_sz - 1); }
    backward_iterator rend()   { return backward_iterator(m_data - 1);        }

    template <typename Func, typename... Args>
    void forEach(Func func, Args &&...args) {
        for (size i = 0; i < m_sz; ++i)
            func(m_data[i].m_data, std::forward<Args>(args)...);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[";
        for (size i = 0; i < m_sz; ++i) {
            if (i > 0) oss << ", ";
            oss << m_data[i].m_data;
        }
        oss << "]";
        return oss.str();
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector &v) {
        return os << v.toString();
    }
};

#endif