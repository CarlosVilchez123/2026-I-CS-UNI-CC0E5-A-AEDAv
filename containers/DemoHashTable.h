#include <iostream>
#include <string>
#include "../types.h"
#include "hashtable.h"

using namespace std;

void printLookup(HashTable<KVTrait<Ref, string>>& m, Ref key) {
    cout << "m[" << key << "] = " << m[key] << endl;
}

void printForRange(HashTable<KVTrait<Ref, string>>& m) {
    cout << "\nfor-range:" << endl;
    for (const auto& [key, value] : m)
        cout << "  Key: " << key << " -> Value: " << value << endl;
}

void printToString(const HashTable<KVTrait<Ref, string>>& m) {
    cout << "\noperator<< (toString):" << endl;
    cout << m << endl;
}

void printSizeComparison(const HashTable<KVTrait<Ref, string>>& original,
                         const HashTable<KVTrait<Ref, string>>& copia) {
    cout << "\nTamano Original: " << original.size()
         << " | Tamano Copia: "  << copia.size() << endl;
}

void DemoHashTable() {
    cout << "PRUEBAS HASHTABLE" << endl;

    HashTable<KVTrait<Ref, string>> m(3);
    m[11] = "once";
    m[15] = "quince";
    m[11] = "oncePri";
    m[3]  = "Tres";

    printLookup(m, 11);
    printForRange(m);
    printToString(m);

    HashTable<KVTrait<Ref, string>> copia(m);
    copia[99] = "Dato de Copia";
    printSizeComparison(m, copia);

    cout << "\nFIN PRUEBAS HASHTABLE" << endl;
}