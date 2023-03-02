#include <utils/safe_string.h>

void safeStringCopy(char *destination, const char *source, size_t max_size) {
    size_t i = 0;
    for (; i < max_size - 1 && source[i] != '\0'; ++i) {
        destination[i] = source[i];
    }

    destination[i] = '\0';
}