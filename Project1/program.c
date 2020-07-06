
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

#ifndef PI 
#define PI 3.14159265359
#endif

GLuint defaultSkybox;

float rad(float d) {
    return d * (PI / 180.0f);
}

int width = 35;
int length = 50;
float lineWidth = 2.0f;
float scale = 40.0f;
float heightFactor = 500.0f;
float eqFactor = .15f;

vec3f* positions;
HMESH hMesh;
object_model* om;

float get_y_pos(int x, int z) {
    //float y = sinf(x * eqFactor) * cosf(z * eqFactor) * heightFactor;
    float y = perlin2d(x * eqFactor, z * eqFactor, 1, 4);

    float edgeValue = fabs(x - ((width - 1.0f) / 2.0f)) / ((width - 1.0f) / 2.0f); // 1 is right next to an edge, 0 is right in the middle
    edgeValue = min(1, edgeValue + .2f);
    float edgeFactor = powf(edgeValue, 5) * heightFactor;

    y *= edgeFactor;

    return y;
}

void programInit() {
    //defaultSkybox = create_skybox_texture("assets/skybox.png");
    //set_current_skybox(defaultSkybox);

    set_background_color(0.0f, 0.0f, 0.0f);

    /*
    HMESH hMesh = load_mesh("models/small_complex_world.obj", "models/small_complex_world.mtl");
    object_model* om = create_model_from_mesh(hMesh);
    om->scale = (vec3f){ .5f, .5f, .5f };
    */

    int numLines = width * (length - 1) + length * (width - 1);
    
    vec3f* normals = calloc(width * length * 4, sizeof(vec3f));
    int* indices = calloc(width * length * 4 + 4 * numLines, sizeof(int));
    positions = calloc(width * length * 4, sizeof(vec3f));

    vec3f normal = (vec3f){ 0, 1, 0 };

    float loffMid = lineWidth / 2.0f;

    /* buffer positions / normals */
    int indexNum = 0;
    for (int z = 0; z < length; z++) {
        for (int x = 0; x < width; x++) {
            int offset = z * width + x;
            offset *= 4;

            vec3f pos = (vec3f){ x * scale, get_y_pos(x, z), -z * scale };
            
            *(positions + offset + 0) = vector_add_3f(pos, (vec3f) { -loffMid, 0, -loffMid }); // topLeft
            *(positions + offset + 1) = vector_add_3f(pos, (vec3f) { loffMid, 0, -loffMid }); // topRight
            *(positions + offset + 2) = vector_add_3f(pos, (vec3f) { -loffMid, 0, loffMid }); // bottomLeft
            *(positions + offset + 3) = vector_add_3f(pos, (vec3f) { loffMid, 0, loffMid }); // bottomRight

            *(indices + indexNum) = offset + 0;
            indexNum++;
            *(indices + indexNum) = offset + 2;
            indexNum++;
            *(indices + indexNum) = offset + 3;
            indexNum++;
            *(indices + indexNum) = offset + 1;
            indexNum++;

            for (int i = 0; i < 4; i++) {
                *(normals + offset + i) = normal;
            }
        }
    }

    
   
    for (int z = 0; z < length; z++) {
        for (int x = 0; x < width; x++) {
            int thisIndex = 4 * (z * width + x);
            
            // horizontal lines
            if (x < width - 1) {
                int rightIndex = 4 * (z * width + x + 1);

                *(indices + indexNum) = thisIndex + 1;
                indexNum++;
                *(indices + indexNum) = thisIndex + 3;
                indexNum++;
                *(indices + indexNum) = rightIndex + 2;
                indexNum++;
                *(indices + indexNum) = rightIndex + 0;
                indexNum++;
            }

            // "vertical" lines
            if (z > 0) {
                int bottomIndex = 4 * ((z - 1) * width + x);

                *(indices + indexNum) = thisIndex + 3;
                indexNum++;
                *(indices + indexNum) = thisIndex + 2;
                indexNum++;
                *(indices + indexNum) = bottomIndex + 0;
                indexNum++;
                *(indices + indexNum) = bottomIndex + 1;
                indexNum++;
            }
        }
    }

    int* groupData = calloc(2, sizeof(int));
    *(groupData + 1) = width * length * 4 + numLines * 4;

    mtllib_material mat = { 0 };
    mat.material.ambient = (vec3f){ 0, .5, 1 };
    mat.material.diffuse = (vec3f){ 0, 0, 1 };
    mat.material.emitter = 1;
    mat.materialName = calloc(2, sizeof(char));

    mtllib mlib = { 0 };
    mlib.materials = calloc(1, sizeof(mtllib_material));
    mlib.numMaterials = 1;
    *(mlib.materials) = mat;

    hMesh = init_dynamic_mesh(GL_QUADS);
    edit_mesh_indices(hMesh, indices, width * length * 4 + numLines * 4);
    edit_mesh_positions(hMesh, positions, width * length * 4);
    edit_mesh_normals(hMesh, normals, width * length * 4);
    edit_mesh_mtl_data(hMesh, mlib, groupData, 2);

    glLineWidth(5.0f);
    om = create_model_from_mesh(hMesh);
    om->visible = 1;

    free(normals);
    free(indices);

    currentCamera.position = (vec3f){ 0, 80.0f, 0 };
}

float sensitivity = .2;
float speed = 50.0f;
void handle_input(float dt) {
    float xDir = 0;
    float zDir = 0;
    float yDir = 0;

    if (isKeyDown(0x57)) { // w
        zDir += -1;
    }
    if (isKeyDown(0x53)) { // s
        zDir += 1;
    }

    if (isKeyDown(0x41)) { // a
        xDir += -1;
    }
    if (isKeyDown(0x44)) { // d
        xDir += 1;
    }

    if (isKeyDown(0x45)) { // e
        yDir += 1;
    }
    if (isKeyDown(0x51)) { // q
        yDir += -1;
    }

    vec3f rightVector = { 0 };
    rightVector.y = 0;
    rightVector.x = cos(currentCamera.rotation.y);
    rightVector.z = -sin(currentCamera.rotation.y);

    vec3f lookVector = { 0 };
    lookVector.y = sin(currentCamera.rotation.x);
    lookVector.x = cos((currentCamera.rotation.y) + rad(90));
    lookVector.z = -sin((currentCamera.rotation.y) + rad(90));

    vec3f upVector = { 0 };
    upVector.y = yDir;

    vec3f movementVector = vector_add_3f(vector_mul_3f(rightVector, xDir), vector_mul_3f(lookVector, -zDir));
    movementVector = vector_add_3f(upVector, movementVector);
    float m = magnitude_3f(movementVector);

    if (m != 0) {
        movementVector = normalize_3f(movementVector);
    }

    movementVector = vector_mul_3f(movementVector, speed * dt);

    float speedModifier = 1;
    if (isKeyDown(VK_SHIFT)) {
        speedModifier = .05;
    }

    vec3f pos = currentCamera.position;
    currentCamera.position = vector_add_3f(pos, vector_mul_3f(movementVector, speedModifier));

    if (isKeyDown(VK_UP)) {
        currentCamera.rotation.x += dt * sensitivity * 10;
    }
    if (isKeyDown(VK_DOWN)) {
        currentCamera.rotation.x += dt * -sensitivity * 10;
    }
    currentCamera.rotation.x = min(max(currentCamera.rotation.x, rad(-80.0f)), rad(80.0f));

    if (isKeyDown(VK_RIGHT)) {
        currentCamera.rotation.y += dt * -sensitivity * 10;
    }
    if (isKeyDown(VK_LEFT)) {
        currentCamera.rotation.y += dt * sensitivity * 10;
    }


    if (isKeyDown(VK_ADD)) {
        set_fov(get_fov() + dt * 10);
    }
    else if (isKeyDown(VK_SUBTRACT)) {
        set_fov(get_fov() - dt * 10);
    }
}

float totalTime = 0;
float timePerMove = 1.0f;
int lastPos = 0;
void onFrame(float dt) {
    //handle_input(dt);

    totalTime += dt;
 
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

        edit_mesh_positions(hMesh, positions, width * length * 4);
    }

    om->position = (vec3f){-(width - 1) * scale * .5f, 0, offset * scale};
}

void programClose() {
    free(positions);
}