#include "PrioQueue.h"

//=============================================================================
// Constructors & Deconstructors
//=============================================================================
PrioQueue* createPrioQueue(int size, double *prio, RemPolicy rem_policy)
{
    PrioQueue *queue;

    queue = (PrioQueue*)calloc(1,sizeof(PrioQueue));

    queue->size = size;
    queue->prio = prio;
    queue->state = (ElemState*)calloc(size, sizeof(ElemState));
    queue->node = (int*)calloc(size, sizeof(int));
    queue->pos = (int*)calloc(size, sizeof(int));
    queue->last_elem_pos = -1;
    queue->rem_policy = rem_policy;

    for(int i = 0; i < queue->size; i++)
    {
        queue->state[i] = WHITE_STATE;
        queue->pos[i] = -1;
        queue->node[i] = -1;
    }

    return queue;
}

void freePrioQueue(PrioQueue **queue)
{
    if(*queue != NULL)
    {
        PrioQueue *tmp;

        tmp = *queue;

        free(tmp->state);
        free(tmp->node);
        free(tmp->pos);
        free(*queue);
    }
}
//=============================================================================
// Bool
//=============================================================================
bool insertPrioQueue(PrioQueue **queue, int index)
{
    bool success;

    if(isPrioQueueFull(*queue)) 
    {
        printWarning("insertPrioQueue", "The queue is full");
        success = false;
    }
    else
    {
        PrioQueue *tmp;

        tmp = *queue;

        tmp->last_elem_pos++;
        
        tmp->node[tmp->last_elem_pos] = index;
        tmp->state[index] = GRAY_STATE; // Newly inserted
        tmp->pos[index] = tmp->last_elem_pos;

        moveIndexUpPrioQueue(queue, index);
        success = true;
    }

    return success;
}

inline bool isPrioQueueEmpty(PrioQueue *queue)
{
    return (queue->last_elem_pos == -1);
}

inline bool isPrioQueueFull(PrioQueue *queue)
{
    return (queue->last_elem_pos == queue->size - 1);
}

//=============================================================================
// Int
//=============================================================================
inline int getFatherPos(int pos)
{
    return (pos - 1)/2; 
}

inline int getLeftSonPos(int pos)
{
    return (2 * pos + 1); 
}

inline int getRightSonPos(int pos)
{
    return (2 * pos + 2); 
}

int popPrioQueue(PrioQueue **queue)
{    
    int index;
    
    if(isPrioQueueEmpty(*queue)) 
    {
        printWarning("popPrioQueue", "The queue is empty");
        index = -1;
    }
    else
    {
        PrioQueue *tmp;

        tmp = *queue;

        index = tmp->node[0]; // Aux

        tmp->pos[index] = -1;
        tmp->state[index] = BLACK_STATE; // Ordely removed

        // Puts in first and removes the last
        tmp->node[0] = tmp->node[tmp->last_elem_pos];
        tmp->pos[tmp->node[0]] = 0;
        tmp->node[tmp->last_elem_pos] = -1;
        
        tmp->last_elem_pos--;

        moveIndexDownPrioQueue(queue, tmp->node[0]);
    }

    return index;
}

//=============================================================================
// Void
//=============================================================================
void moveIndexDownPrioQueue(PrioQueue **queue, int index)
{    
    int pos, curr_pos, left_pos, right_pos, curr_index;
    float curr_prio;
    PrioQueue *tmp;

    tmp = *queue;

    if(index < tmp->size && index >= 0) 
    {
        pos = tmp->pos[index]; // Aux

        left_pos = getLeftSonPos(pos);
        right_pos = getRightSonPos(pos);

        curr_pos = pos;
        curr_index = index;
        curr_prio = tmp->prio[curr_index];

        if(left_pos <= tmp->last_elem_pos)
        {
            int left_index;
            float left_prio;

            left_index = tmp->node[left_pos];
            left_prio = tmp->prio[left_index];

            if((tmp->rem_policy == MINVAL_POLICY && left_prio < curr_prio) ||
               (tmp->rem_policy == MAXVAL_POLICY && left_prio > curr_prio))
            {
                curr_pos = left_pos;
                curr_index = left_index;
                curr_prio = left_prio;
            }
            else if(left_prio == curr_prio)
            {
                curr_pos = left_pos;
                curr_index = left_index;
                curr_prio = left_prio;
            }
        }

        if(right_pos <= tmp->last_elem_pos)
        {
            int right_index;
            float right_prio;

            right_index = tmp->node[right_pos];
            right_prio = tmp->prio[right_index];

            if((tmp->rem_policy == MINVAL_POLICY && right_prio < curr_prio) ||
               (tmp->rem_policy == MAXVAL_POLICY && right_prio > curr_prio))
            {
                curr_pos = right_pos;
                curr_index = right_index;
                curr_prio = right_prio;
            }
            else if(right_prio == curr_prio)
            {
                curr_pos = right_pos;
                curr_index = right_index;
                curr_prio = right_prio;
            }
        }

        if(curr_pos != pos)
        {
            tmp->node[curr_pos] = index;
            tmp->pos[curr_index] = pos;

            tmp->node[pos] = curr_index;
            tmp->pos[index] = curr_pos;

            moveIndexDownPrioQueue(queue, index);
        }
    }
}

void moveIndexUpPrioQueue(PrioQueue **queue, int index)
{
    int pos, curr_pos, curr_index, father_pos;
    float prio, curr_prio;
    PrioQueue *tmp;

    tmp = *queue;

    if(index < tmp->size && index >= 0) 
    {
        // Aux
        pos = tmp->pos[index];
        prio = tmp->prio[index];

        father_pos = getFatherPos(pos);

        curr_pos = pos;
        curr_index = index;
        curr_prio = prio;

        if(father_pos >= 0)
        {
            int father_index;
            float father_prio;

            father_index = tmp->node[father_pos];
            father_prio = tmp->prio[father_index];

            if((tmp->rem_policy == MINVAL_POLICY && father_prio > curr_prio) ||
               (tmp->rem_policy == MAXVAL_POLICY && father_prio < curr_prio))
            {
                curr_pos = father_pos;
                curr_index = father_index;
                curr_prio = father_prio;
            }
        }

        if(curr_pos != pos)
        {
            tmp->node[curr_pos] = index;
            tmp->pos[curr_index] = pos;

            tmp->node[pos] = curr_index;
            tmp->pos[index] = curr_pos;

            moveIndexUpPrioQueue(queue, index);
        }
    }
}
void removePrioQueueElem(PrioQueue **queue, int index)
{    
    double prio;
    PrioQueue *tmp;

    tmp = *queue;

    prio = tmp->prio[index];

    // Force the elem to be first in heap (for pop)
    if(tmp->rem_policy == MINVAL_POLICY)
        tmp->prio[index] = -INFINITY;
    else
        tmp->prio[index] = INFINITY;

    moveIndexUpPrioQueue(queue, index);
    popPrioQueue(queue); // Discard

    // Change back its original value
    tmp->prio[index] = prio;
    tmp->state[index] = WHITE_STATE; // Non-ordely removed
}

void resetPrioQueue(PrioQueue **queue)
{
    PrioQueue *tmp;

    tmp = *queue;

    for(int i = 0; i < tmp->size; i++)
    {
        tmp->state[i] = WHITE_STATE;
        tmp->pos[i] = -1;
        tmp->node[i] = -1;
    }
    tmp->last_elem_pos = -1;
}