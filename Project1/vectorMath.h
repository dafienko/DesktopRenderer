#ifndef CUSTOMVECTORMATH_H
#define CUSTOMVECTORMATH_H

typedef struct vec2i {
	int x, y;
} vec2i;

typedef struct vec3i {
	int x, y, z;
} vec3i;

typedef struct vec4i {
	int x, y, z, w;
} vec4i;


typedef struct vec2f {
	float x, y;
} vec2f;

typedef struct vec3f {
	float x, y, z;
} vec3f;

typedef struct vec4f {
	float x, y, z, w;
} vec4f;

/* vec2i stuff */
vec2i vector_add_2i(const vec2i, const vec2i);
vec2i vector_sub_2i(const vec2i, const vec2i);
vec2i vector_div_2i(const vec2i, const int);

/* vec4i stuff */
int vec4i_equal(const vec4i a, const vec4i b);

/* vec2f  stuff */
vec2f vector_add_2f(const vec2f a, const vec2f b);
vec2f vector_sub_2f(const vec2f a, const vec2f b);
vec2f vector_div_2f(const vec2f a, const float s);
float vector_dot_2f(const vec2f a, const vec2f b);
float magnitude_2f(const vec2f a);
vec2f normalize_2f(const vec2f a);
int vec2f_equal(const vec2f a, const vec2f b);

/* vec3f stuff */
vec3f vector_add_3f(const vec3f a, const vec3f b);
vec3f vector_sub_3f(const vec3f a, const vec3f b);
vec3f vector_div_3f(const vec3f a, const float s);
vec3f vector_mul_3f(const vec3f a, const float s);
float vector_dot_3f(const vec3f a, const vec3f b);
vec3f vector_cross_3f(const vec3f a, const vec3f b);
float magnitude_3f(const vec3f a);
vec3f normalize_3f(const vec3f a);
float* get_vals_vec3f(const vec3f a);
int vec3f_equal(const vec3f a, const vec3f b);
vec3f inverse_vec3f(const vec3f a);
vec3f cross_vec3f(const vec3f a, const vec3f b);

/* vec4f stuff */
vec4f vector_add_4f(const vec4f a, const vec4f b);
vec4f vector_sub_4f(const vec4f a, const vec4f b);
vec4f vector_div_4f(const vec4f a, const float s);
vec4f vector_mul_4f(const vec4f a, const float s);
float vector_dot_4f(const vec4f a, const vec4f b);
float magnitude_4f(const vec4f a);
vec4f normalize_4f(const vec4f a);
float* get_vals_vec4f(const vec4f a);
int vec4f_equal(const vec4f a, const vec4f b);

#endif