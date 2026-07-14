#include <iostream>
#include <sstream>
#include "Graphs.h"
#include "traits.h"
#include "../types.h"

using Trait = DefaultGraphTrait<>;   // Ref para ids, Type para dato, Weight para peso
using Grafo = graph::CGraph<Trait>;

// Trait con orden descendente en el peso de las aristas
using TraitDesc = DefaultGraphTrait<DefaultNodeTrait<>, DescendingEdgeTrait<>>;
using GrafoDesc = graph::CGraph<TraitDesc>;

void DemoAddNodes(Grafo& g) {
    std::cout << "== Agregando nodos ==\n";
    g.add_node(1, 100);
    g.add_node(2, 200);
    g.add_node(3, 300);
    std::cout << "node_count: " << g.node_count() << "\n";

    // Intentar agregar un nodo con id repetido: debe devolver nullptr
    if (auto* n = g.add_node(1, 999)) {
        std::cout << "ERROR: no debio insertar\n";
    } else {
        std::cout << "OK: add_node(1, ...) devolvio nullptr porque el id 1 ya existia\n";
    }
}

void DemoAddEdges(Grafo& g) {
    std::cout << "\n== Agregando aristas ==\n";
    if (auto* e = g.add_edge(10, 1, 2, 3.5)) {
        std::cout << "Arista 10 creada: " << e->source() << " -> " << e->target()
                   << " (peso " << e->weight() << ")\n";
    }
    if (auto* e = g.add_edge(11, 2, 3, 1.5)) {
        std::cout << "Arista 11 creada: " << e->source() << " -> " << e->target()
                   << " (peso " << e->weight() << ")\n";
    }

    // Intentar crear una arista hacia un nodo inexistente (99): debe fallar
    if (auto* e = g.add_edge(12, 1, 99, 2.0)) {
        std::cout << "ERROR: no debio crear arista a nodo inexistente\n";
    } else {
        std::cout << "OK: add_edge(12, 1, 99, ...) devolvio nullptr (nodo 99 no existe)\n";
    }

    std::cout << "edge_count: " << g.edge_count() << "\n";
}

void DemoRemoveNode(Grafo& g) {
    std::cout << "\n== Eliminando nodo 2 (debe arrastrar sus aristas) ==\n";
    std::cout << "Antes: node_count=" << g.node_count() << " edge_count=" << g.edge_count() << "\n";

    GraphFT ok = g.remove_node(2);
    std::cout << "remove_node(2) devolvio: " << (ok ? "true" : "false") << "\n";

    std::cout << "Despues: node_count=" << g.node_count() << " edge_count=" << g.edge_count() << "\n";

    // La arista 10 (1->2) y la arista 11 (2->3) deben haber desaparecido
    if (g.find_edge(10) == nullptr && g.find_edge(11) == nullptr) {
        std::cout << "OK: las aristas 10 y 11 fueron eliminadas junto con el nodo 2\n";
    } else {
        std::cout << "ERROR: quedaron aristas colgando\n";
    }

    // Intentar borrar un nodo que ya no existe
    GraphFT ok2 = g.remove_node(2);
    std::cout << "remove_node(2) de nuevo (ya no existe) devolvio: " << (ok2 ? "true" : "false") << "\n";
}

void DemoSortedEdges() {
    std::cout << "\n== Recorrido de aristas ordenado por peso (usa Trait::Comp) ==\n";

    Grafo gAsc;
    gAsc.add_node(1, 0);
    gAsc.add_node(2, 0);
    gAsc.add_node(3, 0);
    gAsc.add_node(4, 0);
    gAsc.add_edge(1, 1, 2, 5.0);
    gAsc.add_edge(2, 2, 3, 1.0);
    gAsc.add_edge(3, 3, 4, 3.0);

    std::cout << "Ascendente (AscendingEdgeTrait, default):\n";
    gAsc.ForEachEdgeSorted([](const auto& e) {
        std::cout << "  arista " << e.id() << ": " << e.source() << "->" << e.target()
                   << " peso=" << e.weight() << "\n";
    });

    GrafoDesc gDesc;
    gDesc.add_node(1, 0);
    gDesc.add_node(2, 0);
    gDesc.add_node(3, 0);
    gDesc.add_node(4, 0);
    gDesc.add_edge(1, 1, 2, 5.0);
    gDesc.add_edge(2, 2, 3, 1.0);
    gDesc.add_edge(3, 3, 4, 3.0);

    std::cout << "Descendente (DescendingEdgeTrait):\n";
    gDesc.ForEachEdgeSorted([](const auto& e) {
        std::cout << "  arista " << e.id() << ": " << e.source() << "->" << e.target()
                   << " peso=" << e.weight() << "\n";
    });
}

void DemoSerializacion() {
    std::cout << "\n== Serializacion (operator<< / operator>>) ==\n";

    Grafo g1;
    g1.add_node(1, 10);
    g1.add_node(2, 20);
    g1.add_node(3, 30);
    g1.add_edge(100, 1, 2, 2.5);
    g1.add_edge(101, 2, 3, 4.0);

    std::ostringstream oss;
    oss << g1;
    std::string serializado = oss.str();
    std::cout << "Serializado: " << serializado << "\n";

    Grafo g2;
    std::istringstream iss(serializado);
    iss >> g2;

    std::cout << "g2 node_count=" << g2.node_count() << " edge_count=" << g2.edge_count() << "\n";

    T3 ok = (g2.node_count() == g1.node_count()) && (g2.edge_count() == g1.edge_count());
    if (auto* n = g2.find_node(2)) {
        ok = ok && (n->data() == 20);
    } else {
        ok = false;
    }
    if (auto* e = g2.find_edge(101)) {
        ok = ok && (e->source() == 2 && e->target() == 3 && e->weight() == 4.0);
    } else {
        ok = false;
    }
    std::cout << (ok ? "OK: round-trip identico al original\n" : "ERROR: round-trip no coincide\n");
}

void DemoGraph() {
    Grafo g;
    DemoAddNodes(g);
    DemoAddEdges(g);
    DemoRemoveNode(g);
    DemoSortedEdges();
    DemoSerializacion();
}