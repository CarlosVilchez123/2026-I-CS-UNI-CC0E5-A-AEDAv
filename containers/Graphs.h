#pragma once

#include <concepts>
#include <type_traits>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include "../types.h"
#include "./traits.h"


namespace graph {

// Forward declarations
template<typename Trait>
class CNode;

template<typename Trait>
class CEdge;

// CNode template
template<typename Trait>
class CNode {
public:
    using traits_type = Trait;
    using id_type = typename Trait::id_type;
    using value_type = typename Trait::value_type;

    // Constructors
    explicit CNode(id_type id) : id_(id) {}
    CNode(id_type id, value_type data) : id_(id), data_(std::move(data)) {}

    // Getters
    id_type id() const noexcept { return id_; }
    value_type& data() noexcept { return data_; }
    const value_type& data() const noexcept { return data_; }

    // Setters
    void set_data(const value_type& new_data) { data_ = new_data; }

    // Possibly other methods...

    GraphFT update_data(const value_type& new_data) noexcept {
        GraphFT changed = !(data_ == new_data);
        data_ = new_data;
        return changed;
    }

private:
    id_type id_;
    value_type data_;
};

// CEdge template
template<typename Trait>
class CEdge {
public:
    using traits_type = Trait;
    using id_type = typename Trait::id_type;
    using node_id_type = typename Trait::node_id_type;
    using weight_type = typename Trait::weight_type;
    using Comp = typename Trait::Comp;

    // Constructors
    CEdge(id_type id, node_id_type src, node_id_type tgt)
        : id_(id), source_(src), target_(tgt), weight_() {}
    CEdge(id_type id, node_id_type src, node_id_type tgt, weight_type w)
        : id_(id), source_(src), target_(tgt), weight_(w) {}

    // Getters
    id_type id() const noexcept { return id_; }
    node_id_type source() const noexcept { return source_; }
    node_id_type target() const noexcept { return target_; }
    weight_type weight() const noexcept { return weight_; }

    // Setters
    void set_weight(weight_type w) noexcept { weight_ = w; }

private:
    id_type id_;
    node_id_type source_;
    node_id_type target_;
    weight_type weight_;
};

// CGraph template
template<typename Trait>
class CGraph {
public:
    // Exposed types
    using graph_traits = Trait;
    using node_type = typename Trait::Node;
    using edge_type = typename Trait::Edge;
    using node_id_type = typename node_type::id_type;
    using edge_id_type = typename edge_type::id_type;

    // Container types (can be customized via allocators later)
    using node_container = std::unordered_map<node_id_type, node_type>;
    using edge_container = std::unordered_map<edge_id_type, edge_type>;

    // Iterators
    using node_iterator = typename node_container::iterator;
    using const_node_iterator = typename node_container::const_iterator;
    using edge_iterator = typename edge_container::iterator;
    using const_edge_iterator = typename edge_container::const_iterator;

    // Constructors
    CGraph() = default;
    explicit CGraph(const node_container& nodes) : nodes_(nodes) {}
    explicit CGraph(node_container&& nodes) : nodes_(std::move(nodes)) {}

    // Rule of five (defaulted)
    ~CGraph() = default;
    CGraph(const CGraph&) = default;
    CGraph(CGraph&&) = default;
    CGraph& operator=(const CGraph&) = default;
    CGraph& operator=(CGraph&&) = default;

    // Node operations
    node_type* add_node(node_id_type id, typename node_type::value_type data = {}) {
        // Check if node already exists? Could throw or return existing.
        auto [it, inserted] = nodes_.try_emplace(id, node_type(id, std::move(data)));
        if (!inserted) {
            return nullptr;
        }
        // if not inserted, handle error (e.g., throw or return existing)
        return &it->second;
    }

    GraphFT remove_node(node_id_type id) noexcept {
        if (nodes_.erase(id) == 0) {
            return false;
        }

        // Also need to remove edges incident to this node.
        // For skeleton, just remove from nodes, leaving edges dangling.
        // Better to remove edges as well.
        for (auto it = edges_.begin(); it != edges_.end(); ) {
            if (it->second.source() == id || it->second.target() == id) {
                it = edges_.erase(it);
            }else {
                ++it;
            }
        }

        return true;
    }

    node_type* find_node(node_id_type id) noexcept {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }

    const node_type* find_node(node_id_type id) const noexcept {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }

    // Edge operations
    edge_type* add_edge(edge_id_type id, node_id_type src, node_id_type tgt,
                        typename edge_type::weight_type weight = {}) {
        // Check if nodes exist? Could throw if not.
        if (!find_node(src) || !find_node(tgt)) {
            return nullptr;
        }

        auto [it, inserted] = edges_.try_emplace(id, edge_type(id, src, tgt, weight));
        return &it->second;
    }

    GraphFT remove_edge(edge_id_type id) noexcept {
        return edges_.erase(id) > 0;
    }

    edge_type* find_edge(edge_id_type id) noexcept {
        auto it = edges_.find(id);
        return it != edges_.end() ? &it->second : nullptr;
    }

    const edge_type* find_edge(edge_id_type id) const noexcept {
        auto it = edges_.find(id);
        return it != edges_.end() ? &it->second : nullptr;
    }

    // Iterators
    node_iterator nodes_begin() noexcept { return nodes_.begin(); }
    node_iterator nodes_end() noexcept { return nodes_.end(); }
    const_node_iterator nodes_cbegin() const noexcept { return nodes_.cbegin(); }
    const_node_iterator nodes_cend() const noexcept { return nodes_.cend(); }

    edge_iterator edges_begin() noexcept { return edges_.begin(); }
    edge_iterator edges_end() noexcept { return edges_.end(); }
    const_edge_iterator edges_cbegin() const noexcept { return edges_.cbegin(); }
    const_edge_iterator edges_cend() const noexcept { return edges_.cend(); }

    // Capacity
    GraphCount node_count() const noexcept { return nodes_.size(); }
    GraphCount edge_count() const noexcept { return edges_.size(); }
    GraphFT empty() const noexcept { return nodes_.empty(); }

    // Clear
    void clear() noexcept {
        nodes_.clear();
        edges_.clear();
    }

    template<typename Func>
    void ForEachEdgeSorted(Func func) const {
        using Comp = typename edge_type::Comp;
        std::vector<const edge_type*> ordenadas;
        ordenadas.reserve(edges_.size());
        for (const auto& [id, e] : edges_) {
            ordenadas.push_back(&e);
        }
        Comp comp{};
        std::sort(ordenadas.begin(), ordenadas.end(),
                  [&comp](const edge_type* a, const edge_type* b) {
                      return comp(a->weight(), b->weight());
                  });
        for (const edge_type* e : ordenadas) {
            func(*e);
        }
    }

private:
    node_container nodes_;
    edge_container edges_;
};

template<typename Trait>
std::ostream& operator<<(std::ostream& os, const CGraph<Trait>& g) {
    os << "N{";
    GraphFT first = true;
    for (auto it = g.nodes_cbegin(); it != g.nodes_cend(); ++it) {
        if (!first) os << ",";
        os << it->second.id() << ":" << it->second.data();
        first = false;
    }
    os << "}E{";
    first = true;
    for (auto it = g.edges_cbegin(); it != g.edges_cend(); ++it) {
        if (!first) os << ",";
        os << it->second.id() << ":" << it->second.source() << "-" << it->second.target()
           << ":" << it->second.weight();
        first = false;
    }
    os << "}";
    return os;
}
 
template<typename Trait>
std::istream& operator>>(std::istream& is, CGraph<Trait>& g) {
    using node_id_type = typename CGraph<Trait>::node_id_type;
    using edge_id_type = typename CGraph<Trait>::edge_id_type;
    using value_type   = typename CGraph<Trait>::node_type::value_type;
    using weight_type  = typename CGraph<Trait>::edge_type::weight_type;
 
    g.clear();
    T3 ch;
 
    is >> ch >> ch;   // 'N' '{'
    is >> ch;         // primer caracter del contenido, o '}'
    if (ch != '}') {
        is.putback(ch);
        do {
            node_id_type id{};
            value_type data{};
            T3 sep;
            is >> id >> sep >> data;   // id : data
            g.add_node(id, data);
            is >> ch;                 // ',' o '}'
        } while (ch != '}');
    }
 
    is >> ch >> ch;   // 'E' '{'
    is >> ch;
    if (ch != '}') {
        is.putback(ch);
        do {
            edge_id_type id{};
            node_id_type src{}, tgt{};
            weight_type w{};
            T3 sep;
            is >> id >> sep >> src >> sep >> tgt >> sep >> w;   // id : src - tgt : peso
            g.add_edge(id, src, tgt, w);
            is >> ch;
        } while (ch != '}');
    }
 
    return is;
}


} // namespace graph