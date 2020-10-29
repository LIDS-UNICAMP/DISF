/**
* Dynamic and Iterative Spanning Forest (Python3)
*
* @date August, 2020
*/

//=============================================================================
// Includes
//=============================================================================
#include <Python.h>
#include <numpy/arrayobject.h>

#include "Image.h"
#include "DISF.h"

//=============================================================================
// Prototypes
//=============================================================================
void usage();
PyMODINIT_FUNC PyInit_disf(void);
static PyObject* DISF_Superpixels(PyObject* self, PyObject* args);

Graph *createGraphFromPyArray(PyObject *pyarr, int ndim, npy_intp *dims, Image **border_img);
PyObject *createPyObjectFromGrayImage(Image *img);

//=============================================================================
// Structures
//=============================================================================
static PyMethodDef methods[] = {
    { "DISF_Superpixels", DISF_Superpixels, METH_VARARGS, "Generates superpixels with the DISF algorithm" },
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef disfModule = 
{
    PyModuleDef_HEAD_INIT,
    "DISF algorithm",
    "Dynamic and Iterative Spanning Forest method for superpixel segmentation",
    -1,
    methods
};

//=============================================================================
// Methods
//=============================================================================
void usage()
{
    printf("Usage: [<a>,<b>] = DISF_Superpixels(<1>,<2>,<3>)\n");
    printf("----------------------------------\n");
    printf("INPUTS:\n");
    printf("<1> - 2D int32 grayscale/RGB numpy array\n" );
    printf("<2> - Initial number of seeds (e.g., N0 = 8000)\n");
    printf("<3> - Final number of superpixels (e.g., Nf = 50)\n");
    printf("OUTPUTS:\n");
    printf("<a> - 2D int32 label numpy array\n" );
    printf("<b> - 2D int32 border numpy array\n");
}

PyMODINIT_FUNC PyInit_disf(void)
{
    _import_array();
    return PyModule_Create(&disfModule);
}

static PyObject* DISF_Superpixels(PyObject* self, PyObject* args)
{
    int n_0, n_f,ndim;
    Image *label_img, *border_img;
    Graph *graph;
    PyObject *in_obj, *in_arr, *label_obj, *border_obj;
    npy_intp *dims;

    if(!PyArg_ParseTuple(args, "O!ii", &PyArray_Type, &in_obj, &n_0, &n_f))
    {
        usage(); return NULL;
    }

    in_arr = PyArray_FROM_OTF(in_obj, NPY_INT32, NPY_ARRAY_C_CONTIGUOUS);
    if(in_arr == NULL) return PyErr_Format(PyExc_TypeError, "Could not convert the input data to a C contiguous int32 numpy array!");

    if(n_0 <= 1) 
        return PyErr_Format(PyExc_ValueError, "N0 must be > 1!");
    if(n_f <= 1) 
        return PyErr_Format(PyExc_ValueError, "Nf must be > 1!");
    if(n_0 < n_f) 
        return PyErr_Format(PyExc_ValueError, "N0 must be >> Nf!");
    
    ndim = PyArray_NDIM(in_arr);
    dims = (npy_intp *)PyArray_DIMS(in_arr);

    if(ndim < 2 || ndim > 3) return PyErr_Format(PyExc_Exception, "The number of dimensions must be either 2 or 3!");
    if(ndim == 3 && dims[2] != 3) PyErr_Format(PyExc_Exception, "The image must be RGB-colored (i.e., 3 channels)");

    graph = createGraphFromPyArray(in_arr, ndim, dims, &border_img);

    label_img = runDISF(graph, n_0, n_f, &border_img);
    freeGraph(&graph);
    
    label_obj = createPyObjectFromGrayImage(label_img);
    border_obj = createPyObjectFromGrayImage(border_img);
    
    Py_DECREF(in_arr);
    Py_INCREF(label_obj);
    Py_INCREF(border_obj);

    freeImage(&label_img);
    freeImage(&border_img);

    return Py_BuildValue("OO",label_obj,border_obj);
}

Graph *createGraphFromPyArray(PyObject *pyarr, int ndim, npy_intp *dims, Image **border_img)
{
    int num_cols, num_rows, num_channels;
    Graph *graph;
    Image *img;

    num_rows = dims[0]; num_cols = dims[1];

    if(ndim == 2) num_channels = 1;
    else num_channels = 3;

    img = createImage(num_rows, num_cols, num_channels);
    (*border_img) = createImage(num_rows, num_cols, 1);

    #pragma omp parallel for
    for(int y = 0; y < img->num_rows; ++y)
        for(int x = 0; x < img->num_cols; ++x)
            for(int f = 0; f < img->num_channels; ++f)
                img->val[x + num_cols * y][f] = *(int*)(PyArray_GETPTR3(pyarr, y, x, f));

    graph = createGraph(img);

    freeImage(&img);

    return graph;
}

PyObject *createPyObjectFromGrayImage(Image *img)
{
    npy_intp *dims;
    PyObject *pyobj;

    dims = (npy_intp *)malloc(2 * sizeof(npy_intp));
    dims[0] = img->num_rows; dims[1] = img->num_cols;
    pyobj = (PyObject *)PyArray_SimpleNew(2, dims, PyArray_INT);

    #pragma omp parallel for
    for(int y = 0; y < img->num_rows; ++y)
        for(int x = 0; x < img->num_cols; ++x)
        {
            int *ptr = (int*)PyArray_GETPTR2(pyobj, y, x);
            *ptr = img->val[x + y*img->num_cols][0];
        }

    free(dims);

    return pyobj;
}