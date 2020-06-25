#include "stringUtil.h"
#include <string.h>

int strfind(const char* haystack, const char* needle) {
	int needlesFound = 0;
	
	int needleIndex = 0;
	for (int i = 0; i < strlen(haystack); i++) {
		char c = *(haystack + i);
		if (c == *(needle + needleIndex)) {
			needleIndex++;
			if (needleIndex == strlen(needle)) {
				needleIndex = 0;
				needlesFound++;
			}
		}
		else {
			needleIndex = 0;
		}
	}

	return needlesFound;
}