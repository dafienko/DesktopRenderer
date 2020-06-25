#include "assetLoader.h"
#include <stdlib.h>
#include <stdio.h>
#include "stringUtil.h"
#include <string.h>
#include "vectorMath.h"

void print_vec4i(vec4i v) {
	char* s1 = calloc(60, sizeof(char));
	sprintf_s(s1, 60, "(%i, %i, %i, %i)\n", v.x, v.y, v.z, v.w);
	OutputDebugStringA(s1);
	free(s1);
}

void print_vec3f(vec3f v) {
	char* s1 = calloc(160, sizeof(char));
	sprintf_s(s1, 160, "(%0.3f, %0.3f, %0.3f)\n", v.x, v.y, v.z);
	OutputDebugStringA(s1);
	free(s1);
}

vec4i get_face_indices(const char* faceComponent) {
	vec4i indices = { 0 };

	int numSubComponents = strfind(faceComponent, "/");
	SPLITSTR subComponents = strsplit(faceComponent, "/");

	indices.x = atoi(*(subComponents + 0)) - 1; // obj file indices start at 0, c arrays start at 0 so subtract 1 from obj index
	
	// only vertex index is guaranteed, texture/normal indices are optional
	if (numSubComponents >= 1) {
		indices.y = atoi(*(subComponents + 1)) - 1;
	}
	if (numSubComponents >= 2) {
		indices.z = atoi(*(subComponents + 2)) - 1;
	}

	free_splitstr(&subComponents);

	indices.w = -1; /* the 4th component of the index is used in get_obj_data(), it determines the "unique" index for opengl. init to -1 */

	return indices;
}

obj_data get_obj_data(const char* filename) {
	lines_data ld = get_file_lines(filename);
	obj_data od = { 0 };

	int numNormals = 0;
	int numVertices = 0;
	int numFaces = 0;
	for (int i = 0; i < ld.numLines; i++) {
		char* line = *(ld.lines + i);
		STRIPPEDSTR lstrippedLine = lstrip(line);
		SPLITSTR components = strsplit(lstrippedLine, " ");

		if (strcmp(*(components + 0), "v") == 0) {
			numVertices++;
		}
		else if (strcmp(*(components + 0), "vn") == 0) {
			numNormals++;
		}
		else if (strcmp(*(components + 0), "f") == 0) {
			if (strfind(lstrippedLine, " ") > 3) {
				numFaces += 2;
			}
			else {
				numFaces++;
			}
		}

		free_strippedstr(&lstrippedLine);
		free_splitstr(&components);
	}

	vec4i* rawIndices = calloc(numFaces * 3, sizeof(vec4i)); // 3 vec3i's per face-- each vec3i has one vertex index and each face has 3 vertices
	vec3f* rawVertices = calloc(numVertices, sizeof(vec3f));
	vec3f* rawNormals = calloc(numNormals, sizeof(vec3f));
	
	int normalIndex = 0;
	int vertexIndex = 0;
	int indexIndex = 0;
	for (int i = 0; i < ld.numLines; i++) {
		char* line = *(ld.lines + i);
		STRIPPEDSTR lstrippedLine = lstrip(line);
		SPLITSTR components = strsplit(lstrippedLine, " ");

		if (strcmp(*(components + 0), "v") == 0) {
			vec3f v = { 0 };
			
			v.x = (float)atof(*(components + 1));
			v.y = (float)atof(*(components + 2));
			v.z = (float)atof(*(components + 3));

			*(rawVertices + vertexIndex) = v;

			vertexIndex++;
		}
		else if ((strcmp(*(components + 0), "vn")) == 0) {
			vec3f n = { 0 };

			n.x = (float)atof(*(components + 1));
			n.y = (float)atof(*(components + 2));
			n.z = (float)atof(*(components + 3));

			*(rawNormals + normalIndex) = n;

			normalIndex++;
		}
		else if (strcmp(*(components + 0), "f") == 0) {
			int numSplits = strfind(lstrippedLine, " ");
			vec4i* indexComponents = calloc(numSplits, sizeof(vec4i));
			for (int j = 1; j < numSplits + 1; j++) {
				*(indexComponents + j - 1) = get_face_indices(*(components + j));
			}
			 
			if (numSplits == 3) {
				*(rawIndices + indexIndex) = *(indexComponents + 0);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 1);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 2);
				indexIndex++;
			}
			else {
				*(rawIndices + indexIndex) = *(indexComponents + 0);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 1);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 2);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 2);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 3);
				indexIndex++;

				*(rawIndices + indexIndex) = *(indexComponents + 0);
				indexIndex++;
			}
		}

		free_strippedstr(&lstrippedLine);
		free_splitstr(&components);
	}

	/* 
	obj indices are different than opengl indices. 
	
	opengl only has one index group that must correspond to every other vertex buffer data: normals, positions, etc.

	obj files have 3 index "buffers", one for normals, one for positions, and one for texture positions.

	the next several lines combine all three index buffers so there are no duplicate vertex "groups" and only one index buffer.
	*/
	(rawIndices + 0)->w = 0; 
	int uniqueIndices = 1;
	for (int i = 1; i < numFaces * 3; i++) {
		vec4i indexGroup = *(rawIndices + i);

		int foundMatch = 0;
		/* look for a clone of this index group */
		for (int j = 0; j < i; j++) {
			vec4i thisIndexGroup = *(rawIndices + j);
			
			/* if one exists, use it's opengl-compatible index */
			if (indexGroup.x == thisIndexGroup.x && indexGroup.y == thisIndexGroup.y && indexGroup.z == thisIndexGroup.z) {
				foundMatch = 1;
				(rawIndices + i)->w = thisIndexGroup.w;
				break;
			}
		}

		if (!foundMatch) {
			(rawIndices + i)->w = uniqueIndices;

			uniqueIndices++;
		}
	}

	/* 
	by this point, the w component of each raw index group contains the opengl-compatible index.

	now we need to make the actual normal/texture/position buffers that those opengl-compatible indices point to
	*/
	int* indices = calloc(numFaces * 3, sizeof(int)); // 3 indices per face
	vec3f* vertices = calloc(uniqueIndices, sizeof(vec3f));
	vec3f* normals = calloc(uniqueIndices, sizeof(vec3f));

	for (int i = 0; i < uniqueIndices; i++) {
		for (int j = 0; j < numFaces * 3; j++) {
			vec4i indexGroup = *(rawIndices + j);

			if (indexGroup.w == i) { /* find the matching opengl-compatible index and assign its corresponding pos/norm values */
				*(vertices + i) = *(rawVertices + indexGroup.x);
				/* texture coords would be the y value of indexGroup */
				*(normals + i) = *(rawNormals + indexGroup.z);

				break;
			}
		}
	}

	for (int i = 0; i < numFaces * 3; i++) {
		vec4i indexGroup = *(rawIndices + i);

		*(indices + i) = indexGroup.w;
	}

	free(rawVertices);
	free(rawNormals);
	free(rawIndices);

	od.indices = indices;
	od.normals = normals;
	od.positions = vertices;
	od.numFaces = numFaces;
	od.numIndices = uniqueIndices;

	for (int i = 0; i < od.numFaces; i++) {
		int faceIndex = i * 3;

		int i1 = *(od.indices + faceIndex + 0);
		int i2 = *(od.indices + faceIndex + 1);
		int i3 = *(od.indices + faceIndex + 2);

		vec3f v1 = *(od.positions + i1);
		vec3f v2 = *(od.positions + i2);
		vec3f v3 = *(od.positions + i3);

		int b = 2;
	}

	return od;
}

void free_obj_data(obj_data* od) {
	free(od->indices);
	free(od->normals);
	free(od->positions);
}
