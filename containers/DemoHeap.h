#ifndef __DEMO_HEAP_H__
#define __DEMO_HEAP_H__

#include <iostream>
#include <sstream>
#include <thread>
#include "../types.h"
#include "heap.h"
using namespace std;

using MinHeap = Heap<MinHeapTrait<int>>;
using MaxHeap = Heap<MaxHeapTrait<int>>;

static const initializer_list<int> kTestValues = {38, 7, 91, 24, 55, 13, 46};

void DemoMinHeap() {
    cout << "\nMINHEAP" << endl;
    MinHeap h;

    cout << "insert:" << endl;
    for (int v : kTestValues) {
        h.insert(v, v * 5);
        cout << "insert(" << v << ") -> " << h.toString() << endl;
    }

    cout << "Tamano: " << h.size() << endl;
    cout << "peek (minimo): (" << h.peek().getData()
         << "," << h.peek().getRef() << ")" << endl;

    // operator
    cout << "\noperator<<: " << h << endl;

    // operator>>
    MinHeap h2;
    istringstream iss("[(3,15),(9,45),(6,30),(21,105)]");
    iss >> h2;
    cout << "operator>> cargado: " << h2 << endl;
    cout << "peek cargado: (" << h2.peek().getData()
         << "," << h2.peek().getRef() << ")" << endl;

    // copy constructor
    MinHeap copia(h);
    copia.insert(100, 500);
    cout << "\nOriginal size: " << h.size()
         << " | Copia size: "  << copia.size() << endl;

    // move constructor
    MinHeap movido(move(copia));
    cout << "Movido size: "        << movido.size()
         << " | Copia tras move: " << copia.size() << endl;

    // forward_iterator
    cout << "\nforward_iterator:" << endl;
    for (auto it = h.begin(); it != h.end(); ++it)
        cout << "  (" << (*it).getData() << "," << (*it).getRef() << ")" << endl;

    // extract orden ascendente
    cout << "\nextract orden ascendente:" << endl;
    while (!h.isEmpty()) {
        cout << "  extract -> (" << h.peek().getData()
             << "," << h.peek().getRef() << ")" << endl;
        h.extract();
    }
}

void DemoMaxHeap() {
    cout << "\nMAXHEAP" << endl;
    MaxHeap h;

    cout << "insert:" << endl;
    for (int v : kTestValues) {
        h.insert(v, v * 3);
        cout << "insert(" << v << ") -> " << h.toString() << endl;
    }

    cout << "Tamano: " << h.size() << endl;
    cout << "peek (maximo): (" << h.peek().getData()
         << "," << h.peek().getRef() << ")" << endl;

    // forward_iterator
    cout << "\nforward_iterator:" << endl;
    for (auto it = h.begin(); it != h.end(); ++it)
        cout << "  (" << (*it).getData() << "," << (*it).getRef() << ")" << endl;

    // extract orden descendente
    cout << "\nextract orden descendente:" << endl;
    while (!h.isEmpty()) {
        cout << "  extract -> (" << h.peek().getData()
             << "," << h.peek().getRef() << ")" << endl;
        h.extract();
    }
}

void DemoHeapConcurrencia() {
    cout << "\nCONCURRENCIA" << endl;
    MinHeap h;
    auto worker = [&h](int id) {
        for (int i = 0; i < 200; ++i)
            h.insert(i * id, id);
    };
    thread th1(worker,1), th2(worker,2), th3(worker,3),
           th4(worker,4), th5(worker,5);
    th1.join(); th2.join(); th3.join(); th4.join(); th5.join();
    cout << "size esperado 1000: " << h.size() << endl;
    cout << "peek minimo: (" << h.peek().getData()
         << "," << h.peek().getRef() << ")" << endl;
}

void HeapDemo() {
    cout << "\nPRUEBAS HEAP" << endl;
    DemoMinHeap();
    DemoMaxHeap();
    DemoHeapConcurrencia();
    cout << "\nFIN DE LAS PRUEBAS" << endl;
}

#endif // __DEMO_HEAP_H__