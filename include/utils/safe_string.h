#ifndef SAFE_STRING_H
#define SAFE_STRING_H

#include <stddef.h>

void safeStringCopy(char *destination, const char *source, size_t max_size);

#endif