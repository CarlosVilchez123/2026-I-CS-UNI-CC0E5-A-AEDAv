// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <utility>
#include "BTreePage.h"
#include "general_iterator.h"
#include "util.h"
#include "../types.h"


#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree 
// this is the full version of the BTree
{
       using keyType = typename Trait::keyType;
       using ObjIDType = typename Trait::ObjIDType;
       typedef CBTreePage <Trait> BTNode;// useful shorthand
       /*struct ObjectInfo codigo original
       {
               keyType first;
               long    second;
               ObjectInfo *&operator->() { return this; }
       };*/

public:
       //typedef ObjectInfo iterator;
       //typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       //typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       //typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       //typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

       using BTPage = BTNode;
       using iterator         = BTreeForwardIterator<BTree<Trait>>;
       using iteratorE = BTreeBackwardIterator<BTree<Trait>>;

public:
       BTree(T1 order = DEFAULT_BTREE_ORDER, T2 unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       T2            Insert (const keyType key, const ObjIDType ObjID);
       T2           Remove (const keyType key, const ObjIDType ObjID);
       ObjIDType       Search (const keyType key);
       Ref            size()  { return m_NumKeys; }
       Ref            height() { return m_Height;      }
       Ref            GetOrder() { return m_Order;     }

       void            Print (ostream &os);

       template <typename Func, typename... Args>
       void            ForEach( Func func, Args&&... args );
       template <typename Func, typename... Args>
       void            ReverseForEach( Func func, Args&&... args );
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( Func func, Args&&... args );
       template <typename Func, typename... Args>
       ObjectInfo*     ReverseFirstThat( Func func, Args&&... args );
       template <typename It, typename Func, typename... Args>
       ObjectInfo* BucleUnico(It it, It end, Func func, Args&&... args);


       iterator begin() { return iterator(&m_Root, false); }
       iterator end()   { return iterator(&m_Root, true);  }
       iteratorE rbegin() { return iteratorE(&m_Root, false); }
       iteratorE rend()   { return iteratorE(&m_Root, true);  }

protected:
       BTNode          m_Root;
       T1             m_Height;  // height of tree
       T1             m_Order;   // order of tree
       Ref            m_NumKeys; // number of keys
       T2            m_Unique;  // Accept the elements only once ?
};

const T1 MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(T1 order, T2 unique)
                               : m_Root(2 * order  + 1, unique),
                                 m_Height(1),
                                 m_Order(order),
                                 m_NumKeys(0),
                                 m_Unique(unique)
{
       m_Root.SetMaxKeysForChilds(order);
}

template <typename Trait>
BTree<Trait>::~BTree()
{
}

template <typename Trait>
T2 BTree<Trait>::Insert(const typename Trait::keyType key, const typename Trait::ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow )
       {
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Trait>
T2 BTree<Trait>::Remove(const typename Trait::keyType key, const typename Trait::ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if( error == bt_duplicate || error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Trait>
typename Trait::ObjIDType BTree<Trait>::Search (const typename Trait::keyType key)
{
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}
template <typename Trait>
template <typename It, typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo* BTree<Trait>::BucleUnico(It it, It end, Func func, Args&&... args)
{
       for( ; it != end ; ++it )
       {
              ObjectInfo &node = *it; // referencia, no copia
              BTreeCount level = static_cast<BTreeCount>(it.Level());
              if constexpr (std::is_void_v<std::invoke_result_t<Func, ObjectInfo&, BTreeCount, Args...>>)
                     call(func, node, level, std::forward<Args>(args)...); // void -> sigue
              else if( call(func, node, level, std::forward<Args>(args)...) )
                     return &node;                                          // valor -> para y devuelve
       }
       return nullptr;
}

template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(Func func, Args&&... args)
{
       BucleUnico(begin(), end(), func, std::forward<Args>(args)...);
}
 
template <typename Trait>
template <typename Func, typename... Args>
void BTree<Trait>::ReverseForEach(Func func, Args&&... args)
{
       BucleUnico(rbegin(), rend(), func, std::forward<Args>(args)...);
}
 
template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo * BTree<Trait>::FirstThat(Func func, Args&&... args)
{
       return BucleUnico(begin(), end(), func, std::forward<Args>(args)...);
}
 
template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo * BTree<Trait>::ReverseFirstThat(Func func, Args&&... args)
{
       return BucleUnico(rbegin(), rend(), func, std::forward<Args>(args)...);
}
 
template <typename Trait>
void BTree<Trait>::Print(ostream &os){
       m_Root.Print(os);
}

template <typename Trait>
ostream& operator<<(ostream &os, BTree<Trait> &tree)
{
       tree.Print(os);
       return os;
}

template <typename Trait>
istream& operator>>(istream &is, BTree<Trait> &tree)
{
       using node_type = typename BTree<Trait>::ObjectInfo;
       while( true )
       {
               node_type node{};
               if( !(is >> node) )
               {
                       if( is.eof() )
                               is.clear();
                       break;
               }
               tree.Insert(node.key, node.ObjID);
       }
       return is;
}

#endif