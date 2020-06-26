#include "assetLoader.h"
#include "errors.h"
#include <stdio.h>

#define MAX_LINE_WIDTH 500

int get_num_lines(FILE* file) {
	int numLines = 1;

	char c;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			numLines++;
		}
	}
	
	fseek(file, 0, SEEK_SET);
	return numLines;
}

lines_data get_file_lines(const char* filename) {
	lines_data ld;

	FILE* file;
	fopen_s(&file, filename, "rb");
	if (file == NULL) {
		check_std_err(filename, errno);
		return;
	}

	int numLines = get_num_lines(file);
	ld.lines = calloc(numLines, sizeof(char*));
	ld.lengths = calloc(numLines, sizeof(int));

	char c;
	char* currentLine = calloc(MAX_LINE_WIDTH, sizeof(char));
	int lineCharIndex = 0;
	int lineNum = 0;
	while (TRUE) {
		c = fgetc(file);
		
		if (c != EOF) { // if not at the end of the file, add this character
			*(currentLine + lineCharIndex) = c;
			lineCharIndex++;
		} 

		if (c == '\n' || c == EOF) { // if the character is the end of a line, add this line to the lines array and reset for the next line
			char* line = calloc(lineCharIndex + 1, sizeof(char)); // add 1 for null terminator
			memcpy(line, currentLine, lineCharIndex); 
			*(ld.lines + lineNum) = line;
			*(ld.lengths + lineNum) = lineCharIndex;

			lineNum++;
			lineCharIndex = 0;
		}

		if (c == EOF) { // if this is the end of the file, break out of this loop
			break;
		}
	}

	ld.numLines = numLines;

	free(currentLine);
	fclose(file);
	return ld;
}

void free_lines_data(lines_data* ld) {
	for (int i = 0; i < ld->numLines; i++) {
		free(*(ld->lines + i));
	}

	free(ld->lines);
	free(ld->lengths);
}
