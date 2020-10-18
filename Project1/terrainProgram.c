
#define NO_MIN_MAX

#include "program.h"
#include "keyboard.h"
#include "renderer.h"
#include "matrixMath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "assetLoader.h"
#include <math.h>
#include "perlin.h"
#include "stringUtil.h"

#ifndef PI 
#define PI 3.14159265359
#endif

GLuint defaultSkybox;

float rad(float d) {
    return d * (PI / 180.0f);
}

int width = 35; // 35
int length = 50; // 50
float lineWidth = 3.0f;
float scale = 50.0f;
float heightFactor = 500.0f;
float eqFactor = .15f;

vec3f* positions;
vec3f* normals;
vec3f* colors; 
HMESH hMesh;
object_model* om;

float get_y_pos(int x, int z) {
    float y = perlin2d(x * eqFactor, z * eqFactor, 1, 4);

    float edgeValue = fabs(x - ((width - 1.0f) / 2.0f)) / ((width - 1.0f) / 2.0f); // 1 is right next to an edge, 0 is right in the middle
    edgeValue = min(1, edgeValue + .2f);
    float edgeFactor = powf(edgeValue, 5) * heightFactor;

    y *= edgeFactor;

    y += max(0, perlin2d(x * eqFactor * 5, z * eqFactor * 5, 1, 1) - .6f) * heightFactor * .08;

    return y;
}

void update_normals() {
    for (int z = 0; z < length; z++) {
        for (int x = 0; x < width; x++) {
            int thisIndex = 4 * (z * width + x);
            if (z > 0 && z < length - 1 && x > 0 && x < width - 1) {
                vec3f thisPos = *(positions + thisIndex);

                int topIndex = 4 * ((z + 1) * width + x + 0);
                int bottomIndex = 4 * ((z - 1) * width + x + 0);
                int rightIndex = 4 * ((z + 0) * width + x + 1);
                int leftIndex = 4 * ((z + 0) * width + x - 1);

                vec3f topPos = *(positions + topIndex);
                vec3f bottomPos = *(positions + bottomIndex);
                vec3f rightPos = *(positions + rightIndex);
                vec3f leftPos = *(positions + leftIndex);

                vec3f up = vector_sub_3f(topPos, thisPos);
                vec3f down = vector_sub_3f(bottomPos, thisPos);
                vec3f right = vector_sub_3f(rightPos, thisPos);
                vec3f left = vector_sub_3f(leftPos, thisPos);

                vec3f topRightNormal = normalize_3f(vector_cross_3f(right, up));
                vec3f topLeftNormal = normalize_3f(vector_cross_3f(up, left));
                vec3f bottomRightNormal = normalize_3f(vector_cross_3f(down, right));
                vec3f bottomLeftNormal = normalize_3f(vector_cross_3f(left, down));

                *(normals + thisIndex + 0) = topLeftNormal;
                *(normals + thisIndex + 1) = topRightNormal;
                *(normals + thisIndex + 2) = bottomLeftNormal;
                *(normals + thisIndex + 3) = bottomRightNormal;
            }
            else {
                vec3f n = (vec3f){ 0, 1, 0 };
                for (int i = 0; i < 4; i++) {
                    *(normals + thisIndex + i) = n;
                }
            }
        }
    }
}

void programInit(LPSTR args) {
    vec3f lineColor = (vec3f){ 1, 1, 1 };

    set_sun_pos(0, 1, 0);
    set_sun_color(1, 1, 1);
    defaultSkybox = create_skybox_texture("assets/skybox.png");
    set_current_skybox(defaultSkybox);

    int numQuads = (width - 1) * (length - 1);
    int numTris = numQuads * 2; // 2 tri's per quad
    positions = calloc(numTris * 3, sizeof(vec3f));
    normals = calloc(numTris * 3, sizeof(vec3f));
    colors = calloc(numTris * 3, sizeof(vec3f));
    int* indices = calloc(numTris * 3, sizeof(int));
    
    // generate height map
    float* heights = calloc(width * length, sizeof(float));
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < length; z++) {
            *(heights + x * length + z) = get_y_pos(x, z);
        }
    }
    
    for (int x = 0; x < width - 1; x++) {
        for (int z = 0; z < length - 1; z++) {
            vec3f TLpos = (vec3f) {x }
        }
    }
    
    
    
    hMesh = init_dynamic_mesh(GL_TRIANGLES);

    edit_mesh_indices(hMesh, indices, numTris * 3);
    edit_mesh_positions(hMesh, positions, numTris * 3);
    edit_mesh_normals(hMesh, normals, numTris * 3);

    om = create_model_from_mesh(hMesh);
    om->visible = 1;

    free(indices);

    currentCamera.position = (vec3f){ 0, 80.0f, 0 };
}


float totalTime = 0;
float timePerMove = 1.0f;
int lastPos = 0;
void onFrame(float dt) {
    //handle_input(dt);

    totalTime += dt;

    ///*
    float offset = fmod(totalTime, timePerMove) / timePerMove;
    int pos = floor(totalTime / timePerMove);
    if (pos > lastPos) {
        lastPos = pos;

        float loffMid = lineWidth / 2.0f;

        for (int z = 0; z < length; z++) {
            for (int x = 0; x < width; x++) {
                int offset = z * width + x;
                offset *= 4;

                vec3f pos = (vec3f){ x * scale, get_y_pos(x, z + lastPos), -z * scale };

                *(positions + offset + 0) = vector_add_3f(pos, (vec3f) { -loffMid, 0, -loffMid }); // topLeft
                *(positions + offset + 1) = vector_add_3f(pos, (vec3f) { loffMid, 0, -loffMid }); // topRight
                *(positions + offset + 2) = vector_add_3f(pos, (vec3f) { -loffMid, 0, loffMid }); // bottomLeft
                *(positions + offset + 3) = vector_add_3f(pos, (vec3f) { loffMid, 0, loffMid }); // bottomRight
            }
        }

        if (FILL_QUADS) {
            update_normals();
        }

        edit_mesh_positions(hMesh, positions, width * length * 4);
        edit_mesh_normals(hMesh, normals, width * length * 4);
    }

    om->position = (vec3f){ -(width - 1) * scale * .5f, 0, offset * scale };
    //*/
}

void programClose() {
    free(positions);
    free(normals);
}