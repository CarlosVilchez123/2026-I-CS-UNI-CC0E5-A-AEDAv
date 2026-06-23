#include <thread>
#include <vector>
#include <cctype>
#include "../types.h"
#include "BTree.h"
#include "traits.h"
#include "util.h"

using Trait = BTreeOrden3<TypeBTree>;
using BT    = BTree<Trait>;

static BT buildTree() {
    BT bt;
    const String keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (size i = 0; i < keys.size(); ++i)
        bt.insert(TypeBTree(keys[i]), Ref(i * i));
    return bt;
}

static void demoInsert(OStream& os) {
    os << "\n=== Insert ===\n";
    BT bt = buildTree();
    os << "size=" << bt.numKeys()
       << " | height=" << bt.height()
       << " | order=" << bt.order() << "\n";
    os << bt << "\n";
}

static void demoSearch(OStream& os) {
    os << "\n=== Search ===\n";
    BT bt = buildTree();
    try {
        auto [val, ref] = bt.search(TypeBTree('Z'));
        os << "search('Z') -> valor=" << val << "  ref=" << ref << "\n";
    } catch (const std::runtime_error& e) {
        os << "search('Z') -> " << e.what() << "\n";
    }
    try {
        bt.search(TypeBTree('!'));
    } catch (const std::runtime_error& e) {
        os << "search('!') -> " << e.what() << "\n";
    }
}

static void demoForEach(OStream& os) {
    os << "\n=== ForEach ===\n";
    BT bt = buildTree();
    os << "inorder: ";
    bt.forEach([&os](BT::Entry& e, level /*lv*/) {
        os << e.m_data << " ";
    });
    os << "\n";
    size letterCount = 0;
    bt.forEach([](BT::Entry& e, level /*lv*/, size& count) {
        if (isalpha((unsigned char)e.m_data)) ++count;
    }, letterCount);
    os << "letras: " << letterCount << "\n";
}

static void demoFirstThat(OStream& os) {
    os << "\n=== FirstThat ===\n";
    BT bt = buildTree();
    auto* e = bt.firstThat([](BT::Entry& e, level /*lv*/, TypeBTree target) -> flag {
        return e.m_data == target;
    }, TypeBTree('M'));
    os << "firstThat('M') -> " << (e ? "encontrado" : "no encontrado");
    if (e) os << "  ref=" << e->m_ref;
    os << "\n";
    auto* g = bt.firstThat([](BT::Entry& e, level /*lv*/) -> flag {
        return e.m_data > TypeBTree('Z');
    });
    os << "firstThat(> 'Z') -> " << (g ? "encontrado" : "no encontrado");
    if (g) os << "  clave=" << g->m_data << "  ref=" << g->m_ref;
    os << "\n";
}

static void demoRemove(OStream& os) {
    os << "\n=== Remove ===\n";
    BT bt = buildTree();
    size before = bt.numKeys();
    auto [val, ref] = bt.remove(TypeBTree('A'));
    os << "remove('A') -> valor=" << val << "  ref=" << ref
       << "  antes=" << before << "  despues=" << bt.numKeys() << "\n";
    try {
        bt.remove(TypeBTree('!'));
    } catch (const std::runtime_error& e) {
        os << "remove('!') -> " << e.what() << "\n";
    }
}

static void demoIterator(OStream& os) {
    os << "\n=== Iterator inorder ===\n";
    BT bt = buildTree();
    os << "range-for: ";
    String inorder;
    for (auto& e : bt) inorder += e.m_data;
    os << inorder << "\n";
}

static void demoUseCount(OStream& os) {
    os << "\n=== UseCount ===\n";
    BT bt = buildTree();
    bt.search(TypeBTree('B')); bt.search(TypeBTree('B')); bt.search(TypeBTree('B'));
    bt.search(TypeBTree('C'));
    for (auto& e : bt)
        if (e.m_data == TypeBTree('B') || e.m_data == TypeBTree('C'))
            os << "'" << e.m_data << "'  useCount=" << e.useCount() << "\n";
}

static void demoIO(OStream& os) {
    os << "\n=== operator<< / operator>> ===\n";
    BT bt = buildTree();
    OSStream oss;
    oss << bt;
    os << "serializado:   " << oss.str() << "\n";
    BT bt2;
    ISStream iss(oss.str());
    iss >> bt2;
    os << "deserializado: " << bt2 << "\n";
}

static void demoCopyMove(OStream& os) {
    os << "\n=== Copy / Move ===\n";
    BT bt = buildTree();
    BT copia(bt);
    copia.insert(TypeBTree('!'), 999);
    os << "original : size=" << bt.numKeys()    << "  " << bt    << "\n";
    os << "copia    : size=" << copia.numKeys() << "  " << copia << "\n";
    BT movida(std::move(copia));
    os << "movida   : size=" << movida.numKeys() << "  " << movida << "\n";
    os << "fuente tras move: size=" << copia.numKeys() << "\n";
}

void DemoBTree(OStream& os) {
    os << "\n=== BTree Demo ===\n";
    demoInsert     (os);
    demoSearch     (os);
    demoForEach    (os);
    demoFirstThat  (os);
    demoRemove     (os);
    demoIterator   (os);
    demoUseCount   (os);
    demoIO         (os);
    demoCopyMove   (os);
    os << "\n=== Fin BTree Demo ===\n";
}