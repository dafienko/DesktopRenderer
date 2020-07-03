#include "assetLoader.h"
#include "errors.h"
#include "stringUtil.h"
#include <stdlib.h>
#include <stdio.h>

/* for debugging file-loading */
void print_mtllib(mtllib* ml) {
	int bufferSize = 1000;
	char* buffer = calloc(bufferSize, sizeof(char));

	for (int i = 0; i < ml->numMaterials; i++) {
		memset(buffer, 0, bufferSize * sizeof(char));

		mtllib_material mm = *(ml->materials + i);

		sprintf_s(buffer, bufferSize,
			"material %s {\n\tambient: (%0.3f, %0.3f, %0.3f)\n\tdiffuse: (%0.3f, %0.3f, %0.3f)\n\tspecular: (%0.3f, %0.3f, %0.3f)\n}\n",
			mm.materialName, 
			mm.material.ambient.x, mm.material.ambient.y, mm.material.ambient.z,
			mm.material.diffuse.x, mm.material.diffuse.y, mm.material.diffuse.z,
			mm.material.specular.x, mm.material.specular.y, mm.material.specular.z);

		OutputDebugStringA(buffer);
	}

	free(buffer);
}

vec3f parse_color(SPLITSTR components) {
	vec3f ambient = { 0 };

	ambient.x = atof(*(components + 1));
	ambient.y = atof(*(components + 2));
	ambient.z = atof(*(components + 3));

	return ambient;
}

mtllib read_mtl_file(const char* filename) {
	lines_data ld = get_file_lines(filename);

	int materialIndex = -1;
	int materialSpace = 5;
	mtllib_material* materials = calloc(materialSpace, sizeof(mtllib_material));

	for (int i = 0; i < ld.numLines; i++) {
		char* line_raw = *(ld.lines + i);
		STRIPPEDSTR lstripped = lstrip(line_raw);
		STRIPPEDSTR rstripped = rstrip(lstripped);
		SPLITSTR components = strsplit(rstripped, " ");
		char* firstToken = *(components + 0);

		if (strcmp(firstToken, "newmtl") == 0) {
			materialIndex++;
			if (materialIndex >= materialSpace) {
				materialSpace *= 2;
				materials = realloc(materials, materialSpace * sizeof(mtllib_material));
			}

			char* materialName = *(components + 1);

			(materials + materialIndex)->materialName = calloc(strlen(materialName) + 1, sizeof(char));
			memcpy((materials + materialIndex)->materialName, materialName, strlen(materialName) * sizeof(char));
		}
		else if (strcmp(firstToken, "Ka") == 0) {
			(materials + materialIndex)->material.ambient = parse_color(components);
		}
		else if (strcmp(firstToken, "Kd") == 0) {
			(materials + materialIndex)->material.diffuse = parse_color(components);
		}
		else if (strcmp(firstToken, "Ks") == 0) {
			(materials + materialIndex)->material.specular = parse_color(components);
		}

		free_strippedstr(&lstripped);
		free_strippedstr(&rstripped);
		free_splitstr(&components);
	}

	free_lines_data(&ld);

	materials = realloc(materials, (materialIndex + 1) * sizeof(mtllib_material)); // shrinkwrap the materials pointer

	mtllib ml = { 0 };
	ml.materials = materials;
	ml.numMaterials = materialIndex + 1;

	return ml;
}

void free_mtllib_material(mtllib_material* mlm) {
	free(mlm->materialName); 
}

void free_mtllib_data(mtllib* ml) {
	for (int i = 0; i < ml->numMaterials; i++) {
		free_mtllib_material(ml->materials + i);
	}

	free(ml->materials);
}

