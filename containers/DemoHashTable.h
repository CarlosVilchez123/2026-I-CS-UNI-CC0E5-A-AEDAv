#include <iostream>
#include <string>
#include <sstream>
#include "../types.h"
#include "hashtable.h"

using namespace std;

void DemoHashTable() {
    cout << "Prueba hastbale" << endl;

    HashTable<int, int> tabla(7);
    tabla[101] = 500;
    tabla[202] = 300;
    tabla[303] = 800;
    tabla[404] = 150;
    tabla[505] = 600;
    tabla[101] = 450; // sobrescribe

    cout << "tabla[101] = " << tabla[101] << endl;
    cout << "tabla[303] = " << tabla[303] << endl;
    cout << "Tamano: " << tabla.size() << endl;

    // --- contains y remove ---
    cout << "\nCONTAINS Y REMOVE" << endl;
    cout << "contains(202): " << (tabla.contains(202) ? "si" : "no") << endl;
    cout << "contains(999): " << (tabla.contains(999) ? "si" : "no") << endl;

    tabla.remove(202);
    cout << "Tras remove(202):" << endl;
    cout << "contains(202): " << (tabla.contains(202) ? "si" : "no") << endl;
    cout << "Tamano: " << tabla.size() << endl;

    // --- for-range ---
    cout << "\nFOR-RANGE" << endl;
    for (const auto& [key, value] : tabla)
        cout << "  " << key << " -> " << value << endl;

    // --- operator<< ---
    cout << "\nOPERATOR<<" << endl;
    cout << tabla << endl;

    // --- operator>> ---
    cout << "\nOPERATOR>> " << endl;
    HashTable<int, int> cargado(4);
    istringstream iss("{11:100,22:200,33:300}");
    iss >> cargado;
    cout << "Cargado desde stream: " << cargado << endl;

    // --- copy constructor ---
    cout << "\nCOPIA" << endl;
    HashTable<int, int> copia(cargado);
    copia[44] = 400;
    cout << "Original size: " << cargado.size()
         << " | Copia size: " << copia.size() << endl;

    // --- move constructor ---
    cout << "\nMOVE" << endl;
    HashTable<int, int> movido(move(copia));
    cout << "Movido size: "        << movido.size()
         << " | Copia tras move: " << copia.size() << endl;
}