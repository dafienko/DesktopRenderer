
#define NO_MIN_MAX

#include "program.h"
#include "keyboard.h"
#include "renderer.h"
#include "matrixMath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef PI 
#define PI 3.14159265359
#endif

float rad(float d) {
    return d * (PI / 180.0f);
}


void programInit() {

}

float sensitivity = .2;
float speed = 5.0f;
void onFrame(float dt) {
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

}

void programClose() {

}