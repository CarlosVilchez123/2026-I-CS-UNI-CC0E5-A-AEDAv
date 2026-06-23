#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <iostream>
#include <sstream>
#include <utility>
#include "../types.h"
#include "general_iterator.h"
#include "util.h"

template <typename T>
struct VectorTrait {
    using value_type = T;
};

template <typename T>
class VectorNode {
    T   m_data;
    Ref m_ref;
public:
    VectorNode() : m_data(T{}), m_ref(Ref{}) {}
    VectorNode(const T& data, Ref ref = Ref{}) : m_data(data), m_ref(ref) {}

    VectorNode(const VectorNode& o)            : m_data(o.m_data), m_ref(o.m_ref) {}
    VectorNode(VectorNode&& o) noexcept        : m_data(std::move(o.m_data)), m_ref(std::move(o.m_ref)) {}
    VectorNode& operator=(const VectorNode& o) { m_data = o.m_data; m_ref = o.m_ref; return *this; }
    VectorNode& operator=(VectorNode&& o) noexcept {
        m_data = std::move(o.m_data);
        m_ref  = std::move(o.m_ref);
        return *this;
    }

    T&       getDataRef()       { return m_data; }
    const T& getDataRef() const { return m_data; }
    T        getData()    const { return m_data; }
    void     setData(const T& d){ m_data = d; }
    Ref      getRef()     const { return m_ref; }
    void     setRef(Ref r)      { m_ref = r; }

    flag operator==(const VectorNode& o) const { return m_data == o.m_data && m_ref == o.m_ref; }

    friend std::ostream& operator<<(std::ostream& os, const VectorNode& n) {
        return os << "(" << n.m_data << ", " << n.m_ref << ")";
    }
    friend std::istream& operator>>(std::istream& is, VectorNode& n) {
        char ch; T data; Ref ref;
        if (!(is >> ch) || ch != '(')         { is.setstate(std::ios_base::failbit); return is; }
        if (!(is >> data >> ch) || ch != ',') { is.setstate(std::ios_base::failbit); return is; }
        if (!(is >> ref >> ch) || ch != ')')  { is.setstate(std::ios_base::failbit); return is; }
        n.setData(data); n.setRef(ref);
        return is;
    }
};

template <typename Container>
class vector_forward_iterator
    : public general_iterator<Container, vector_forward_iterator<Container>> {
public:
    using MySelf = vector_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { ++this->m_pNode; return *this; }
};

template <typename Container>
class vector_backward_iterator
    : public general_iterator<Container, vector_backward_iterator<Container>> {
public:
    using MySelf = vector_backward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { --this->m_pNode; return *this; }
};

template <typename Trait>
class Vector {
public:
    using value_type        = typename Trait::value_type;
    using Node              = VectorNode<value_type>;
    using forward_iterator  = vector_forward_iterator<Vector<Trait>>;
    using backward_iterator = vector_backward_iterator<Vector<Trait>>;

    friend forward_iterator;
    friend backward_iterator;

private:
    Node *m_data;
    size  m_sz;
    size  m_capacity;

    void grow() {
        size newCap = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
        Node *tmp = new Node[newCap];
        for (size i = 0; i < m_sz; ++i) tmp[i] = m_data[i];
        delete[] m_data;
        m_data     = tmp;
        m_capacity = newCap;
    }

public:
    explicit Vector(size capacity = 10)
        : m_data(new Node[capacity]), m_sz(0), m_capacity(capacity) {}

    Vector(const Vector& o)
        : m_data(new Node[o.m_capacity]), m_sz(o.m_sz), m_capacity(o.m_capacity) {
        for (size i = 0; i < m_sz; ++i) m_data[i] = o.m_data[i];
    }

    Vector(Vector&& o) noexcept
        : m_data(o.m_data), m_sz(o.m_sz), m_capacity(o.m_capacity) {
        o.m_data = nullptr; o.m_sz = 0; o.m_capacity = 0;
    }

    Vector& operator=(const Vector& o) {
        if (this == &o) return *this;
        delete[] m_data;
        m_capacity = o.m_capacity; m_sz = o.m_sz;
        m_data = new Node[m_capacity];
        for (size i = 0; i < m_sz; ++i) m_data[i] = o.m_data[i];
        return *this;
    }

    Vector& operator=(Vector&& o) noexcept {
        if (this == &o) return *this;
        delete[] m_data;
        m_data = o.m_data; m_sz = o.m_sz; m_capacity = o.m_capacity;
        o.m_data = nullptr; o.m_sz = 0; o.m_capacity = 0;
        return *this;
    }

    ~Vector() { delete[] m_data; }

    void push_back(const value_type& val, Ref ref = Ref{}) {
        if (m_sz == m_capacity) grow();
        m_data[m_sz++] = Node(val, ref);
    }

    void assign(size n, const value_type& val, Ref ref = Ref{}) {
        if (n > m_capacity) {
            delete[] m_data;
            m_capacity = n;
            m_data     = new Node[m_capacity];
        }
        for (size i = 0; i < n; ++i) m_data[i] = Node(val, ref);
        m_sz = n;
    }

    // Acceso al Node completo (data + ref)
    Node&       node(size i)       { return m_data[i]; }
    const Node& node(size i) const { return m_data[i]; }

    // Acceso directo al dato (conveniencia, compatible con código anterior)
    value_type&       operator[](size i)       { return m_data[i].getDataRef(); }
    const value_type& operator[](size i) const { return m_data[i].getDataRef(); }

    value_type&       back()       { return m_data[m_sz - 1].getDataRef(); }
    const value_type& back() const { return m_data[m_sz - 1].getDataRef(); }

    Node&       backNode()       { return m_data[m_sz - 1]; }
    const Node& backNode() const { return m_data[m_sz - 1]; }

    void pop_back() { if (m_sz > 0) --m_sz; }

    size numElems() const { return m_sz;       }
    size capacity() const { return m_capacity; }
    flag empty()    const { return m_sz == 0;  }

    flag operator==(const Vector& o) const {
        if (m_sz != o.m_sz) return false;
        for (size i = 0; i < m_sz; ++i)
            if (!(m_data[i] == o.m_data[i])) return false;
        return true;
    }
    flag operator!=(const Vector& o) const { return !(*this == o); }

    forward_iterator  begin()  { return forward_iterator (this, m_data);            }
    forward_iterator  end()    { return forward_iterator (this, m_data + m_sz);     }
    backward_iterator rbegin() { return backward_iterator(this, m_data + m_sz - 1); }
    backward_iterator rend()   { return backward_iterator(this, m_data - 1);        }

    template <typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        ForEach(begin(), end(), func, std::forward<Args>(args)...);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "[";
        for (size i = 0; i < m_sz; ++i) {
            if (i > 0) oss << ", ";
            oss << m_data[i];
        }
        oss << "]";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector& v) {
        return os << v.toString();
    }

    friend std::istream& operator>>(std::istream& is, Vector& v) { return is; }
};

#endif