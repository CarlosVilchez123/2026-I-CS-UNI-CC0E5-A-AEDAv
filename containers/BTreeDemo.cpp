#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include "../types.h"
#include "BTree.h"
#include "traits.h"
using namespace std;

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
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

// ── Construye e inserta las claves de keys1 en el árbol ──────
void DemoInsert(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    for (T1 i = 0; keys1[i]; i++)
        bt.Insert(static_cast<TypeBTree>(keys1[i]), static_cast<Ref>(i * i));
}

// ── Imprime el árbol completo ─────────────────────────────────
void DemoPrint(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    bt.Print(cout);
}

// ── Recorre el árbol con ForEach imprimiendo cada clave ───────
void DemoForEach(BTree<BTreeTrait<TypeBTree, Ref>>& bt)
{
    cout << "\nForEach Variac" << endl;
    bt.ForEach(ImprimirClave);
    cout << endl;
}

// ── Busca la primera vocal con FirstThat ──────────────────────
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

// ── Orquestador ────────────────────────────────────────────────
void DemoBTree()
{
    BTree<BTreeTrait<TypeBTree, Ref>> bt(BTreeSize);

    DemoInsert(bt);
    DemoPrint(bt);
    DemoForEach(bt);
    DemoFirstThat(bt);
}