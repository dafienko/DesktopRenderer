#include "assetLoader.h"
#include <stdlib.h>
#include <stdio.h>
#include "stringUtil.h"
#include <string.h>
#include "vectorMath.h"
#include "errors.h"

int get_material_index(mtllib* mlib, const char* materialName) {
	for (int i = 0; i < mlib->numMaterials; i++) {
		char* thisMtlName = (mlib->materials + i)->materialName;
		if (strcmp(materialName, thisMtlName) == 0) {
			return i;
		}
	}

	return 0;
}

/* for debugging file loading */
void print_obj_data(obj_data od) {
	int bufferSize = 500;
	char* buffer = calloc(bufferSize, sizeof(char));
	for (int i = 0; i < od.numUniqueIndices; i++) {
		memset(buffer, 0, bufferSize * sizeof(char));
		vec3f v = *(od.positions + i);
		vec3f n = *(od.normals + i);
		sprintf_s(buffer, bufferSize * sizeof(char), "%i: \tv: (%0.2f, %0.2f, %0.2f)       n: (%0.2f, %0.2f, %0.2f)\n", i, v.x, v.y, v.z, n.x, n.y, n.z);
		OutputDebugStringA(buffer);
	}

	OutputDebugStringA("\n");

	for (int i = 0; i < od.numMaterials; i++) {
		memset(buffer, 0, bufferSize * sizeof(char));
		int materialFloor = *(od.materialBounds + i);
		int materialCeil = *(od.materialBounds + i + 1);

		sprintf_s(buffer, bufferSize * sizeof(char), "\nmaterial %i (%i-%i):\n", i, materialFloor, materialCeil);
		OutputDebugStringA(buffer);

		for (int j = materialFloor / 3; j < materialCeil / 3; j++) {
			memset(buffer, 0, bufferSize * sizeof(char));
			int a = *(od.indices + j * 3 + 0);
			int b = *(od.indices + j * 3 + 1);
			int c = *(od.indices + j * 3 + 2);
			sprintf_s(buffer, bufferSize * sizeof(char), "\t%i %i %i\n", a, b, c);
			OutputDebugStringA(buffer);
		}
	}

	OutputDebugStringA("\n");

	free(buffer);
}

vec3f parse_vertex_position(SPLITSTR components) {
	vec3f v = { 0 };

	v.x = atof(*(components + 1));
	v.y = atof(*(components + 2));
	v.z = atof(*(components + 3));

	return v;
}

vec3f parse_vertex_normal(SPLITSTR components) {
	vec3f n = { 0 };

	n.x = atof(*(components + 1));
	n.y = atof(*(components + 2));
	n.z = atof(*(components + 3));

	return n;
}

vec4i parse_index_group(const char* faceComponent) {
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

obj_data read_obj_file(const char* filename, const char* mtlFilename) {
	mtllib mlib = { 0 };
	mlib.numMaterials = 1;
	if (mtlFilename) {
		mlib = read_mtl_file(mtlFilename);
	}
	else {
		mlib.materials = calloc(1, sizeof(mtllib_material));
		mlib.materials->materialName = calloc(1, sizeof(char));
		mlib.materials->material.ambient = (vec3f){.1f, .1f, .1f};
		mlib.materials->material.diffuse = (vec3f){ 0, 0, 1 };
		mlib.materials->material.specular = (vec3f){ .6, .6, 1 };
	}

	lines_data ld = get_file_lines(filename);

	int numPositions = 0;
	int numPositionsSpace = 100;
	vec3f* positions = calloc(numPositionsSpace, sizeof(vec3f));

	int numNormals = 0;
	int numNormalsSpace = 100;
	vec3f* normals = calloc(numNormalsSpace, sizeof(vec3f));

	int numIndexGroups = 0;
	int numIndexGroupsSpace = 100;
	vec4i* indexGroups = calloc(numIndexGroupsSpace, sizeof(vec4i));

	int numGroupBounds = 1;
	int numGroupBoundsSpace = 2;
	vec4i* groupBounds = calloc(numGroupBoundsSpace, sizeof(vec4i)); // x = position bound, y = tex coord bound, z = normal bound, w = indexGroup bound
	*(groupBounds + 0) = (vec4i){0, 0, 0, 0}; /* the first object "group's" indices will always start at 0p, 0tc, 0n, 0igb */

	int numMaterialGroupBounds = 1;
	int numMaterialGroupBoundsSpace = 2;
	vec2i* materialGroupBounds = calloc(numMaterialGroupBoundsSpace, sizeof(vec2i));
	*(materialGroupBounds + 0) = (vec2i){0, 0};

	int vertexIndex = 0; // current vertex/normal position into corresponding raw obj storage buffer
	int normalIndex = 0;
	int indexGroupIndex = 0;
	for (int i = 0; i < ld.numLines; i++) {
		char* line = *(ld.lines + i);
		STRIPPEDSTR lstripped = lstrip(line);
		STRIPPEDSTR rstripped = rstrip(lstripped);
		SPLITSTR components = strsplit(rstripped, " ");

		char* firstToken = *(components + 0);
		if (strcmp(firstToken, "v") == 0) {
			if (numPositions + 1 >= numPositionsSpace) {
				numPositionsSpace *= 2;
				positions = realloc(positions, numPositionsSpace * sizeof(vec3f));
			}

			vec3f v = parse_vertex_position(components);
			*(positions + numPositions) = v;

			numPositions++;
		}
		else if (strcmp(firstToken, "vn") == 0) {
			if (numNormals + 1 >= numNormalsSpace) {
				numNormalsSpace *= 2;
				normals = realloc(normals, numNormalsSpace * sizeof(vec3f));
			}

			vec3f n = parse_vertex_normal(components);
			*(normals + numNormals) = n;

			numNormals++;
		}
		else if (strcmp(firstToken, "f") == 0) {
			int numSplits = strfind(rstripped, " "); // useful for determining if this face is just one tri or a quad. (Obj spec doesn't mandate tris-only)
			int thisNumIndexGroups = (numSplits == 3 ? 3 : 6);

			if (numIndexGroups + thisNumIndexGroups + 1 >= numIndexGroupsSpace) {
				numIndexGroupsSpace *= 2;
				indexGroups = realloc(indexGroups, numIndexGroupsSpace * sizeof(vec4i));
			}

			vec4i* indexComponents = calloc(numSplits, sizeof(vec4i));
			for (int j = 1; j < numSplits + 1; j++) {
				*(indexComponents + j - 1) = parse_index_group(*(components + j));
			}

			for (int i = 0; i < 3; i++) { // regardless of whether or not this face is a tri, the first three index groups will be one trie
				*(indexGroups + numIndexGroups) = *(indexComponents + i);
				numIndexGroups++;
			}
			if (numSplits != 3) { // if this face is a quad, add second tri to finish the quad
				*(indexGroups + numIndexGroups) = *(indexComponents + 2);
				numIndexGroups++;

				*(indexGroups + numIndexGroups) = *(indexComponents + 3);
				numIndexGroups++;

				*(indexGroups + numIndexGroups) = *(indexComponents + 0);
				numIndexGroups++;
			}
		}

		if (strcmp(firstToken, "g") == 0 || i == ld.numLines - 1) { // we're starting a new group or on the last line (which means we're ending the last group)
			if (numGroupBounds + 1 >= numGroupBoundsSpace) {
				numGroupBoundsSpace *= 2;
				groupBounds = realloc(groupBounds, numGroupBoundsSpace * sizeof(vec4i));
			}

			vec4i thisGroupBound = (vec4i){numPositions, 0, numNormals, numIndexGroups};
			*(groupBounds + numGroupBounds) = thisGroupBound;
			numGroupBounds++;
		} 

		if (strcmp(firstToken, "usemtl") == 0 || i == ld.numLines - 1) { // we're using a new material or on the last line (which means we're ending the last material)
			if (numMaterialGroupBounds + 1 >= numMaterialGroupBoundsSpace) {
				numMaterialGroupBoundsSpace *= 2;
				materialGroupBounds = realloc(materialGroupBounds, numMaterialGroupBoundsSpace * sizeof(vec2i));
			}

			int materialIndex = 0;
			if (mtlFilename) {
				char* materialName = *(components + 1);
				if (materialName != NULL) {
					materialIndex = get_material_index(&mlib, *(components + 1));
				}
			}
			vec2i thisMaterialGroupBound = (vec2i){materialIndex, numIndexGroups};
			*(materialGroupBounds + numMaterialGroupBounds) = thisMaterialGroupBound;
			numMaterialGroupBounds++;
		}

		free_splitstr(&components);
		free_strippedstr(&lstripped);
		free_strippedstr(&rstripped);
	}

	materialGroupBounds = realloc(materialGroupBounds, numMaterialGroupBounds * sizeof(vec2i));
	groupBounds = realloc(groupBounds, numGroupBounds * sizeof(vec4i));
	indexGroups = realloc(indexGroups, numIndexGroups * sizeof(vec4i));
	positions = realloc(positions, numPositions * sizeof(vec3f));
	normals = realloc(normals, numNormals * sizeof(vec3f));

	int numIndicesUsed = 0;
	int numIndicesSpace = 100;
	vec3f* positionsFiltered = calloc(numIndicesSpace, sizeof(vec3f));
	vec3f* normalsFiltered = calloc(numIndicesSpace, sizeof(vec3f));
	

	int* materialGroupNumIndices = calloc(mlib.numMaterials, sizeof(int));
	memset(materialGroupNumIndices, 0, mlib.numMaterials * sizeof(int));
	int initialSpace = 100;
	int* materialGroupSpace = calloc(mlib.numMaterials, sizeof(int));
	int** materialGroups = calloc(mlib.numMaterials, sizeof(int*));
	for (int i = 0; i < mlib.numMaterials; i++) {
		*(materialGroupSpace + i) = initialSpace;
		*(materialGroups + i) = calloc(*(materialGroupSpace + i), sizeof(int));
	}

	int numUniqueIndices = -1;
	for (int i = 0; i < numGroupBounds - 1; i++) {
		vec4i groupFloorBound = *(groupBounds + i);
		vec4i groupCeilBound = *(groupBounds + i + 1);
		
		int currentMaterialIndex = 0;
		if (mtlFilename) {
			for (int i = 0; i < numMaterialGroupBounds; i++) {
				vec2i mgroupBound = *(materialGroupBounds + i);
				
				if (mgroupBound.y >= groupCeilBound.w) {
					break;
				}

				currentMaterialIndex = mgroupBound.x;
			}
		}

		for (int j = groupFloorBound.w; j < groupCeilBound.w; j++) {
			vec4i thisIndexGroup = *(indexGroups + j); // the w of each index group stores the unique opengl buffer index for that index group
			int uniqueIndex = -1;
			for (int k = j - 1; k >= groupFloorBound.w; k--) {
				vec4i prevIndexGroup = *(indexGroups + k);
				if (thisIndexGroup.x == prevIndexGroup.x && thisIndexGroup.y == prevIndexGroup.y && thisIndexGroup.z == prevIndexGroup.z) {
					uniqueIndex = prevIndexGroup.w;
					break;
				}
			}

			if (uniqueIndex == -1) { // the current index group has no match in all previous index groups-- add to p/tc/n buffer
				uniqueIndex = ++numUniqueIndices;

				if (uniqueIndex >= numIndicesSpace) {
					numIndicesSpace *= 2;

					positionsFiltered = realloc(positionsFiltered, numIndicesSpace * sizeof(vec3f));
					normalsFiltered = realloc(normalsFiltered, numIndicesSpace * sizeof(vec3f));
				}

				*(positionsFiltered + uniqueIndex) = *(positions + thisIndexGroup.x);
				*(normalsFiltered + uniqueIndex) = *(normals + thisIndexGroup.z);
			}
			(indexGroups + j)->w = uniqueIndex;

			if (currentMaterialIndex >= mlib.numMaterials) {
				int x = 0;
			}
			if (*(materialGroupNumIndices + currentMaterialIndex) + 1 >= *(materialGroupSpace + currentMaterialIndex)) {
				*(materialGroupSpace + currentMaterialIndex) *= 2;
				*(materialGroups + currentMaterialIndex) = realloc(*(materialGroups + currentMaterialIndex), *(materialGroupSpace + currentMaterialIndex) * sizeof(int));
			}

			int* materialGroupIndexBuffer = *(materialGroups + currentMaterialIndex);
			*(materialGroupIndexBuffer + *(materialGroupNumIndices + currentMaterialIndex)) = uniqueIndex;
			*(materialGroupNumIndices + currentMaterialIndex) += 1;
			
		}
	}
	free(materialGroupSpace);
	free(positions);
	free(normals);
	free(indexGroups);
	free(groupBounds);
	free(materialGroupBounds);

	/* calculate the total index buffer size and save the material group index bounds */
	int totalBufferSize = 0;
	int* materialIndexBounds = calloc(mlib.numMaterials + 1, sizeof(int));
	*(materialIndexBounds + 0) = 0;
	for (int i = 0; i < mlib.numMaterials; i++) {
		int thisBufferSize = *(materialGroupNumIndices + i);
		totalBufferSize += thisBufferSize;
		*(materialGroups + i) = realloc(*(materialGroups + i), thisBufferSize * sizeof(int)); // shrinkwrap the individual material index-buffers
		*(materialIndexBounds + i + 1) = totalBufferSize;
	}
	
	/* combine each material group's indices into one index buffer */
	int numFaceComponents = 0;
	int* indices = calloc(totalBufferSize, sizeof(int));
	for (int i = 0; i < mlib.numMaterials; i++) {
		int* materialGroupIndexBuffer = *(materialGroups + i);
		int numIndices = *(materialGroupNumIndices + i);
		
		memcpy(indices + numFaceComponents, materialGroupIndexBuffer, numIndices * sizeof(int));

		numFaceComponents += numIndices;
		free(materialGroupIndexBuffer);
	}
	free(materialGroups);
	free(materialGroupNumIndices);

	indices = realloc(indices, numFaceComponents * sizeof(int));
	positionsFiltered = realloc(positionsFiltered, (numUniqueIndices+1) * sizeof(vec3f));
	normalsFiltered = realloc(normalsFiltered, (numUniqueIndices+1) * sizeof(vec3f));

	obj_data od = { 0 };
	od.numTris = numIndexGroups / 3;
	od.numMaterials = mlib.numMaterials;
	od.numUniqueIndices = numUniqueIndices + 1;

	od.materialBounds = materialIndexBounds;
	od.indices = indices;
	od.positions = positionsFiltered;
	od.normals = normalsFiltered;
	od.materials = mlib;

	return od;
}

void free_obj_data(obj_data* od) {
	free(od->materialBounds);
	free(od->indices);
	free(od->positions);
	free(od->normals);
	free_mtllib_data(&od->materials);
}