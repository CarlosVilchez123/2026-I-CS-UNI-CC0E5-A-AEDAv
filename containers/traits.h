#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para less y greater

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, greater<typename _Node::value_type>>{
};

template <typename _Key, typename _ObjID = long>
struct BTreeTrait {
    using keyType = _Key;
    using ObjIDType = _ObjID;
};

template <typename keyType, typename ObjIDType, typename Comp>
struct BaseBTreeTrait : public BTreeTrait<keyType, ObjIDType>
{
    using Comparator = Comp;
};

template <typename keyType, typename ObjIDType = long>
struct AscendingBTreeTrait : public BaseBTreeTrait<keyType, ObjIDType, less<keyType>>
{};

template <typename keyType, typename ObjIDType = long>
struct DescendingBTreeTrait : public BaseBTreeTrait<keyType, ObjIDType, greater<keyType>>
{};

#endif // __TRAITS_H__