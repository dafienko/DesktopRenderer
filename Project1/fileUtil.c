#include "assetLoader.h"
#include "errors.h"
#include <stdio.h>

#define MAX_LINE_WIDTH 500

#define FREAD_BUFFER_SIZE 10000

int get_num_lines(FILE* file) {
	int numLines = 1;

	char buffer[FREAD_BUFFER_SIZE];
	int numRead = 0;
	do {
		numRead = fread(buffer, sizeof(char), FREAD_BUFFER_SIZE, file);

		for (int i = 0; i < numRead; i++) {
			if (buffer[i] == '\n') {
				numLines++;
			}
		}
	} while (numRead == FREAD_BUFFER_SIZE);
	
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

	char buffer[FREAD_BUFFER_SIZE];
	int numRead = 0;
	do {
		memset(buffer, 0, FREAD_BUFFER_SIZE * sizeof(char));
		numRead = fread(buffer, sizeof(char), FREAD_BUFFER_SIZE, file);

		for (int i = 0; i < numRead; i++) {
			char c = buffer[i];

			*(currentLine + lineCharIndex) = c;
			lineCharIndex++;

			if (c == '\n' || (numRead != FREAD_BUFFER_SIZE && i == numRead - 1)) { // if the character is the end of a line, add this line to the lines array and reset for the next line
				if (lineCharIndex > 0) {
					char* line = calloc(lineCharIndex + 1, sizeof(char)); // add 1 for null terminator
					memcpy(line, currentLine, lineCharIndex);
					*(ld.lines + lineNum) = line;
					*(ld.lengths + lineNum) = lineCharIndex;

					lineNum++;
				}

				lineCharIndex = 0;
			}
		}
	} while (numRead == FREAD_BUFFER_SIZE);

	ld.numLines = lineNum;

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
