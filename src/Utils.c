#include "Utils.h"

//=============================================================================
// Void
//=============================================================================
void printError(const char* function_name, const char* message, ...)
{
    va_list args;
    char full_msg[4096];

    va_start(args, message);
    vsprintf(full_msg, message, args);
    va_end(args);

    fprintf(stderr, "\nError in %s:\n%s!\n", function_name, full_msg);
    fflush(stdout);
    exit(-1);
}

void printWarning(const char *function_name, const char *message, ...)
{
    va_list args;
    char full_msg[4096];

    va_start(args, message);
    vsprintf(full_msg, message, args);
    va_end(args);

    fprintf(stdout, "\nWarning in %s:\n%s!\n", function_name, full_msg);
}