#include "DISF.h"

//=============================================================================
// Constructors & Deconstructors
//=============================================================================
NodeAdj *create4NeighAdj()
{
    NodeAdj *adj_rel;

    adj_rel = (NodeAdj*)calloc(1, sizeof(NodeAdj));

    adj_rel->size = 4;
    adj_rel->dx = (int*)calloc(4, sizeof(int));
    adj_rel->dy = (int*)calloc(4, sizeof(int));

    adj_rel->dx[0] = -1; adj_rel->dy[0] = 0; // Left
    adj_rel->dx[1] = 1; adj_rel->dy[1] = 0; // Right

    adj_rel->dx[2] = 0; adj_rel->dy[2] = -1; // Top
    adj_rel->dx[3] = 0; adj_rel->dy[3] = 1; // Bottom

    return adj_rel;
}

NodeAdj *create8NeighAdj()
{
    NodeAdj *adj_rel;

    adj_rel = (NodeAdj*)calloc(1, sizeof(NodeAdj));

    adj_rel->size = 8;
    adj_rel->dx = (int*)calloc(8, sizeof(int));
    adj_rel->dy = (int*)calloc(8, sizeof(int));

    adj_rel->dx[0] = -1; adj_rel->dy[0] = 0; // Center-Left
    adj_rel->dx[1] = 1; adj_rel->dy[1] = 0; // Center-Right

    adj_rel->dx[2] = 0; adj_rel->dy[2] = -1; // Top-Center
    adj_rel->dx[3] = 0; adj_rel->dy[3] = 1; // Bottom-Center

    adj_rel->dx[4] = -1; adj_rel->dy[4] = 1; // Bottom-Left
    adj_rel->dx[5] = 1; adj_rel->dy[5] = -1; // Top-Right

    adj_rel->dx[6] = -1; adj_rel->dy[6] = -1; // Top-Left
    adj_rel->dx[7] = 1; adj_rel->dy[7] = 1; // Bottom-Right

    return adj_rel;
}

Graph *createGraph(Image *img)
{
    int normval;
    Graph *graph;

    normval = getNormValue(img);

    graph = (Graph*)calloc(1, sizeof(Graph));

    graph->num_cols = img->num_cols;
    graph->num_rows = img->num_rows;
    graph->num_feats = 3; // L*a*b cspace
    graph->num_nodes = img->num_pixels;

    graph->feats = (float**)calloc(graph->num_nodes, sizeof(float*));

    #pragma omp parallel for
    for(int i = 0; i < graph->num_nodes; i++)
    {
        if(img->num_channels <= 2) // Grayscale w/ w/o alpha
            graph->feats[i] = convertGrayToLab(img->val[i], normval);
        else// sRGB
            graph->feats[i] = convertsRGBToLab(img->val[i], normval);
    }

    return graph;
}
Tree *createTree(int root_index, int num_feats)
{
    Tree *tree;

    tree = (Tree*)calloc(1, sizeof(Tree));

    tree->root_index = root_index;
    tree->num_nodes = 0;
    tree->num_feats = num_feats;

    tree->sum_feat = (float*)calloc(num_feats, sizeof(float));

    return tree;
}

void freeNodeAdj(NodeAdj **adj_rel)
{
    if(*adj_rel != NULL)
    {
        NodeAdj *tmp;

        tmp = *adj_rel;

        free(tmp->dx); free(tmp->dy);
        free(tmp);

        *adj_rel = NULL;
    }
}

void freeGraph(Graph **graph)
{
    if(*graph != NULL)
    {
        Graph *tmp;

        tmp = *graph;

        for(int i = 0; i < tmp->num_nodes; i++)
            free(tmp->feats[i]);
        free(tmp->feats);
        free(tmp);

        *graph = NULL;
    }
}

void freeTree(Tree **tree)
{
    if(*tree != NULL)
    {
        Tree *tmp;

        tmp = *tree;

        free(tmp->sum_feat);
        free(tmp);

        *tree = NULL;
    }
}

//=============================================================================
// Bool
//=============================================================================
inline bool areValidNodeCoords(Graph *graph, NodeCoords coords)
{
    return (coords.x >= 0 && coords.x < graph->num_cols) &&
            (coords.y >= 0 && coords.y < graph->num_rows);
}

//=============================================================================
// Int
//=============================================================================
inline int getNodeIndex(Graph *graph, NodeCoords coords)
{
    return coords.y * graph->num_cols + coords.x;
}

//=============================================================================
// Double
//=============================================================================
inline double euclDistance(float *feat1, float *feat2, int num_feats)
{
    double dist;

    dist = 0;

    for(int i = 0; i < num_feats; i++)
        dist += (feat1[i] - feat2[i]) * (feat1[i] - feat2[i]);
    dist = sqrtf(dist);

    return dist;
}

inline double taxicabDistance(float *feat1, float *feat2, int num_feats)
{
    double dist;

    dist = 0;

    for(int i = 0; i < num_feats; i++)
        dist += fabs(feat1[i] - feat2[i]);

    return dist;
}

//=============================================================================
// NodeCoords
//=============================================================================
inline NodeCoords getAdjacentNodeCoords(NodeAdj *adj_rel, NodeCoords coords, int id)
{
    NodeCoords adj_coords;

    adj_coords.x = coords.x + adj_rel->dx[id];
    adj_coords.y = coords.y + adj_rel->dy[id];

    return adj_coords;
}


inline NodeCoords getNodeCoords(Graph *graph, int index)
{
    NodeCoords coords;

    coords.x = index % graph->num_cols;
    coords.y = index / graph->num_cols;

    return coords;
}

//=============================================================================
// Float*
//=============================================================================
inline float* meanTreeFeatVector(Tree *tree)
{
    float* mean_feat;

    mean_feat = (float*)calloc(tree->num_feats, sizeof(float));

    for(int i = 0; i < tree->num_feats; i++)
        mean_feat[i] = tree->sum_feat[i]/(float)tree->num_nodes;

    return mean_feat;
}

//=============================================================================
// Double*
//=============================================================================
double *computeGradient(Graph *graph)
{
    float max_adj_dist, sum_weight;
    float *dist_weight;
    double *grad;
    NodeAdj *adj_rel;

    grad = (double*)calloc(graph->num_nodes, sizeof(double));
    adj_rel = create8NeighAdj();

    max_adj_dist = sqrtf(2); // Diagonal distance for 8-neighborhood
    dist_weight = (float*)calloc(adj_rel->size, sizeof(float));
    sum_weight = 0;
    
    // Closer --> higher weight
    for(int i = 0; i < adj_rel->size; i++)
    {
        float div;

        div = sqrtf(adj_rel->dx[i] * adj_rel->dx[i] + adj_rel->dy[i] * adj_rel->dy[i]);
        
        dist_weight[i] = max_adj_dist / div;
        sum_weight += dist_weight[i];
    }

    for(int i = 0; i < adj_rel->size; i++)
        dist_weight[i] /= sum_weight;

    #pragma omp parallel for
    for(int i = 0; i < graph->num_nodes; i++)
    {
        float *feats;
        NodeCoords coords;

        feats = graph->feats[i];
        coords = getNodeCoords(graph, i);

        for(int j = 0; j < adj_rel->size; j++)
        {
            float *adj_feats;
            NodeCoords adj_coords;

            adj_coords = getAdjacentNodeCoords(adj_rel, coords, j);

            if(areValidNodeCoords(graph, adj_coords))
            {
                int adj_index;
                double dist;

                adj_index = getNodeIndex(graph, adj_coords);

                adj_feats = graph->feats[adj_index];

                dist = taxicabDistance(adj_feats, feats, graph->num_feats);

                grad[i] += dist * dist_weight[j];
            }            
        }
    }

    free(dist_weight);
    freeNodeAdj(&adj_rel);

    return grad;
}

//=============================================================================
// Image*
//=============================================================================
Image *runDISF(Graph *graph, int n_0, int n_f, Image **border_img)
{
    bool want_borders;
    int num_rem_seeds, iter;
    double *cost_map;
    NodeAdj *adj_rel;
    IntList *seed_set;
    Image *label_img;
    PrioQueue *queue;

    // Aux
    cost_map = (double*)calloc(graph->num_nodes, sizeof(double));
    // adj_rel = create4NeighAdj();
    adj_rel = create8NeighAdj();
    label_img = createImage(graph->num_rows, graph->num_cols, 1);
    queue = createPrioQueue(graph->num_nodes, cost_map, MINVAL_POLICY);

    want_borders = border_img != NULL;

    seed_set = gridSampling(graph, n_0);

    iter = 1; // At least a single iteration is performed
    do
    {
        int seed_label, num_trees, num_maintain;
        Tree **trees;
        IntList **tree_adj;
        bool **are_trees_adj;

        trees = (Tree**)calloc(seed_set->size, sizeof(Tree*));
        tree_adj = (IntList**)calloc(seed_set->size, sizeof(IntList*));
        are_trees_adj = (bool**)calloc(seed_set->size, sizeof(bool*));

        // Initialize values
        #pragma omp parallel for
        for(int i = 0; i < graph->num_nodes; i++)
        {
            cost_map[i] = INFINITY;
            label_img->val[i][0] = -1;

            if(want_borders)
                (*border_img)->val[i][0] = 0;
        }

        seed_label = 0;
        for(IntCell *ptr = seed_set->head; ptr != NULL; ptr = ptr->next)
        {   
            int seed_index;

            seed_index = ptr->elem;

            cost_map[seed_index] = 0;
            label_img->val[seed_index][0] = seed_label;

            trees[seed_label] = createTree(seed_index, graph->num_feats);
            tree_adj[seed_label] = createIntList();
            are_trees_adj[seed_label] = (bool*)calloc(seed_set->size, sizeof(bool));

            seed_label++;
            insertPrioQueue(&queue, seed_index);
        }

        // IFT algorithm
        while(!isPrioQueueEmpty(queue))
        {
            int node_index, node_label;
            NodeCoords node_coords;
            float *mean_feat_tree;

            node_index = popPrioQueue(&queue);
            node_coords = getNodeCoords(graph, node_index);
            node_label = label_img->val[node_index][0];

            // This node won't appear here ever again
            insertNodeInTree(graph, node_index, &(trees[node_label]));

            mean_feat_tree = meanTreeFeatVector(trees[node_label]);

            for(int i = 0; i < adj_rel->size; i++)
            {
                NodeCoords adj_coords;

                adj_coords = getAdjacentNodeCoords(adj_rel, node_coords, i);

                if(areValidNodeCoords(graph, adj_coords))
                {
                    int adj_index, adj_label;

                    adj_index = getNodeIndex(graph, adj_coords);
                    adj_label = label_img->val[adj_index][0];

                    // If it wasn't inserted nor orderly removed from the queue
                    if(queue->state[adj_index] != BLACK_STATE)
                    {
                        double arc_cost, path_cost;

                        arc_cost = euclDistance(mean_feat_tree, graph->feats[adj_index], graph->num_feats);

                        path_cost = MAX(cost_map[node_index], arc_cost);

                        if(path_cost < cost_map[adj_index])
                        {
                            cost_map[adj_index] = path_cost;
                            label_img->val[adj_index][0] = node_label;

                            if(queue->state[adj_index] == GRAY_STATE) moveIndexUpPrioQueue(&queue, adj_index);
                            else insertPrioQueue(&queue, adj_index);
                        }
                    }
                    else if(node_label != adj_label) // Their trees are adjacent
                    {
                        if(want_borders) // Both depicts a border between their superpixels
                        {
                            (*border_img)->val[node_index][0] = 255;
                            (*border_img)->val[adj_index][0] = 255;
                        }

                        if(!are_trees_adj[node_label][adj_label])
                        {
                            insertIntListTail(&(tree_adj[node_label]), adj_label);
                            insertIntListTail(&(tree_adj[adj_label]), node_label);
                            are_trees_adj[adj_label][node_label] = true;
                            are_trees_adj[node_label][adj_label] = true;
                        }
                    }
                }
            }

            free(mean_feat_tree);
        }

        num_maintain = MAX(n_0 * exp(-iter), n_f);

        // Aux
        num_trees = seed_set->size;
        freeIntList(&seed_set);

        seed_set = selectKMostRelevantSeeds(trees, tree_adj, graph->num_nodes, num_trees, num_maintain);

        num_rem_seeds = num_trees - seed_set->size;
        
        iter++;
        resetPrioQueue(&queue);

        for(int i = 0; i < num_trees; ++i)
        {
            freeTree(&(trees[i]));
            freeIntList(&(tree_adj[i]));
            free(are_trees_adj[i]);
        }
        free(trees);
        free(tree_adj);
        free(are_trees_adj);
    } while(num_rem_seeds > 0);

    free(cost_map);
    freeNodeAdj(&adj_rel);
    freeIntList(&seed_set);
    freePrioQueue(&queue);

    return label_img;
}

//=============================================================================
// IntList*
//=============================================================================
IntList *gridSampling(Graph *graph, int num_seeds)
{
    float size, stride, delta_x, delta_y;
    double *grad;
    bool *is_seed;
    IntList *seed_set;
    NodeAdj *adj_rel;

    seed_set = createIntList();
    is_seed = (bool*)calloc(graph->num_nodes, sizeof(bool));

    // Approximate superpixel size
    size = 0.5 + (float)(graph->num_nodes/(float)num_seeds);
    stride = sqrtf(size) + 0.5;

    delta_x = delta_y = stride/2.0;

    if(delta_x < 1.0 || delta_y < 1.0)
        printError("gridSampling", "The number of samples is too high");

    grad = computeGradient(graph);
    adj_rel = create8NeighAdj();

    for(int y = (int)delta_y; y < graph->num_rows; y += stride)
    {
        for(int x = (int)delta_x; x < graph->num_cols; x += stride)
        {
            int min_grad_index;
            NodeCoords curr_coords;

            curr_coords.x = x;
            curr_coords.y = y;

            min_grad_index = getNodeIndex(graph, curr_coords);

            for(int i = 0; i < adj_rel->size; i++)
            {
                NodeCoords adj_coords;

                adj_coords = getAdjacentNodeCoords(adj_rel, curr_coords, i);

                if(areValidNodeCoords(graph, adj_coords))
                {
                    int adj_index;

                    adj_index = getNodeIndex(graph, adj_coords);

                    if(grad[adj_index] < grad[min_grad_index])
                        min_grad_index = adj_index;
                }
            }

            is_seed[min_grad_index] = true;
        }
    }

    for(int i = 0; i < graph->num_nodes; i++)
        if(is_seed[i]) // Assuring unique values
            insertIntListTail(&seed_set, i);

    free(grad);
    free(is_seed);
    freeNodeAdj(&adj_rel);

    return seed_set;
}


IntList *selectKMostRelevantSeeds(Tree **trees, IntList **tree_adj, int num_nodes, int num_trees, int num_maintain)
{
    double *tree_prio;
    IntList *rel_seeds;
    PrioQueue *queue;

    tree_prio = (double*)calloc(num_trees, sizeof(double));
    rel_seeds = createIntList();
    queue = createPrioQueue(num_trees, tree_prio, MAXVAL_POLICY);

    for(int i = 0; i < num_trees; i++)
    {
        double area_prio, grad_prio;
        float *mean_feat_i;

        area_prio = trees[i]->num_nodes/(float)num_nodes;

        grad_prio = INFINITY;
        mean_feat_i = meanTreeFeatVector(trees[i]);

        for(IntCell *ptr = tree_adj[i]->head; ptr != NULL; ptr = ptr->next)
        {
            int adj_tree_id;
            float *mean_feat_j;
            double dist;

            adj_tree_id = ptr->elem;
            mean_feat_j = meanTreeFeatVector(trees[adj_tree_id]);

            dist = euclDistance(mean_feat_i, mean_feat_j, trees[i]->num_feats);

            grad_prio = MIN(grad_prio, dist);

            free(mean_feat_j);
        }

        tree_prio[i] = area_prio * grad_prio;

        insertPrioQueue(&queue, i);

        free(mean_feat_i);
    }

    for(int i = 0; i < num_maintain && !isPrioQueueEmpty(queue); i++)
    {
        int tree_id, root_index;

        tree_id = popPrioQueue(&queue);
        root_index = trees[tree_id]->root_index;

        insertIntListTail(&rel_seeds, root_index);
    }

    freePrioQueue(&queue); // The remaining are discarded
    free(tree_prio);

    return rel_seeds;
}

//=============================================================================
// Void
//=============================================================================
void insertNodeInTree(Graph *graph, int index, Tree **tree)
{
    (*tree)->num_nodes++;

    for(int i = 0; i < graph->num_feats; i++)
        (*tree)->sum_feat[i] += graph->feats[index][i];
}