#ifndef ASSRT_H
#define ASSRT_H

#include <stdio.h>
#include <cstdarg>

inline void assrt(bool pass_condition, const char* fail_message, ...) {
    if (pass_condition) return;
    va_list args;
    va_start(args, fail_message);
    fprintf(stderr, "[LearnOpenGL] ERROR: ");
    vfprintf(stderr, fail_message, args);
    printf("\n");
    va_end(args);
}

#endif
