#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional>
#include "../types.h"


template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, std::less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, std::greater<typename _Node::value_type>>{
};

template <typename Trait> struct tagObjectInfo;

template <typename _Key, typename _ObjID = long, typename _Comp = std::less<_Key>>
struct BTreeTrait {
    using keyType   = _Key;
    using ObjIDType = _ObjID;
    using Comp      = _Comp;
    using Entry     = tagObjectInfo<BTreeTrait<_Key, _ObjID, _Comp>>;
};

template <typename keyType, typename ObjIDType, typename Comp>
struct BaseBTreeTrait : public BTreeTrait<keyType, ObjIDType, Comp>
{};

template <typename keyType, typename ObjIDType = long>
struct AscendingBTreeTrait : public BaseBTreeTrait<keyType, ObjIDType, std::less<keyType>>
{};

template <typename keyType, typename ObjIDType = long>
struct DescendingBTreeTrait : public BaseBTreeTrait<keyType, ObjIDType, std::greater<keyType>>
{};

//-------------------------------------------Grafos
namespace graph {
    template <typename NodeTraits> class CNode;
    template <typename EdgeTraits> class CEdge;
}

template <typename _IdType, typename _ValueType>
struct NodeTrait {
    using id_type    = _IdType;
    using value_type = _ValueType;
};

template <typename _IdType = Ref, typename _ValueType = Type>
struct DefaultNodeTrait : public NodeTrait<_IdType, _ValueType>
{};

template <typename _IdType, typename _NodeIdType, typename _WeightType, typename _Comp>
struct BaseEdgeTrait {
    using id_type      = _IdType;
    using node_id_type = _NodeIdType;
    using weight_type  = _WeightType;
    using Comp         = _Comp;
};

template <typename _IdType = Ref, typename _NodeIdType = Ref, typename _WeightType = Weight>
struct AscendingEdgeTrait : public BaseEdgeTrait<_IdType, _NodeIdType, _WeightType, std::less<_WeightType>>
{};

template <typename _IdType = Ref, typename _NodeIdType = Ref, typename _WeightType = Weight>
struct DescendingEdgeTrait : public BaseEdgeTrait<_IdType, _NodeIdType, _WeightType, std::greater<_WeightType>>
{};

template <typename _NodeTrait, typename _EdgeTrait>
struct BaseGraphTrait {
    using Node = graph::CNode<_NodeTrait>;
    using Edge = graph::CEdge<_EdgeTrait>;
};

template <typename _NodeTrait = DefaultNodeTrait<>, typename _EdgeTrait = AscendingEdgeTrait<>>
struct DefaultGraphTrait : public BaseGraphTrait<_NodeTrait, _EdgeTrait>
{};


#endif // __TRAITS_H__