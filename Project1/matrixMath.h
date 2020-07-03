#ifndef CUSTOMMATRIXMATH_H
#define CUSTOMMATRIXMATH_H

#include "vectorMath.h"

typedef struct mat4f {
	float a, b, c, d;
	float e, f, g, h;
	float i, j, k, l;
	float m, n, o, p;
} mat4f;

mat4f new_identity();
mat4f new_perspective(float nearDist, float farDist, float fov, float aspect);

float* get_vals_mat4f(mat4f mat);

mat4f mat_mul_mat(mat4f m1, mat4f m2);
vec4f mat_mul_vec4f(mat4f m, vec4f v);

mat4f from_translation(float x, float y, float z);
mat4f from_position_and_rotation(vec3f p, vec3f r);

mat4f rotate_xyz(float x, float y, float z);
mat4f rotate_x(float a);
mat4f rotate_y(float a);
mat4f rotate_z(float a);

#endif