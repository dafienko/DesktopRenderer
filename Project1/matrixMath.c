#include "matrixMath.h"
#include <math.h>
#include <stdlib.h>

/* column-major order */
float* get_vals_mat4f(mat4f mat) {
    float* vals = calloc(16, sizeof(float));

    *(vals + 0) = mat.a;
    *(vals + 1) = mat.e;
    *(vals + 2) = mat.i;
    *(vals + 3) = mat.m;

    *(vals + 4) = mat.b;
    *(vals + 5) = mat.f;
    *(vals + 6) = mat.j;
    *(vals + 7) = mat.n;

    *(vals + 8) = mat.c;
    *(vals + 9) = mat.g;
    *(vals + 10) = mat.k;
    *(vals + 11) = mat.o;

    *(vals + 12) = mat.d;
    *(vals + 13) = mat.h;
    *(vals + 14) = mat.l;
    *(vals + 15) = mat.p;

    return vals;
}

mat4f rotate_xyz(float x, float y, float z) {
    return mat_mul_mat(rotate_x(x), mat_mul_mat(rotate_y(y), rotate_z(z)));
}

mat4f from_position_and_rotation(vec3f position, vec3f rotation) {
    return mat_mul_mat(rotate_xyz(rotation.x, rotation.y, rotation.z), from_translation(position.x, position.y, position.z));
}

mat4f new_identity() {
    mat4f identity = { 0 };

    identity.a = 1.0f;
    identity.f = 1.0f;
    identity.k = 1.0f;
    identity.p = 1.0f;

    return identity;
}

mat4f new_perspective(float nearDist, float farDist, float fov, float aspect) {
    float t = nearDist * tan((3.14159265 / 180) * (fov / 2));
    float r = t * aspect;

    mat4f m = { 0 };

    m.a = nearDist / r;
    m.f = nearDist / t;
    m.k = -(farDist + nearDist) / (farDist - nearDist);
    m.l = (-2 * farDist * nearDist) / (farDist - nearDist);
    m.o = -1;
    m.p = 0;

    return m;
}

mat4f mat_mul_mat(mat4f m1, mat4f m2) {
    return (mat4f) {
        m1.a* m2.a + m1.b * m2.e + m1.c * m2.i + m1.d * m2.m,
        m1.a* m2.b + m1.b * m2.f + m1.c * m2.j + m1.d * m2.n,
        m1.a* m2.c + m1.b * m2.g + m1.c * m2.k + m1.d * m2.o,
        m1.a* m2.d + m1.b * m2.h + m1.c * m2.l + m1.d * m2.p,

        m1.e* m2.a + m1.f * m2.e + m1.g * m2.i + m1.h * m2.m,
        m1.e* m2.b + m1.f * m2.f + m1.g * m2.j + m1.h * m2.n,
        m1.e* m2.c + m1.f * m2.g + m1.g * m2.k + m1.h * m2.o,
        m1.e* m2.d + m1.f * m2.h + m1.g * m2.l + m1.h * m2.p,

        m1.i* m2.a + m1.j * m2.e + m1.k * m2.i + m1.l * m2.m,
        m1.i* m2.b + m1.j * m2.f + m1.k * m2.j + m1.l * m2.n,
        m1.i* m2.c + m1.j * m2.g + m1.k * m2.k + m1.l * m2.o,
        m1.i* m2.d + m1.j * m2.h + m1.k * m2.l + m1.l * m2.p,

        m1.m* m2.a + m1.n * m2.e + m1.o * m2.i + m1.p * m2.m,
        m1.m* m2.b + m1.n * m2.f + m1.o * m2.j + m1.p * m2.n,
        m1.m* m2.c + m1.n * m2.g + m1.o * m2.k + m1.p * m2.o,
        m1.m* m2.d + m1.n * m2.h + m1.o * m2.l + m1.p * m2.p
    };
}


mat4f rotate_x(float a) {
    return (mat4f) {
        1, 0, 0, 0,
        0, cos(a), -sin(a), 0,
        0, sin(a), cos(a), 0,
        0, 0, 0, 1
    };
}

mat4f rotate_y(float a) {
    return (mat4f) {
        cos(a), 0, sin(a), 0,
        0, 1, 0, 0,
        -sin(a), 0, cos(a), 0,
        0, 0, 0, 1
    };
}

mat4f rotate_z(float a) {
    return (mat4f) {
        cos(a), -sin(a), 0, 0,
        sin(a), cos(a), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

mat4f from_translation(float x, float y, float z) {
    return (mat4f) {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    };
}


vec4f mat_mul_vec4f(mat4f m, vec4f v) {
    return (vec4f) {
        m.a* v.x + m.b * v.y + m.c * v.z + m.d * v.w,
            m.e* v.x + m.f * v.y + m.g * v.z + m.h * v.w,
            m.i* v.x + m.j * v.y + m.k * v.z + m.l * v.w,
            m.m* v.x + m.n * v.y + m.o * v.z + m.p * v.w
    };
};