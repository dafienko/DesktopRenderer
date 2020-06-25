#include "stringUtil.h"
#include <stdlib.h>
#include <string.h>

STRIPPEDSTR lstrip(const char* src) {
    char* result = (char*)calloc(strlen(src) + 1, sizeof(char));
    int rLen = 0;

    char c = 0;
    int strip = 1;
    for (int i = 0; i < strlen(src); i++) {
        c = *(src + i);
        if (strip) {
            if (c != ' ' && c != '\n' && c != '\t') {
                strip = 0;
            }
        }

        if (!strip) {
            *(result + rLen) = c;
            rLen++;
        }
    }
    return result;
}

void free_strippedstr(STRIPPEDSTR* s) {
    free(*s);
}