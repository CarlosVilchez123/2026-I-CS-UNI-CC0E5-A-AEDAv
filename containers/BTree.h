// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <utility>
#include "BTreePage.h"


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

public:
       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const int ObjID);
       bool            Remove (const keyType key, const int ObjID);
       ObjIDType       Search (const keyType key);
       long            size()  { return m_NumKeys; }
       long            height() { return m_Height;      }
       long            GetOrder() { return m_Order;     }

       void            Print (ostream &os);

       template <typename Func, typename... Args>
       void            ForEach( bt_IterDir dir, Func func, Args&&... args );

       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( bt_IterDir dir, Func func, Args&&... args );
       /*
       template <typename Func, typename... Args>
       void            ForEach( Func func, Args&&... args );
       template <typename Func, typename... Args>
       ObjectInfo*     FirstThat( Func func, Args&&... args );*/
       //typedef               ObjectInfo iterator;

protected:
       BTNode          m_Root;
       int             m_Height;  // height of tree
       int             m_Order;   // order of tree
       long            m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?
};

const int MaxHeight = 5;
template <typename Trait>
BTree<Trait>::BTree(int order, bool unique)
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
bool BTree<Trait>::Insert(const typename Trait::keyType key, const int ObjID)
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
bool BTree<Trait>::Remove (const typename Trait::keyType key, const int ObjID)
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
template <typename Func, typename... Args>
void BTree<Trait>::ForEach(bt_IterDir dir, Func func, Args&&... args)
{
       m_Root.ForEach(dir, func, 0, std::forward<Args>(args)...);
}

template <typename Trait>
template <typename Func, typename... Args>
typename BTree<Trait>::ObjectInfo * BTree<Trait>::FirstThat(bt_IterDir dir, Func func, Args&&... args)   //AGREGADO
{
       return m_Root.FirstThat(dir, func, 0, std::forward<Args>(args)...);
}

//operator << >>
template <typename Trait>
ostream& operator<<(ostream &os, BTree<Trait> &bt) 
{
       using ObjectInfo = typename BTree<Trait>::ObjectInfo;
       bool first = true;
       os << "{";
       bt.ForEach(bt_fwd, [&](ObjectInfo &info, int level)
       {
               if( !first )
                       os << ",";
               os << info.key << ":" << info.ObjID;
               first = false;
       });
       os << "}";
       return os;
}

//Operators
template <typename Trait>
istream& operator>>(istream &is, BTree<Trait> &bt) 
{
       char ch;
       is >> ch;               // se espera '{'
       if( ch != '{' )
       {
               is.setstate(ios::failbit);
               return is;
       }

       is >> ws;
       if( is.peek() == '}' )  // arbol vacio: "{}"
       {
               is.get();
               return is;
       }

       typename Trait::keyType   key;
       typename Trait::ObjIDType ObjID;

       while( true )
       {
               is >> key;      // lee la clave
               is >> ch;       // se espera ':'
               is >> ObjID;    // lee el ObjID

               bt.Insert(key, ObjID);

               is >> ch;       // se espera ',' o '}'
               if( ch == '}' )
                       break;
       }
       return is;
}

template <typename Trait>
void BTree<Trait>::Print(ostream &os){
       m_Root.Print(os);
}

#endif