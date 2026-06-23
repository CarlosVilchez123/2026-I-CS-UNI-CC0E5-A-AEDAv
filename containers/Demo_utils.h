#ifndef __DEMO_UTILS_H__
#define __DEMO_UTILS_H__

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include "../types.h"

inline void printSection(const std::string& title) {
    std::cout << "\n[" << title << "]\n";
}
inline void printHeader(const std::string& name) {
    std::cout << "\n             PRUEBAS " << name << "             \n";
}
inline void printFooter(const std::string& name) {
    std::cout << "\n             FIN " << name << "             \n";
}

template <typename Container, typename InsertFn>
void testCopyMove(Container& original, InsertFn insertExtra) {
    printSection("Copy constructor");
    Container copia(original);
    insertExtra(copia);
    std::cout << "  Original : size=" << original.numKeys() << "  " << original << "\n";
    std::cout << "  Copia    : size=" << copia.numKeys()    << "  " << copia    << "\n";

    printSection("Move constructor");
    Container movida(std::move(copia));
    std::cout << "  Movida   : size=" << movida.numKeys() << "  " << movida << "\n";
    std::cout << "  Fuente tras move: size=" << copia.numKeys() << "\n";
}

template <typename Container>
void testIO(const Container& c) {
    printSection("operator<< / operator>>");
    std::ostringstream oss;
    oss << c;
    std::cout << "  Serializado  : " << oss.str() << "\n";
    Container c2;
    std::istringstream iss(oss.str());
    iss >> c2;
    std::cout << "  Deserializado: " << c2 << "\n";
}

template <typename Container>
void testIteration(Container& c) {
    printSection("forEach / range-for");
    std::cout << "  range-for: ";
    for (auto& item : c) std::cout << item << " ";
    std::cout << "\n";
}

#endif // __DEMO_UTILS_H__