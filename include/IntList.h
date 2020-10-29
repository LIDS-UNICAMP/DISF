/**
* Single-Linked Integer List
*
* @date September, 2019
*/
#ifndef INTLIST_H
#define INTLIST_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Includes
//=============================================================================
#include "Utils.h"

//=============================================================================
// Structures
//=============================================================================
typedef struct IntCell
{
    int elem;
    struct IntCell* next;
} IntCell;

typedef struct
{  
    int size;
    IntCell* head;
} IntList;

//=============================================================================
// Prototypes
//=============================================================================
IntCell *createIntCell(int elem);
IntList *createIntList();
void freeIntCell(IntCell **node);
void freeIntList(IntList **list);

bool isIntListEmpty(IntList *list);
bool existsIntListElem(IntList *list, int elem);
bool insertIntListAt(IntList **list, int elem, int index); // index in [0,size[
bool insertIntListHead(IntList **list, int elem);
bool insertIntListTail(IntList **list, int elem);

int removeIntListAt(IntList **list, int index); // index in [0,size[
int removeIntListHead(IntList **list);
int removeIntListTail(IntList **list);

void removeIntListElem(IntList **list, int elem);

#ifdef __cplusplus
}
#endif

#endif // INTLIST_H
