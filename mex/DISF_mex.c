/**
* Dynamic and Iterative Spanning Forest (MEX)
*
* @date August, 2020
*/

//=============================================================================
// Includes
//=============================================================================
#include <mex.h>

#include <time.h>

#include "Color.h"
#include "Image.h"
#include "DISF.h"

//=============================================================================
// Prototypes
//=============================================================================
void usage();
Graph *createGraphFromMexArray(const mxArray *mxarray, int ndims, const mwSize *dims, Image **border_img);
mxArray *createMexArrayFromGrayImage(Image *img);

//=============================================================================
// Main
//=============================================================================
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int n_0, n_f, ndims;
    Image *border_img, *label_img;
    Graph *graph;
    const mwSize *dims;

    if(nrhs != 3) usage();

    dims = mxGetDimensions(prhs[0]);
    ndims = mxGetNumberOfDimensions(prhs[0]);
    
    if(ndims < 2 || ndims > 3) mexErrMsgIdAndTxt("DISF_Superpixels","The 2D image array must be grayscale or RGB-colored (i.e., 2 or 3 dimensions)");

    if(!mxIsInt32(prhs[0])) mexErrMsgIdAndTxt("DISF_Superpixels","The 2D image array must be int32!");

    n_0 = mxGetScalar(prhs[1]);
    n_f = mxGetScalar(prhs[2]);

    if(n_0 <= 1) 
        mexErrMsgIdAndTxt("DISF_Superpixels","N0 must be > 1!");
    if(n_f <= 1) 
        mexErrMsgIdAndTxt("DISF_Superpixels","Nf must be > 1!");
    if(n_0 < n_f) 
        mexErrMsgIdAndTxt("DISF_Superpixels","N0 must be >> Nf!");

    graph = createGraphFromMexArray(prhs[0], ndims, dims, &border_img);

    label_img = runDISF(graph, n_0, n_f, &border_img);
    freeGraph(&graph);
    
    plhs[0] = createMexArrayFromGrayImage(label_img);
    plhs[1] = createMexArrayFromGrayImage(border_img);

    freeImage(&label_img);
    freeImage(&border_img);
}

//=============================================================================
// Methods
//=============================================================================
void usage()
{
    mexPrintf("Usage: [<a>,<b>] = DISF_Superpixels(<1>,<2>,<3>)\n");
    mexPrintf("----------------------------------\n");
    mexPrintf("INPUTS:\n");
    mexPrintf("<1> - 2D int32 grayscale/RGB array\n" );
    mexPrintf("<2> - Initial number of seeds (e.g., N0 = 8000)\n");
    mexPrintf("<3> - Final number of superpixels (e.g., Nf = 50)\n");
    mexPrintf("OUTPUTS:\n");
    mexPrintf("<a> - 2D int32 label map\n" );
    mexPrintf("<b> - 2D int32 border map\n");
    mexErrMsgIdAndTxt("DISF_Superpixels","Too few/many parameters!");
}

Graph *createGraphFromMexArray(const mxArray *mxarray, int ndims, const mwSize *dims, Image **border_img)
{
    int num_rows, num_cols, num_channels;
    int *in_data;
    Graph *graph;
    Image *img;

    num_rows = (int)dims[0]; 
    num_cols = (int)dims[1];

    if(ndims == 2) num_channels = 1;
    else if(ndims == 3) num_channels = (int)dims[2];

    if(num_channels != 1 && num_channels != 3)
        mexErrMsgIdAndTxt("DISF_Superpixels","The 2D image array must be grayscale or RGB-colored (i.e., 1 or 3 channels)");

    in_data = (int*)mxGetData(mxarray);
    img = createImage(num_rows, num_cols, num_channels);
    *(border_img) = createImage(num_rows, num_cols, 1);

    #pragma omp parallel for
    for(int x = 0; x < num_cols; ++x)
        for(int y = 0; y < num_rows; ++y)
        {
            int my_index;

            my_index = x + num_cols * y;

            for(int f = 0; f < num_channels; ++f)
            {
                int matlab_index = y + x * num_rows + f * num_rows * num_cols;
                img->val[my_index][f] = (int)in_data[matlab_index];
            }
        }

    graph = createGraph(img);

    freeImage(&img);

    return graph;
}

mxArray *createMexArrayFromGrayImage(Image *img)
{
    int normvalue;
    int *mx_data;
    mwSize *out_dims;
    mxArray *mxarray;

    out_dims = (mwSize*)malloc(2 * sizeof(mwSize));
    out_dims[0] = img->num_rows; out_dims[1] = img->num_cols;    

    // Writing the output
    mxarray = mxCreateNumericArray(2, out_dims, mxINT32_CLASS, mxREAL);
    mx_data = (int*)mxGetData(mxarray);

    #pragma omp parallel for
    for(int x = 0; x < img->num_cols; ++x)
        for(int y = 0; y < img->num_rows; ++y)
        {
            int my_index = y * img->num_cols + x;
            int matlab_index = x * img->num_rows + y;

            mx_data[matlab_index] = img->val[my_index][0];
        }

    free(out_dims);

    return mxarray;
}