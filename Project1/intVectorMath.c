#pragma once

#include "vectorMath.h"


/* vec2i stuff */
vec2i vector_add_2i(const vec2i a, const vec2i b) {
	return (vec2i) {
		a.x + b.x,
		a.y + b.y
	};
}

vec2i vector_sub_2i(const vec2i a, const vec2i b) {
	return (vec2i) {
		a.x - b.x,
		a.y - b.y
	};
}

vec2i vector_div_2i(const vec2i a, const int d) {
	return (vec2i) {
		a.x / d,
		a.y / d
	};
}


/* vec4i stuff */
int vec4i_equal(const vec4i a, const vec4i b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}