#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "glExtensions.h"
#include "vectorMath.h"
#include "matrixMath.h"

typedef struct {
	vec3f velocity;
	vec3f rotVelocity;
	vec3f position;
	vec3f orientation;
	float age;

	int initialized;
} Particle;

typedef struct {
	GLuint texId;
	int maxParticles;
	int emissionRate;
	vec3f direction;
	vec3f position;
	vec3f size;
	float lifetime;

	Particle* particles;
	int numParticles;
	float numParticlesF;
} Particle_Emitter;

Particle_Emitter* create_particle_emitter(int maxParticles, float rate, vec3f dir, GLuint tex);

float rand_rangef(float min, float max);

void render_emitters(mat4f cameraMatrix, mat4f perspectiveMatrix, int renderColor);
void updateEmitters(float dt);

#endif