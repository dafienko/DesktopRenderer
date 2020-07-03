#include "stringUtil.h"
#include <stdlib.h>
#include <string.h>

STRIPPEDSTR lstrip(const char* src) {
    char* result = (char*)calloc(strlen(src) + 1, sizeof(char));

    int stripEnd = 0;
    for (int i = 0; i < strlen(src); i++) {
        char c = *(src + i);

        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            stripEnd = i;
            break;
        }
    }

    memcpy(result, src + stripEnd, ((strlen(src) - stripEnd) + 1) * sizeof(char));

    return result;
}

STRIPPEDSTR rstrip(const char* src) {
    if (strlen(src) > 0) {
        int lastNonWhitespaceIndex = -1;
        for (int i = 0; i < strlen(src); i++) {
            char c = *(src + i);
            if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
                lastNonWhitespaceIndex = i;
            }
        }

        unsigned int size = lastNonWhitespaceIndex + 2;
        char* result = (char*)calloc(size, sizeof(char)); // i counted from 0, add one, and then add another for the null terminator

        if (lastNonWhitespaceIndex > -1) {
            memcpy(result, src, sizeof(char) * (lastNonWhitespaceIndex + 1));
        }

        return result;
    }
    else {
        return (char*)calloc(1, sizeof(char));
    }
}

void free_strippedstr(STRIPPEDSTR* s) {
    free(*s);
}