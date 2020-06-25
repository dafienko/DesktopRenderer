#include "stringUtil.h"
#include <stdlib.h>
#include <string.h>

SPLITSTR strsplit(const char* str, const char* split) {
    int numSplits = 0;
    int numCorrect = 0;

    int* splitIndices = calloc(strlen(str) / strlen(split), sizeof(int)); // the indices just before the start of a split occurence

    for (int i = 0; i < strlen(str); i++) {
        char c = *(str + i);
        char compare = *(split + numCorrect);
        if (c == compare) {
            numCorrect++;
            if (numCorrect == strlen(split)) {
                numCorrect = 0;
                *(splitIndices + numSplits) = i - strlen(split);
                numSplits++;

            }
        }
        else {
            numCorrect = 0;
        }
    }
    numSplits++; // numSplits is 1 more than the number of occurences of split in "str"

    char** strings = calloc(numSplits + 1, sizeof(char*)); // the last string is 0;

    for (int i = 0; i < numSplits; i++) {
        *(strings + i) = calloc(strlen(str), sizeof(char));
    }
    *(strings + numSplits) = 0;

    int indicesFound = 0;
    int currentStringLen = 0;
    for (int i = 0; i < strlen(str); ) {
        if (indicesFound > 0 && *(splitIndices + indicesFound) == i - 1 && *(splitIndices + indicesFound - 1) == i - 2 && 0) {
            *(*(strings + indicesFound) + currentStringLen) = 'b';
        }
        else {
            char c = *(str + i);
            *(*(strings + indicesFound) + currentStringLen) = c;
            currentStringLen++;
        }

        if (*(splitIndices + indicesFound) == i) {
            *(*(strings + indicesFound) + currentStringLen) = 0;
            i += strlen(split);
            currentStringLen = 0;
            indicesFound++;
            while (*(splitIndices + indicesFound) == i) {
                //*(*(strings + indicesFound) + currentStringLen) = 'b';
                *(*(strings + indicesFound) + currentStringLen) = 0;
                i += strlen(split);
                currentStringLen = 0;
                indicesFound++;
            }
            i++;
        }
        else {
            i++;
        }
    }

    free(splitIndices);

    return strings;
}


void free_splitstr(SPLITSTR* s) {
    int numFreed = 0;
    while (1) {
        char* component = *(*s + numFreed);
        numFreed++;

        if (component == 0) {
            free(component);
            break;
        }
        else {
            free(component);
        }
    }

    free((*s));
}