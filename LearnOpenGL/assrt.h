#ifndef ASSRT_H
#define ASSRT_H

#include <stdio.h>

inline void assrt(bool pass_condition, const char* fail_message) {
    if (pass_condition) return;
    printf("[LearnOpenGL] {%s}\n", fail_message);
}

#endif
