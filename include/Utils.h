/**
* Utilities
*
* @date September, 2019
*/
#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// Includes
//=============================================================================
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
    
//=============================================================================
// Prototypes
//=============================================================================
void printError(const char* function_name, const char* message, ...); // Exits the program
void printWarning(const char* function_name, const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H