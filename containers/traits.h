#ifndef __TRAITS_H__
#define __TRAITS_H__

#include <functional>
#include "../types.h"

template <typename _Node, typename _Comp>
struct BaseTrait {
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait  : BaseTrait<_Node, std::less<typename _Node::value_type>> {};

template <typename _Node>
struct DescendingTrait : BaseTrait<_Node, std::greater<typename _Node::value_type>> {};

template <typename _Key, typename _Value, typename _Comp = std::less<_Key>>
struct KVTrait {
    using Key   = _Key;
    using Value = _Value;
    using Comp  = _Comp;
};

template <typename _Value, size _Order, typename _Comp = std::less<_Value>>
struct BTreeTrait {
    using value_type            = _Value;
    using Comp                  = _Comp;
    static constexpr size Order = _Order;
};

template <typename _Value, typename _Comp = std::less<_Value>>
using BTreeOrden2 = BTreeTrait<_Value, 2, _Comp>;

template <typename _Value, typename _Comp = std::less<_Value>>
using BTreeOrden3 = BTreeTrait<_Value, 3, _Comp>;


#endif // __TRAITS_H__