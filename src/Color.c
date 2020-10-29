#include "Color.h"

//=============================================================================
// Float
//=============================================================================
inline float gammaCorr(float value)
{
    float new_value;

    if(value > 0.04045)
        new_value = pow(((value + 0.055) / 1.055), 2.4);
    else
        new_value = value/12.92;

    return new_value;
}

inline float labFunc(float value)
{
    float new_value;

    if(value >= 8.85645167903563082e-3)
        new_value = pow(value, 0.333333333333333);
    else
        new_value = (value * 841.0/108.0) + (4.0/29.0);

    return new_value;
}

//=============================================================================
// Float*
//=============================================================================
float *convertGrayToLab(int* gray, int normval)
{
    int *srgb;
    float *lab;

    srgb = (int*)calloc(3, sizeof(int));

    srgb[0] = srgb[1] = srgb[2] = gray[0];

    lab = convertsRGBToLab(srgb, normval);

    free(srgb);

    return lab;
}

float *convertsRGBToLab(int* srgb, int normval)
{
    float r, g, b, x, y, z;
    float *xyz, *lab;

    xyz = (float*)calloc(3, sizeof(float));

    r = gammaCorr(srgb[0] * 1.0/(float)normval);
    g = gammaCorr(srgb[1] * 1.0/(float)normval);
    b = gammaCorr(srgb[2] * 1.0/(float)normval);

    xyz[0] = r * 0.4123955889674142161 + g * 0.3575834307637148171 + b * 0.1804926473817015735;
    xyz[1] = r * 0.2125862307855955516 + g * 0.7151703037034108499 + b * 0.07220049864333622685;
    xyz[2] = r * 0.01929721549174694484 + g * 0.1191838645808485318 + b * 0.9504971251315797660;

    lab = (float*)calloc(3, sizeof(float));

    x = labFunc(xyz[0]/D65_WHITE[0]);
    y = labFunc(xyz[1]/D65_WHITE[1]);
    z = labFunc(xyz[2]/D65_WHITE[2]);

    lab[0] = (116.0 * y) - 16.0;
    lab[1] = 500.0 * (x - y);
    lab[2] = 200.0 * (y - z);

    free(xyz);

    return lab;
}