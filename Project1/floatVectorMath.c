#pragma once

#include "vectorMath.h"
#include <math.h>
#include <stdlib.h>

/* vec2 stuff */
vec2f vector_add_2f(const vec2f a, const vec2f b) {
	return (vec2f) {
		a.x + b.x,
		a.y + b.y
	};
}

vec2f vector_sub_2f(const vec2f a, const vec2f b) {
	return (vec2f) {
		a.x - b.x,
			a.y - b.y
	};
}

vec2f vector_div_2f(const vec2f a, const float s) {
	return (vec2f) {
		a.x / s,
		a.y / s
	};
}

float vector_dot_2f(const vec2f a, const vec2f b) {
	return a.x * b.x + a.y * b.y;
}

float magnitude_2f(const vec2f a) {
	return (float)sqrt((float)(a.x * a.x + a.y * a.y));
}

vec2f normalize_2f(const vec2f a) {
	float m = magnitude_2f(a);

	return vector_div_2f(a, m);
}

/* vec3 stuff */
vec3f vector_add_3f(const vec3f a, const vec3f b) {
	return (vec3f) {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

vec3f vector_sub_3f(const vec3f a, const vec3f b) {
	return (vec3f) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

vec3f vector_div_3f(const vec3f a, const float s) {
	return (vec3f) {
		a.x / s,
		a.y / s,
		a.z / s
	};
}

vec3f vector_mul_3f(const vec3f a, const float s) {
	return (vec3f) {
		a.x * s,
		a.y * s,
		a.z * s
	};
}

float vector_dot_3f(const vec3f a, const vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3f vector_cross_3f(const vec3f a, const vec3f b) {
	return (vec3f) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float magnitude_3f(const vec3f a) {
	return (float)sqrt((float)(a.x * a.x + a.y * a.y + a.z * a.z));
}

vec3f normalize_3f(const vec3f a) {
	float m = magnitude_3f(a);

	return vector_div_3f(a, m);
}

float* get_vals_vec3f(const vec3f a) {
	float* vals = calloc(3, sizeof(float));

	*(vals + 0) = a.x;
	*(vals + 1) = a.y;
	*(vals + 2) = a.z;

	return vals;
}

vec3f inverse_vec3f(const vec3f a) {
	return (vec3f) { -a.x, -a.y, -a.z };
}

vec3f cross_vec3f(const vec3f a, const vec3f b) {
	vec3f r = { 0 };

	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;

	return r;
}

/* vec4 stuff */
vec4f vector_add_4f(const vec4f a, const vec4f b) {
	return (vec4f) {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
}

vec4f vector_sub_4f(const vec4f a, const vec4f b) {
	return (vec4f) {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	};
}

vec4f vector_div_4f(const vec4f a, const float s) {
	return (vec4f) {
		a.x / s,
		a.y / s,
		a.z / s,
		a.w / s
	};
}

vec4f vector_mul_4f(const vec4f a, const float s) {
	return (vec4f) {
		a.x * s,
		a.y * s,
		a.z * s,
		a.w * s
	};
}

float vector_dot_4f(const vec4f a, const vec4f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float magnitude_4f(const vec4f a) {
	return (float)sqrt((float)(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w));
}

vec4f normalize_4f(const vec4f a) {
	float m = magnitude_4f(a);

	return vector_div_4f(a, m);
}

float* get_vals_vec4f(const vec4f a) {
	float* vals = calloc(4, sizeof(float));

	*(vals + 0) = a.x;
	*(vals + 1) = a.y;
	*(vals + 2) = a.z;
	*(vals + 3) = a.w;

	return vals;
}
