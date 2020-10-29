/**
* Priority Queue
*
* @date September, 2019
* @note Based on the priority heap implemented by Samuel Martins
*/
#ifndef PRIOQUEUE_H
#define PRIOQUEUE_H

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
typedef enum 
{
    MAXVAL_POLICY, MINVAL_POLICY
} RemPolicy;

typedef enum
{
    // Never Inserted, Inserted, Orderly removed
    WHITE_STATE, GRAY_STATE, BLACK_STATE
} ElemState;

typedef struct 
{
    int last_elem_pos, size;
    int *pos, *node; // Position in the heap, and its element value
    double* prio; // Priority (clone)
    ElemState* state;
    RemPolicy rem_policy;
} PrioQueue;

//=============================================================================
// Prototypes
//=============================================================================
PrioQueue* createPrioQueue(int size, double *prio, RemPolicy rem_policy);
void freePrioQueue(PrioQueue **queue);

bool insertPrioQueue(PrioQueue **queue, int index);
bool isPrioQueueEmpty(PrioQueue *queue);
bool isPrioQueueFull(PrioQueue *queue);

int getFatherPos(int pos);
int getLeftSonPos(int pos);
int getRightSonPos(int pos);
int popPrioQueue(PrioQueue **queue);

// If the removal policy and the updated value are known, it is faster to use one of both
// functions below, than removing and re-inserting the element
void moveIndexDownPrioQueue(PrioQueue **queue, int index); 
void moveIndexUpPrioQueue(PrioQueue **queue, int index);   
void removePrioQueueElem(PrioQueue **queue, int index);
void resetPrioQueue(PrioQueue **queue);


#ifdef __cplusplus
}
#endif

#endif // PRIOQUEUE_H
