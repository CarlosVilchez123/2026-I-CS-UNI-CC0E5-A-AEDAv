#include <iostream>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include "BTree.h"
#include "traits.h"
#include "../types.h"
using namespace std;

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const TypeBTree * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const TypeBTree * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const TypeBTree * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const T1 BTreeSize = 3;

void ImprimirClave(tagObjectInfo< BTreeTrait<TypeBTree, Ref> >& info, T1 nivel) {
    cout << info.key << " ";
}

T2 EsVocal(tagObjectInfo< BTreeTrait<TypeBTree, Ref> >& info, T1 nivel) {
    TypeBTree k = info.key;
    return (k=='A'||k=='E'||k=='I'||k=='O'||k=='U' ||k=='a'||k=='e'||k=='i'||k=='o'||k=='u');
}

void DemoInsert(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    for (T1 i = 0; keys1[i]; i++)
        bt.Insert(static_cast<TypeBTree>(keys1[i]), static_cast<Ref>(i * i));
}

void DemoPrint(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    bt.Print(cout);
}

void DemoForEach(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nForEach Variac" << endl;
    bt.ForEach(ImprimirClave);
    cout << endl;
}

void DemoFirstThat(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nFirstThat" << endl;
    auto* encontrado = bt.FirstThat(EsVocal);
    if (encontrado)
    {
        cout << "Vocal encontrada: " << encontrado->key
             << " (Ref: " << encontrado->ObjID << ")" << endl;
    }
    else
    {
        cout << "No se encontraron vocales." << endl;
    }
}

void DemoReverseForEach(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nReverseForEach (descendente)" << endl;
    bt.ReverseForEach(ImprimirClave);
    cout << endl;
}

void DemoReverseFirstThat(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nReverseFirstThat" << endl;
    auto* encontrado = bt.ReverseFirstThat(EsVocal);
    if (encontrado)
    {
        cout << "Vocal encontrada (recorriendo al reves): " << encontrado->key
             << " (Ref: " << encontrado->ObjID << ")" << endl;
    }
    else
    {
        cout << "No se encontraron vocales." << endl;
    }
}

void DemoIteradorForward(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nIterador forward (begin/end, range-based for)" << endl;
    for (auto& info : bt)
        cout << info.key << " ";
    cout << endl;
}

void DemoIteradorBackward(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nIterador backward (rbegin/rend)" << endl;
    for (auto it = bt.rbegin(); it != bt.rend(); ++it)
        cout << it->key << " ";
    cout << endl;
}

void DemoComparadorPersonalizado()
{
    cout << "\nComparador personalizado (DescendingBTreeTrait)" << endl;
    BTree<DescendingBTreeTrait<TypeBTree, Ref>> btDesc(BTreeSize);
    for (T1 i = 0; keys1[i]; i++)
        btDesc.Insert(static_cast<TypeBTree>(keys1[i]), static_cast<Ref>(i * i));

    cout << "ForEach con Comp descendente: ";
    btDesc.ForEach(
        [](tagObjectInfo<DescendingBTreeTrait<TypeBTree, Ref>>& info, T1 /*nivel*/)
        {
            cout << info.key << " ";
        });
    cout << endl;
}

void DemoStreams(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nStreams (operator<< / operator>>)" << endl;

    ostringstream oss;
    oss << bt; // operator<< -> bt.Print(oss) por dentro

    BTree<BTreeTrait<TypeBTree, Ref>> btLeido(BTreeSize);
    istringstream iss(oss.str());
    iss >> btLeido; // operator>> -> lee "key ObjID" e inserta

    cout << "size original: " << bt.size()
         << ", size leido: " << btLeido.size() << endl;

    cout << "Primeras claves leidas de vuelta: ";
    T1 contador = 0;
    for (auto& info : btLeido)
    {
        if (contador++ >= 10) break;
        cout << info.key << " ";
    }
    cout << "..." << endl;
}

void DemoBTree()
{
    BTree<BTreeTrait<TypeBTree, Ref>> bt(BTreeSize);

    DemoInsert(bt);
    DemoPrint(bt);
    DemoForEach(bt);
    DemoFirstThat(bt);

    DemoReverseForEach(bt);
    DemoReverseFirstThat(bt);
    DemoIteradorForward(bt);
    DemoIteradorBackward(bt);
    DemoComparadorPersonalizado();
    DemoStreams(bt);
}