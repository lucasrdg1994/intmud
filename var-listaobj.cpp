/* Este programa � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da GNU General Public License V2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var-listaobj.h"
#include "variavel.h"
#include "objeto.h"
#include "instr.h"
#include "random.h"
#include "misc.h"

//#define DEBUG  // Checar listaobj e listaitem
//#define DEBUG_MSG

//----------------------------------------------------------------------------
#ifdef DEBUG
#define DEBUG1 TGrupoX::Debug();
#else
#define DEBUG1
#endif

//----------------------------------------------------------------------------
TGrupoX * TGrupoX::Disp = 0;
TGrupoX * TGrupoX::Usado = 0;
unsigned long TGrupoX::Tempo = 0;
TListaX * TListaX::EndMover = 0;
const char ListaItem1[] = { 7, 0, Instr::cListaItem, 0, 0, 0, '+', 0 };

//----------------------------------------------------------------------------
void TListaObj::Apagar()
{
    if (Objeto)
        while (Inicio)
        {
            Inicio->Objeto->MarcarApagar();
            Inicio->Apagar();
        }
    else
        while (Inicio)
            Inicio->Apagar();
    DEBUG1
}

//----------------------------------------------------------------------------
void TListaObj::Mover(TListaObj * destino)
{
    // Acerta TListaX::Lista em todos os TListaX da lista
    for (TListaX * obj = Inicio; obj; obj=obj->ListaDepois)
        obj->Lista = destino;
    move_mem(destino, this, sizeof(TListaObj));
}

//----------------------------------------------------------------------------
void TListaObj::EndObjeto(TObjeto * obj)
{
    Objeto = obj;
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddInicio(TObjeto * obj)
{
    if (obj==0)
        return 0;
// Cria objeto
    TListaX * l1 = TListaX::Criar();
// Acrescenta no topo da lista
    Total++;
    l1->Lista = this;
    l1->ListaAntes = 0;
    l1->ListaDepois = Inicio;
    if (Inicio)
        Inicio->ListaAntes = l1;
    Inicio = l1;
    if (Fim==0)
        Fim = l1;
// Acrescenta objeto
    l1->Objeto = obj;
    l1->ObjAntes = 0;
    l1->ObjDepois = obj->VarListaX;
    if (obj->VarListaX)
        obj->VarListaX->ObjAntes = l1;
    obj->VarListaX = l1;
// Acerta ListaItem
    l1->ListaItem = 0;
    DEBUG1
    return l1;
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddFim(TObjeto * obj)
{
    if (obj==0)
        return 0;
// Cria objeto
    TListaX * l1 = TListaX::Criar();
    Total++;
// Acrescenta no final da lista
    l1->Lista = this;
    l1->ListaAntes = Fim;
    l1->ListaDepois = 0;
    if (Fim)
        Fim->ListaDepois = l1;
    Fim = l1;
    if (Inicio==0)
        Inicio = l1;
// Acrescenta objeto
    l1->Objeto = obj;
    l1->ObjAntes = 0;
    l1->ObjDepois = obj->VarListaX;
    if (obj->VarListaX)
        obj->VarListaX->ObjAntes = l1;
    obj->VarListaX = l1;
// Acerta ListaItem
    l1->ListaItem = 0;
    DEBUG1
    return l1;
}

//----------------------------------------------------------------------------
int TListaObj::getValor()
{
    return (Inicio!=0);
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddLista(TVariavel * v, TListaX * litem, int tipo)
{
    TListaX * retorno = 0;
    tipo &= 7;

// Verifica adicionar com refer�ncia a litem
    if (tipo&2)
    {
        if (litem==0)
            return 0;
        assert(litem->Lista == this);
    }

// Marca objetos que ainda n�o est�o na lista
    if (tipo >= 4)
    {
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj)
                obj->MarcaLista = 1;
            else if (v1->defvar[2] == Instr::cListaObj)
                for (TListaX * l1 = v1->end_listaobj->Inicio; l1; l1=l1->ListaDepois)
                    l1->Objeto->MarcaLista = 1;
        }
        for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
            l1->Objeto->MarcaLista = 0;
    }

// Adiciona objetos
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        TListaX * lini = 0;
        TObjeto * obj;
        if (v1->defvar[2] == Instr::cListaObj)
        {
            if (v1->end_listaobj == this) // Mesma lista
                continue;
            lini = v1->end_listaobj->Inicio;
            if (lini == 0)
                continue;
            obj = lini->Objeto;
        }
        else
        {
            obj = v1->getObj();
            if (obj==0)
                continue;
        }
        while (true)
        {
            if (tipo<4 || obj->MarcaLista)
            {
                obj->MarcaLista = 0;
            // Cria objeto
                TListaX * l1 = TListaX::Criar();
                l1->Lista = this;
                l1->ListaItem = 0;
                Total++;
            // Acrescenta na lista
                switch (tipo)
                {
                case 0: // Adicionar no in�cio da lista
                case 4:
                    l1->ListaAntes = 0;
                    l1->ListaDepois = Inicio;
                    if (Inicio)
                        Inicio->ListaAntes = l1;
                    Inicio = l1;
                    if (Fim==0)
                        Fim = l1;
                    tipo=(tipo&4)+3;
                    break;
                case 1: // Adicionar no fim da lista
                case 5:
                    l1->ListaAntes = Fim;
                    l1->ListaDepois = 0;
                    if (Fim)
                        Fim->ListaDepois = l1;
                    Fim = l1;
                    if (Inicio==0)
                        Inicio = l1;
                    tipo=(tipo&4)+3;
                    break;
                case 2: // Adicionar antes de objeto
                case 6:
                    l1->ListaAntes = litem->ListaAntes;
                    l1->ListaDepois = litem;
                    (l1->ListaAntes ? l1->ListaAntes->ListaDepois :
                                      l1->Lista->Inicio) = l1;
                    litem->ListaAntes = l1;
                    tipo=(tipo&4)+3;
                    break;
                case 3: // Adicionar depois de objeto
                case 7:
                    l1->ListaAntes = litem;
                    l1->ListaDepois = litem->ListaDepois;
                    (l1->ListaDepois ? l1->ListaDepois->ListaAntes :
                                       l1->Lista->Fim) = l1;
                    litem->ListaDepois = l1;
                    break;
                }
            // Acrescenta no objeto
                l1->Objeto = obj;
                l1->ObjAntes = 0;
                l1->ObjDepois = obj->VarListaX;
                if (obj->VarListaX)
                    obj->VarListaX->ObjAntes = l1;
                obj->VarListaX = l1;
            // Acerta litem
                litem = l1;
            // Acerta retorno
                if (retorno==0)
                    retorno = l1;
                DEBUG1
            }

        // Passa para o pr�ximo objeto da lista
            if (lini==0)
                break;
            lini = lini->ListaDepois;
            if (lini==0)
                break;
            obj = lini->Objeto;
        }
    }
    return retorno;
}

//----------------------------------------------------------------------------
bool TListaObj::Func(TVariavel * v, const char * nome)
{
// Primeiro item da lista
    if (comparaZ(nome, "ini")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(Inicio);
        DEBUG1
        return true;
    }
// �ltimo item da lista
    if (comparaZ(nome, "fim")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(Fim);
        DEBUG1
        return true;
    }
// Objeto em que foi definido
    if (comparaZ(nome, "objlista")==0)
    {
        if (Objeto==0)
            return false;
        TObjeto * obj = Objeto;
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        Instr::VarAtual->setObj(obj);
        return true;
    }
// Adiciona objetos na lista
    int tipo = -1;
    if (comparaZ(nome, "addini")==0)
        tipo = 0;
    else if (comparaZ(nome, "addfim")==0)
        tipo = 1;
    else if (comparaZ(nome, "addini1")==0)
        tipo = 4;
    else if (comparaZ(nome, "addfim1")==0)
        tipo = 5;
    if (tipo >= 0)
    {
        TListaX * valor = AddLista(v, 0, tipo);
        if (valor==0)
            return false;
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(valor);
        return true;
    }
// Remove objeto da lista
    if (comparaZ(nome, "remove")==0)
    {
    // Nenhum objeto: remove objetos duplicados
        if (Instr::VarAtual < v+1)
        {
            int total=0;
            for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
                l1->Objeto->MarcaLista = 0;
            for (TListaX * l1 = Inicio; l1; )
            {
                if (l1->Objeto->MarcaLista == 0)
                {
                    l1->Objeto->MarcaLista = 1;
                    l1 = l1->ListaDepois;
                    continue;
                }
                TListaX::EndMover = l1->ListaDepois;
                l1->Apagar();
                l1 = TListaX::EndMover, total++;
            }
            DEBUG1
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(total);
        }
    // Remover um objeto
        if (Instr::VarAtual == v+1 && v[1].defvar[2] != Instr::cListaObj)
        {
            int total=0;
            TObjeto * obj = v[1].getObj();
            if (obj)
                for (TListaX * l1 = Inicio; l1; )
                {
                    if (l1->Objeto != obj)
                    {
                        l1 = l1->ListaDepois;
                        continue;
                    }
                    TListaX::EndMover = l1->ListaDepois;
                    l1->Apagar();
                    l1 = TListaX::EndMover, total++;
                }
            DEBUG1
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(total);
        }
    // Apagar v�rios objetos
            // Desmarca os objetos da lista
        for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
            l1->Objeto->MarcaLista = 0;
            // Marca os objetos que ser�o removidos
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj)
                obj->MarcaLista = 1;
            else if (v1->defvar[2] == Instr::cListaObj)
                for (TListaX * l1 = v1->end_listaobj->Inicio; l1;
                        l1=l1->ListaDepois)
                    l1->Objeto->MarcaLista = 1;
        }
            // Remove objetos
        int total=0;
        for (TListaX * l1 = Inicio; l1; )
        {
            if (l1->Objeto->MarcaLista == 0)
            {
                l1=l1->ListaDepois;
                continue;
            }
            TListaX::EndMover = l1->ListaDepois;
            l1->Apagar();
            l1 = TListaX::EndMover, total++;
        }
        DEBUG1
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(total);
    }
// Organiza objetos aleatoriamente
    if (comparaZ(nome, "rand")==0)
    {
        if (Total <= 1)
            return false;
    // Aloca mem�ria
        TListaX * lx[256];
        TListaX ** lista = (Total<=256 ? lx : new TListaX*[Total]);
    // Anota objetos
        TListaX * l1 = Inicio;
        for (unsigned int x=0; x<Total; x++,l1=l1->ListaDepois)
            lista[x] = l1;
        assert(l1==0);
    // Anota aleatoriamente na lista
        l1 = 0;
        for (unsigned int x=0; x<Total; x++)
        {
            unsigned int novo = circle_random() % (Total-x) + x;
            lista[novo]->ListaAntes = l1;
            if (l1==0)
                Inicio = lista[novo];
            else
                l1->ListaDepois = lista[novo];
            l1 = lista[novo];
            lista[novo] = lista[x];
        }
        l1->ListaDepois = 0;
        Fim = l1;
    // Desaloca mem�ria
        if (lista != lx)
            delete[] lista;
        DEBUG1
        return false;
    }
// Remove todos os objetos
    if (comparaZ(nome, "limpar")==0)
    {
        while (Inicio)
            Inicio->Apagar();
        DEBUG1
        return false;
    }
// Marca todos os objetos para exclus�o
    if (comparaZ(nome, "apagar")==0)
    {
        for (TListaX * obj = Inicio; obj; obj=obj->ListaDepois)
            obj->Objeto->MarcarApagar();
        DEBUG1
        return false;
    }
// Verifica se objeto est� na lista
    if (comparaZ(nome, "possui")==0)
    {
    // Nenhum objeto
        if (Instr::VarAtual < v+1)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
    // Um objeto
        if (Instr::VarAtual == v+1 && v[1].defvar[2] != Instr::cListaObj)
        {
            int total=0;
            TObjeto * obj = v[1].getObj();
            if (obj)
                for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
                    if (l1->Objeto==obj)
                        total++;
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(total);
        }
    // V�rios objetos
            // Desmarca os objetos da lista
        for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
            l1->Objeto->MarcaLista = 0;
            // Marca os objetos
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj)
                obj->MarcaLista = 1;
            else if (v1->defvar[2] == Instr::cListaObj)
                for (TListaX * l1 = v1->end_listaobj->Inicio; l1;
                        l1=l1->ListaDepois)
                    l1->Objeto->MarcaLista = 1;
        }
            // Conta os objetos
        int total=0;
        for (TListaX * l1 = Inicio; l1; l1=l1->ListaDepois)
            total += l1->Objeto->MarcaLista;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(total);
    }
// Quantidade de itens da lista
    if (comparaZ(nome, "total")==0)
    {
        unsigned int x = Total;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(x);
    }
    return false;
}

//----------------------------------------------------------------------------
void TListaItem::Apagar()
{
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        ListaX = 0;
    }
    DEBUG1
}

//----------------------------------------------------------------------------
void TListaItem::Mover(TListaItem * destino)
{
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    move_mem(destino, this, sizeof(TListaItem));
}

//----------------------------------------------------------------------------
int TListaItem::getValor()
{
    return (ListaX!=0);
}

//----------------------------------------------------------------------------
void TListaItem::MudarRef(TListaX * lista)
{
    if (lista==ListaX)
        return;
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        ListaX = 0;
    }
    if (lista)
    {
        ListaX = lista;
        Antes = 0;
        Depois = lista->ListaItem;
        lista->ListaItem = this;
        if (Depois)
            Depois->Antes = this;
    }
}

//----------------------------------------------------------------------------
bool TListaItem::Func(TVariavel * v, const char * nome)
{
// Quantidade de itens da lista
    if (comparaZ(nome, "total")==0)
    {
        unsigned int x = (ListaX ? ListaX->Lista->Total : 0);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(x);
    }
// Verifica se est� em alguma lista
    if (ListaX==0)
        return false;
// Objeto
    if (comparaZ(nome, "obj")==0)
    {
        if (ListaX==0)
            return false;
        TObjeto * obj = ListaX->Objeto;
        if (obj==0)
            return false;
        Instr::ApagarVar(v); // Nota: pode apagar o pr�prio listaitem
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        Instr::VarAtual->setObj(obj);
        return true;
    }
// Objeto em que a lista foi definida
    if (comparaZ(nome, "objlista")==0)
    {
        if (ListaX==0)
            return false;
        TObjeto * obj = ListaX->Lista->Objeto;
        if (obj==0)
            return false;
        Instr::ApagarVar(v); // Nota: pode apagar o pr�prio listaitem
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        Instr::VarAtual->setObj(obj);
        return true;
    }
// Vai para o objeto anterior
    if (comparaZ(nome, "antes")==0)
    {
        if (Instr::VarAtual < v+1)
            MudarRef(ListaX->ListaAntes);
        else for (int total=v[1].getInt(); total>0 && ListaX; total--)
            MudarRef(ListaX->ListaAntes);
        DEBUG1
        return false;
    }
// Vai para o pr�ximo objeto
    if (comparaZ(nome, "depois")==0)
    {
        if (Instr::VarAtual < v+1)
            MudarRef(ListaX->ListaDepois);
        else for (int total=v[1].getInt(); total>0 && ListaX; total--)
            MudarRef(ListaX->ListaDepois);
        DEBUG1
        return false;
    }
// Remove objeto da lista
    if (comparaZ(nome, "remove")==0)
    {
        ListaX->Apagar();
        DEBUG1
        return false;
    }
    if (comparaZ(nome, "removeantes")==0)
    {
        TListaX * l = ListaX;
        MudarRef(ListaX->ListaAntes);
        l->Apagar();
        DEBUG1
        return false;
    }
    if (comparaZ(nome, "removedepois")==0)
    {
        TListaX * l = ListaX;
        MudarRef(ListaX->ListaDepois);
        l->Apagar();
        DEBUG1
        return false;
    }
// Adiciona objetos na lista
    int tipo = -1;
    if (comparaZ(nome, "addantes")==0)
        tipo = 2;
    else if (comparaZ(nome, "adddepois")==0)
        tipo = 3;
    else if (comparaZ(nome, "addantes1")==0)
        tipo = 6;
    else if (comparaZ(nome, "adddepois1")==0)
        tipo = 7;
    if (tipo >= 0)
    {
        TListaX * valor = ListaX->Lista->AddLista(v, ListaX, tipo);
        if (valor==0)
            return false;
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(ListaItem1))
            return false;
        Instr::VarAtual->end_listaitem->MudarRef(valor);
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
void TListaX::Apagar()
{
// Retira de TListaObj
    Lista->Total--;
    (ListaAntes ? ListaAntes->ListaDepois : Lista->Inicio) = ListaDepois;
    (ListaDepois ? ListaDepois->ListaAntes : Lista->Fim) = ListaAntes;
// Retira de TObjeto
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarListaX) = ObjDepois;
    if (ObjDepois)
        ObjDepois->ObjAntes = ObjAntes;
// Retira de TListaItem
    while (ListaItem)
        ListaItem->MudarRef(0);
// Apaga objeto
#ifdef DEBUG_MSG
    printf("TListaX::Apagar(%p)\n", this); fflush(stdout);
#endif
    TGrupoX::Apagar(this); // Apagar com otimiza��o
    //delete this;       // Apagar sem otimiza��o
}

//----------------------------------------------------------------------------
TListaX * TListaX::Criar()
{
    TListaX * l = TGrupoX::Criar(); // Criar com otimiza��o
    //TListaX * l = new TListaX;      // Criar sem otimiza��o
#ifdef DEBUG_MSG
    printf("TListaX::Criar(%p)\n", l); fflush(stdout);
#endif
    return l;
}

//----------------------------------------------------------------------------
void TListaX::Mover(TListaX * destino)
{
#ifdef DEBUG_MSG
    printf("TListaX::Mover(%p, %p)\n", this, destino); fflush(stdout);
#endif
// Move
    memcpy(destino, this, sizeof(TListaX));
// Acerta lista
    (ListaAntes ? ListaAntes->ListaDepois : Lista->Inicio) = destino;
    (ListaDepois ? ListaDepois->ListaAntes : Lista->Fim) = destino;
// Acerta objeto
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarListaX) = destino;
    if (ObjDepois)
        ObjDepois->ObjAntes = destino;
// Acerta ListaItem
    for (TListaItem * obj = ListaItem; obj; obj=obj->Depois)
        obj->ListaX = destino;
// Acerta EndMover
    if (EndMover == this)
        EndMover = destino;
}

//----------------------------------------------------------------------------
TListaX * TGrupoX::Criar()
{
// Se tem objeto TListaX dispon�vel...
    if (Usado && Usado->Total < TOTAL_LISTAX)
        return Usado->Lista + (Usado->Total++);
// Se n�o tem objeto TListaX dispon�vel...
    TGrupoX * obj;
    if (Disp==0)    // N�o tem objeto TGrupoX dispon�vel
    {
        obj=new TGrupoX;
#ifdef DEBUG_MSG
        printf("TGrupoX::Criar(%p)\n", obj); fflush(stdout);
#endif
    }
    else            // Tem objeto TGrupoX dispon�vel
        obj=Disp, Disp=Disp->Depois; // Retira da lista Disp
    obj->Total = 1;
    obj->Depois = Usado; // Coloca na lista Usado
    Usado = obj;
    return obj->Lista;
}

//----------------------------------------------------------------------------
void TGrupoX::Apagar(TListaX * lista)
{
    TListaX * lfim = Usado->Lista + Usado->Total - 1;
    if (lista != lfim)
        lfim->Mover(lista);
    Usado->Total--;
    if (Usado->Total==0)
    {
        if (Disp==0)
            Tempo = TempoIni;
        TGrupoX * obj = Usado;
        Usado = Usado->Depois; // Retira da lista Usado
        obj->Depois = Disp;    // Coloca na lista Disp
        Disp = obj;
    }
}

//----------------------------------------------------------------------------
void TGrupoX::ProcEventos()
{
    if (Disp && Tempo+10 < TempoIni)
    {
        TGrupoX * obj = Disp;
        Disp = Disp->Depois; // Retira da lista Disp
#ifdef DEBUG_MSG
        printf("TGrupoX::Apagar(%p)\n", obj); fflush(stdout);
#endif
        delete obj;
        Tempo = TempoIni;
    }
}

//----------------------------------------------------------------------------
void TGrupoX::Debug()
{
    for (TGrupoX * gr = Disp; gr; gr=gr->Depois)
        assert(gr->Total == 0);
    for (TGrupoX * gr = Usado; gr; gr=gr->Depois)
    {
        assert(gr->Total > 0);
        assert(gr->Total <= TOTAL_LISTAX);
        for (unsigned int x=0; x<gr->Total; x++)
        {
            TListaX * obj = &gr->Lista[x];
        // Verifica ListaObj
            TListaX * obj2 = obj->Lista->Inicio;
            int encontrou=0;
            for (; obj2; obj2=obj2->ListaDepois)
            {
                if (obj2==obj)
                    encontrou++;
            // TListaX::Lista
                assert(obj2->Lista = obj->Lista);
            // TListaX::ListaAntes
                if (obj2->ListaAntes)
                    assert(obj2->ListaAntes->ListaDepois == obj2);
                else
                    assert(obj2->Lista->Inicio == obj2);
            // TListaX::ListaDepois
                if (obj2->ListaDepois)
                    assert(obj2->ListaDepois->ListaAntes == obj2);
                else
                    assert(obj2->Lista->Fim == obj2);
            }
            assert(encontrou==1);
        // Verifica Objeto
            obj2 = obj->Objeto->VarListaX;
            encontrou=0;
            for (; obj2; obj2=obj2->ObjDepois)
            {
                if (obj2==obj)
                    encontrou++;
            // TListaX::Objeto
                assert(obj2->Objeto = obj->Objeto);
            // TListaX::ObjAntes
                if (obj2->ObjAntes)
                    assert(obj2->ObjAntes->ObjDepois == obj2);
                else
                    assert(obj2->Objeto->VarListaX == obj2);
            // TListaX::ObjDepois
                if (obj2->ObjDepois)
                    assert(obj2->ObjDepois->ObjAntes == obj2);
            }
            assert(encontrou==1);
        // Verifica ListaItem
            for (TListaItem * item=obj->ListaItem; item; item=item->Depois)
            {
            // TListaItem::ListaX
                assert(item->ListaX == obj);
            // TListaItem::Antes
                if (item->Antes)
                    assert(item->Antes->Depois == item);
                else
                    assert(obj->ListaItem == item);
            // TListaItem::Depois
                if (item->Depois)
                    assert(item->Depois->Antes == item);
            }
        }
    }
}
