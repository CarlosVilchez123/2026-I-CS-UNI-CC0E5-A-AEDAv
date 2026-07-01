#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> 

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

#endif // __TRAITS_H__