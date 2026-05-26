#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <functional>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

template<typename T>
class HeapNode {
public:
    using value_type = T;
    using MySelf     = HeapNode<T>;

private:
    T   m_data;
    Ref m_ref;

public:
    HeapNode() : m_data(T()), m_ref(0) {}

    HeapNode(T data, Ref ref) : m_data(data), m_ref(ref) {}

    T&  getDataRef()      { return m_data; }
    T   getData()   const { return m_data; }
    Ref getRef()    const { return m_ref; }

    ~HeapNode() {}
};

//  Traits de comparación
//  MinHeapTrait → la raíz es el elemento mínimo (less<T>)
//  MaxHeapTrait → la raíz es el elemento máximo (greater<T>)
template<typename T>
struct MinHeapTrait : public BaseTrait<HeapNode<T>, less<T>> {};

template<typename T>
struct MaxHeapTrait : public BaseTrait<HeapNode<T>, greater<T>> {};

//  heap_forward_iterator
//  Iterador de acceso secuencial sobre el arreglo interno del Heap.
//  Avanza incrementando el puntero al nodo actual (recorrido por nivel).
template<typename Container>
class heap_forward_iterator
    : public general_iterator<Container, heap_forward_iterator<Container>> {
public:
    using MySelf = heap_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    // Avance: mueve el puntero al siguiente nodo en el arreglo
    MySelf operator++() { this->m_pNode++; return *this; }
};

//  implementacion del Heap<Trait>
//  Heap binario genérico sobre un arreglo dinámico.
//  El Trait define el tipo de nodo y el comparador, lo que permite usar
//  la misma clase tanto para MinHeap como para MaxHeap.
//
//  Propiedades:
//    - Inserción y extracción en O(log n)
//    - Acceso al elemento prioritario (peek) en O(1)
//    - Seguro para uso concurrente mediante shared_mutex
template<typename Trait>
class Heap {
public:
    using value_type     = typename Trait::value_type;
    using Comp           = typename Trait::Comp;
    using MySelf         = Heap<Trait>;
    using Node           = typename Trait::Node;
    using forward_iterator = heap_forward_iterator<MySelf>;

    friend forward_iterator;

private:
    Node*                m_data;      // arreglo dinámico de nodos
    size_t               m_size;      // cantidad de elementos actuales
    size_t               m_capacity;  // capacidad máxima del arreglo
    Comp                 m_comp;      // comparador definido por el Trait
    mutable shared_mutex m_mtx;       // mutex para concurrencia

    // Duplica la capacidad del arreglo cuando está lleno
    void resize() {
        m_capacity = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
        Node* fresh = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i)
            fresh[i] = m_data[i];
        delete[] m_data;
        m_data = fresh;
    }

    // Sube el nodo en la posición `index` hasta restaurar la propiedad heap
    // Se usa después de insertar un nuevo elemento al final
    void heapifyUp(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (m_comp(m_data[index].getData(), m_data[parent].getData())) {
                swap(m_data[index], m_data[parent]);
                index = parent;
            } else break;
        }
    }

    // Baja el nodo en la posición `index` hasta restaurar la propiedad heap
    // Se usa después de extraer la raíz y poner el último elemento al frente
    void heapifyDown(size_t index) {
        while (true) {
            size_t best  = index;
            size_t left  = 2 * index + 1;
            size_t right = 2 * index + 2;

            if (left  < m_size && m_comp(m_data[left].getData(),  m_data[best].getData())) best = left;
            if (right < m_size && m_comp(m_data[right].getData(), m_data[best].getData())) best = right;

            if (best == index) break;
            swap(m_data[index], m_data[best]);
            index = best;
        }
    }

public:

    explicit Heap(size_t capacity = 10)
        : m_data(new Node[capacity]), m_size(0), m_capacity(capacity), m_comp() {}

    virtual ~Heap() { delete[] m_data; }

    // clona el arreglo completo con shared_lock para seguridad
    Heap(const Heap& other) {
        shared_lock lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_comp     = other.m_comp;
        m_data     = new Node[m_capacity];
        for (size_t i = 0; i < m_size; ++i)
            m_data[i] = other.m_data[i];
    }

    // transfiere ownership del arreglo sin copiar elementos
    Heap(Heap&& other) {
        unique_lock lock(other.m_mtx);
        m_data     = exchange(other.m_data,     nullptr);
        m_size     = exchange(other.m_size,     0);
        m_capacity = exchange(other.m_capacity, 0);
        m_comp     = move(other.m_comp);
    }

    // Operaciones principales

    // Inserta un nuevo elemento manteniendo la propiedad heap
    void insert(value_type value, Ref ref = 0) {
        unique_lock lock(m_mtx);
        if (m_size == m_capacity) resize();
        m_data[m_size++] = Node(value, ref);
        heapifyUp(m_size - 1);
    }

    // Elimina la raíz y restaura la propiedad heap
    void extract() {
        unique_lock lock(m_mtx);
        if (m_size == 0) throw out_of_range("Heap::extract — heap is empty");
        swap(m_data[0], m_data[m_size - 1]);
        --m_size;
        if (m_size > 0) heapifyDown(0);
    }

    // Devuelve el elemento prioritario sin eliminarlo
    Node peek() const {
        shared_lock lock(m_mtx);
        if (m_size == 0) throw out_of_range("Heap::peek — heap is empty");
        return m_data[0];
    }

    bool   isEmpty() const { shared_lock lock(m_mtx); return m_size == 0; }
    size_t size()    const { shared_lock lock(m_mtx); return m_size; }

    // implementacion del toString
    string toString() const {
        shared_lock lock(m_mtx);
        ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < m_size; ++i) {
            if (i > 0) oss << ",";
            oss << "(" << m_data[i].getData() << "," << m_data[i].getRef() << ")";
        }
        oss << "]";
        return oss.str();
    }

    // Imprime el heap en cualquier flujo de salida
    friend ostream& operator<<(ostream& os, const Heap& h) {
        return os << h.toString();
    }

    // Carga el heap desde un flujo con formato
    friend istream& operator>>(istream& is, Heap& h) {
        char ch;
        if (!(is >> ch) || ch != '[') { is.setstate(ios::failbit); return is; }
        while (is.peek() != ']' && is >> ch) {
            if (ch == '(') {
                value_type val;
                Ref        ref;
                char       comma, close;
                if (is >> val >> comma >> ref >> close)
                    h.insert(val, ref);
            }
        }
        is >> ch; // consume ']'
        return is;
    }

    // Iteradores 

    // Recorre el arreglo en orden de nivel
    forward_iterator begin() { return forward_iterator(this, m_data); }
    forward_iterator end()   { return forward_iterator(this, m_data + m_size); }
};

#endif // __HEAP_H__
