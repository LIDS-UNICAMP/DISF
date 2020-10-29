/**
* Color related operations
* 
* @date September, 2019
* @note Based on the implementation and theory presented in
*       http://www.easyrgb.com/en/math.php
*       http://www.brucelindbloom.com/index.html?Math.html
*       https://en.wikipedia.org/wiki/SRGB
*       https://en.wikipedia.org/wiki/CIELAB_color_space
*/
#ifndef COLOR_H
#define COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Includes
//=============================================================================
#include "Utils.h"
#include <math.h>

//=============================================================================
// Constants
//=============================================================================
static const float D65_WHITE[] = {0.950456, 1.0, 1.088754};

//=============================================================================
// Prototypes
//=============================================================================
float gammaCorr(float value); // value must be srgb norm in [0,1]
float labFunc(float value); // value must be xyz norm by D65 white

// For 8- and 16-bit, normval is 255 and 65535
float *convertGrayToLab(int* gray, int normval);
float *convertsRGBToLab(int* srgb, int normval);


#ifdef __cplusplus
}
#endif

#endif // COLOR_H
