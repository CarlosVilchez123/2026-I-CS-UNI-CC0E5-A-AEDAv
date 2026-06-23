#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <cctype>
#include "types.h"
#include "BTree.h"
#include "traits.h"
#include "demo_utils.h"

using Trait = BTreeOrden3<TypeBTree>;
using BT    = BTree<Trait>;

static void concurrencyWorker(BT& tree, Ref workerId) {
    for (size i = 0; i < 200; ++i)
        tree.insert(TypeBTree('a' + ((workerId * 7 + (Ref)i) % 26)), workerId);
}

void DemoBTree() {

    printSection("Insertar");
    BT bt;
    const std::string keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (size i = 0; i < keys.size(); ++i)
        bt.insert(TypeBTree(keys[i]), Ref(i * i));
    std::cout << "  size="   << bt.numKeys()
              << "  height=" << bt.height()
              << "  order="  << bt.order() << "\n";

    printSection("search");
    try {
        auto [val, ref] = bt.search(TypeBTree('Z'));
        std::cout << "  search('Z') -> encontrado  valor=" << val << "  ref=" << ref << "\n";
    } catch (const std::runtime_error& e) {
        std::cout << "  search('Z') -> " << e.what() << "\n";
    }
    try {
        bt.search(TypeBTree('!'));
    } catch (const std::runtime_error& e) {
        std::cout << "  search('!') -> " << e.what() << "\n";
    }

    printSection("forEach variadic");
    size letterCount = 0;
    bt.forEach([](BT::Entry& e, level /*lv*/, size& count) {
        if (isalpha((unsigned char)e.m_data)) ++count;
    }, letterCount);
    std::cout << "  letras en el arbol: " << letterCount << "\n";

    printSection("firstThat variadic");
    auto* entry = bt.firstThat([](BT::Entry& e, level /*lv*/, TypeBTree target) -> flag {
        return e.m_data == target;
    }, TypeBTree('M'));
    std::cout << "  firstThat('M') -> " << (entry ? "encontrado" : "no encontrado");
    if (entry) std::cout << "  ref=" << entry->m_ref;
    std::cout << "\n";

    printSection("remove");
    size beforeRemove = bt.numKeys();
    auto [removedVal, removedRef] = bt.remove(TypeBTree('A'));
    std::cout << "  remove('A') -> eliminado: valor=" << removedVal
              << "  ref="          << removedRef
              << "  size antes="   << beforeRemove
              << "  size despues=" << bt.numKeys() << "\n";

    printSection("for (auto& entry : bt) - iterador inorder");
    std::string inorderKeys;
    for (auto& e : bt) inorderKeys += e.m_data;
    std::cout << "  claves en orden: " << inorderKeys << "\n";

    printSection("useCount() - contador de accesos por clave");
    bt.search(TypeBTree('B')); bt.search(TypeBTree('B')); bt.search(TypeBTree('B'));
    bt.search(TypeBTree('C'));
    for (auto& e : bt)
        if (e.m_data == TypeBTree('B') || e.m_data == TypeBTree('C'))
            std::cout << "  '" << e.m_data << "'  useCount=" << e.useCount() << "\n";

    testIO(bt);

    testCopyMove(bt, [](BT& c) { c.insert(TypeBTree('!'), 999); });

    printSection("Concurrencia");
    BT concurrentTree;
    const size kThreads          = 5;
    const size kInsertsPerThread = 200;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (size i = 0; i < kThreads; ++i)
        threads.emplace_back(concurrencyWorker, std::ref(concurrentTree), Ref(i + 1));
    for (auto& t : threads) t.join();
    std::cout << "  inserciones concurrentes lanzadas: " << (kThreads * kInsertsPerThread) << "\n";
    std::cout << "  size final (sin corrupcion, <= 26 claves unicas): "
              << concurrentTree.numKeys() << "\n";

    printFooter("BTREE");
}

int main() {
    DemoBTree();
    return 0;
}