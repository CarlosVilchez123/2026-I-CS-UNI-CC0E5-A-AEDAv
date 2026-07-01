#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include "../types.h"
#include "BTree.h"
#include "traits.h"
using namespace std;

const TypeBTree * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const TypeBTree * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const TypeBTree * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const T1 BTreeSize = 3;

void ImprimirClave(tagObjectInfo< BTreeTrait<TypeBTree, Ref> >& info, T1 nivel) {
    cout << info.key << " ";
}

bool EsVocal(tagObjectInfo< BTreeTrait<TypeBTree, Ref> >& info, T1 nivel) {
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
    cout << "\nForEach Forward" << endl;
    bt.ForEach(bt_fwd, ImprimirClave);
    cout << endl;
}

void DemoForEachBackward(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nForEach Backward" << endl;
    bt.ForEach(bt_bwd, ImprimirClave);
    cout << endl;
}

void DemoFirstThat(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nFirstThat Forward" << endl;
    auto* encontrado = bt.FirstThat(bt_fwd, EsVocal);
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

void DemoFirstThatBackward(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nFirstThat Backward" << endl;
    auto* encontrado = bt.FirstThat(bt_bwd, EsVocal);
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

void DemoOperators(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    using MiBTree = BTree<BTreeTrait<TypeBTree, Ref>>;

    cout << "\n=== 1) operator<< a consola ===" << endl;
    cout << bt << endl;

    cout << "\n=== 2) Round-trip con stringstream ===" << endl;
    ostringstream oss;
    oss << bt;
    cout << "Serializado: " << oss.str() << endl;

    MiBTree btDesdeString(BTreeSize);
    istringstream iss(oss.str());
    iss >> btDesdeString;

    cout << "Reconstruido (ForEach Forward): ";
    btDesdeString.ForEach(bt_fwd, ImprimirClave);
    cout << endl;

    cout << "\n=== 3) Round-trip con archivo (btree.txt) ===" << endl;
    ofstream fout("btree.txt");
    fout << bt;
    fout.close();

    MiBTree btDesdeArchivo(BTreeSize);
    ifstream fin("btree.txt");
    fin >> btDesdeArchivo;
    fin.close();

    cout << "Reconstruido (ForEach Forward): ";
    btDesdeArchivo.ForEach(bt_fwd, ImprimirClave);
    cout << endl;

    cout << "\n=== 4) Verificacion de contenido ===" << endl;
    ostringstream original, reconstruidoString, reconstruidoArchivo;
    bt.ForEach              (bt_fwd, [&](tagObjectInfo<BTreeTrait<TypeBTree,Ref>>& info, int){ original           << info.key << ":" << info.ObjID << " "; });
    btDesdeString.ForEach   (bt_fwd, [&](tagObjectInfo<BTreeTrait<TypeBTree,Ref>>& info, int){ reconstruidoString << info.key << ":" << info.ObjID << " "; });
    btDesdeArchivo.ForEach  (bt_fwd, [&](tagObjectInfo<BTreeTrait<TypeBTree,Ref>>& info, int){ reconstruidoArchivo<< info.key << ":" << info.ObjID << " "; });

    b igualString  = (original.str() == reconstruidoString.str());
    b igualArchivo = (original.str() == reconstruidoArchivo.str());
    cout << "stringstream == original ? " << (igualString  ? "SI" : "NO") << endl;
    cout << "archivo      == original ? " << (igualArchivo ? "SI" : "NO") << endl;
}

void DemoBTree()
{
    BTree<BTreeTrait<TypeBTree, Ref>> bt(BTreeSize);

    DemoInsert(bt);
    DemoPrint(bt);
    DemoForEach(bt);
    DemoForEachBackward(bt);
    DemoFirstThat(bt);
    DemoFirstThatBackward(bt);
    DemoOperators(bt);
}