#ifndef SSGXLIB_AUXILLIARY_H
#define SSGXLIB_AUXILLIARY_H

#include <cstring>
#include <string.h>

inline bool IsNonEmptyString(const char* str) {
    return (str != nullptr) && (*str != '\0');
}

#endif // SSGXLIB_AUXILLIARY_H
