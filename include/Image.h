/**
* Image
* 
* @date September, 2019
*/
#ifndef IMAGE_H
#define IMAGE_H

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
typedef struct
{
    int num_cols, num_rows, num_channels, num_pixels;
    int **val; // Access by val[i < num_pixels][f < num_channels]
} Image;

//=============================================================================
// Prototypes
//=============================================================================
Image *createImage(int num_rows, int num_cols, int num_channels); // Zero-filled
void freeImage(Image **img);

int getMaximumValue(Image *img, int channel); // For all channels, set channel = -1
int getMinimumValue(Image *img, int channel); //
int getNormValue(Image *img); // For 8- and 16-bit, norm is 255 and 65535

#ifdef __cplusplus
}
#endif

#endif // IMAGE_H