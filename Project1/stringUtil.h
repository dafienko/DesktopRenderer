#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#define SPLITSTR char**
#define STRIPPEDSTR char*

SPLITSTR strsplit(const char*, const char*); // returns an array of strings by splitting the first string by the second string
void free_splitstr(SPLITSTR* s);

STRIPPEDSTR lstrip(const char*); // removes all whitespace left of the first non-whitespace character in a string
void free_strippedstr(STRIPPEDSTR* s);

int strfind(const char* haystack, const char* needle);

#endif 
