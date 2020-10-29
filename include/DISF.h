/**
* Dynamic and Iterative Spanning Forest
* 
* @date September, 2019
*/
#ifndef DISF_H
#define DISF_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Includes
//=============================================================================
#include "Utils.h"
#include "Image.h"
#include "Color.h"
#include "IntList.h"
#include "PrioQueue.h"

//=============================================================================
// Macros
//=============================================================================
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

//=============================================================================
// Structures
//=============================================================================
typedef struct
{
    int x, y;
} NodeCoords;

typedef struct
{
    int size;
    int *dx, *dy; // Coordinate shifts in each axis
} NodeAdj;

typedef struct
{
    int root_index, num_nodes, num_feats;
    float *sum_feat;
} Tree;

typedef struct
{
    int num_cols, num_rows, num_feats, num_nodes;
    float **feats; // Access by feats[i < num_nodes][f < num_feats]
} Graph;

//=============================================================================
// Prototypes
//=============================================================================
NodeAdj *create4NeighAdj(); // 4-neighborhood
NodeAdj *create8NeighAdj(); // 8-neighborhood
Graph *createGraph(Image *img); // sRGB/Gray img --> Lab graph
Tree *createTree(int root_index, int num_feats); // root note is not inserted
void freeNodeAdj(NodeAdj **adj_rel);
void freeTree(Tree **tree);
void freeGraph(Graph **graph);

bool areValidNodeCoords(Graph *graph, NodeCoords coords);

int getNodeIndex(Graph *graph, NodeCoords coords);

double euclDistance(float *feat1, float *feat2, int num_feats); // L2-norm
double taxicabDistance(float *feat1, float *feat2, int num_feats); // L1-norm

NodeCoords getAdjacentNodeCoords(NodeAdj *adj_rel, NodeCoords coords, int id);
NodeCoords getNodeCoords(Graph *graph, int index);

float* meanTreeFeatVector(Tree *tree);

double *computeGradient(Graph *graph);

// If border_img is not desired, simply pass NULL.
Image *runDISF(Graph *graph, int n_0, int n_f, Image **border_img);

IntList *gridSampling(Graph *graph, int num_seeds);
IntList *selectKMostRelevantSeeds(Tree **trees, IntList **tree_adj, int num_nodes, int num_trees, int num_maintain);

void insertNodeInTree(Graph *graph, int index, Tree **tree);


#ifdef __cplusplus
}
#endif

#endif // DISF_H