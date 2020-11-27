#define _CRT_SECURE_NO_WARNINGS

#include "particleEmitter.h"
#include "matrixMath.h"
#include <stdlib.h>
#include "errors.h"
#include <stdio.h>
#include <math.h>

Particle_Emitter* emitters;
int numEmitters;
int emitterArrSize;

int emittersInitialized = 0;

GLuint particleProg;
GLuint* pVAO;
GLuint* pVBO;

float DEFAULT_QUAD_CORNERS[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f
};

float rand_rangef(float min, float max) {
	float range = max - min;
	
	float r = rand() / (float)RAND_MAX;

	return min + r * range;
}

Particle create_particle_for_emitter(Particle_Emitter emitter) {
	Particle particle = { 0 };

	vec3f bottomLeft = vector_add_3f(emitter.position, vector_div_3f(emitter.size, -2.0f));
	vec3f topRight = vector_add_3f(emitter.position, vector_div_3f(emitter.size, 2.0f));

	vec3f pos = { 0 };
	pos.x = rand_rangef(bottomLeft.x, topRight.x);
	pos.y = rand_rangef(bottomLeft.y, topRight.y);
	pos.z = rand_rangef(bottomLeft.z, topRight.z);

	particle.position = pos;
	particle.orientation = (vec3f){
		rand_rangef(0, 3.14 / 2),
		rand_rangef(0, 3.14 / 2),
		rand_rangef(0, 3.14 / 2)
	};
	particle.velocity = emitter.direction;
	particle.speed = emitter.speed * rand_rangef(1.0f, 1.2f);
	particle.age = 0.0f;

	particle.rotSpeed = 1.0f;
	particle.rotVelocity = (vec3f){
		rand_rangef(0, 3.14 / 2),
		rand_rangef(0, 3.14 / 2),
		rand_rangef(0, 3.14 / 2)
	};

	particle.initialized = 1;

	return particle;
}

void add_particle_to_emitter(Particle p, Particle_Emitter emitter) {
	for (int i = 0; i < emitter.maxParticles; i++) {
		Particle particle = *(emitter.particles + i);
		if (!particle.initialized) {
			*(emitter.particles + i) = p;
			return;
		}
	}
}

void update_emitter(Particle_Emitter* emitterPtr, float dt) {
	Particle_Emitter emitter = *emitterPtr;
	for (int i = 0; i < emitter.maxParticles; i++) {
		Particle p = *(emitter.particles + i);
		
		if (p.initialized) {
			p.age += dt;

			if (p.age > emitter.lifetime) { // if the particle is too old, "kill" it
				p = (Particle){ 0 };
				emitter.numParticles--;
			}
		}

		*(emitter.particles + i) = p;
	}

	if (emitter.numParticles < emitter.maxParticles) {
		emitter.numParticlesF += emitter.emissionRate * dt;
		emitter.numParticlesF = min((float)emitter.maxParticles, emitter.numParticlesF);

		if ((int)emitter.numParticlesF > emitter.numParticles) {
			int newNumParticles = (int)emitter.numParticlesF;
			int numParticlesToAdd = newNumParticles - emitter.numParticles;

			for (int i = 0; i < numParticlesToAdd; i++) {
				Particle p = create_particle_for_emitter(emitter);
				add_particle_to_emitter(p, emitter);
			}

			emitter.numParticles = newNumParticles;
		}
	}

	for (int i = 0; i < emitter.maxParticles; i++) {
		Particle* p = emitter.particles + i;

		if (p->initialized) {
			vec3f newPos = vector_add_3f(p->position, vector_mul_3f(p->velocity, dt * p->speed));
			p->position = newPos;

			p->orientation = vector_add_3f(p->orientation, vector_mul_3f(p->rotVelocity, dt * p->rotSpeed));
		}
	}
	
	*emitterPtr = emitter;
}

void updateEmitters(float dt) {
	if (emittersInitialized) {
		for (int i = 0; i < numEmitters; i++) {
			update_emitter(emitters + i, dt);
		}
	}
}

#define INSTANCE_GROUP_SIZE 50
void render_emitter(Particle_Emitter emitter, mat4f cameraMatrix, mat4f perspectiveMatrix, int renderColor) {
	glUseProgram(particleProg);

	GLuint sLoc, mvLoc, pLoc, aLoc, rcLoc;

	sLoc = glGetUniformLocation(particleProg, "scale");
	pLoc = glGetUniformLocation(particleProg, "projMat");
	rcLoc = glGetUniformLocation(particleProg, "renderColor");

	glUniform1i(rcLoc, renderColor);

	glBindVertexArray(*pVAO);

	glBindBuffer(GL_ARRAY_BUFFER, *(pVBO + 0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, *(pVBO + 1));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	float* vals = get_vals_mat4f(perspectiveMatrix);
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, vals);
	free(vals);

	char* uniformName = calloc(40, sizeof(char));

	int pi = 0;
	for (int g = 0; g < ceil(emitter.maxParticles / INSTANCE_GROUP_SIZE); g++) {
		for (int i = 0; i < INSTANCE_GROUP_SIZE && pi < emitter.maxParticles; i++) {
			Particle particle = *(emitter.particles + i + g * INSTANCE_GROUP_SIZE);

			if (particle.initialized) {
				aLoc = glGetUniformLocation(particleProg, "particleAlive");
				glUniform1i(aLoc, 1);

				sprintf(uniformName, "mvMat[%i]", i);
				mvLoc = glGetUniformLocation(particleProg, uniformName);

				mat4f mMatrix = from_position_and_rotation(particle.position, particle.orientation);
				mat4f mvMatrix = mat_mul_mat(cameraMatrix, mMatrix);

				vals = get_vals_mat4f(mvMatrix);
				glUniformMatrix4fv(mvLoc, 1, GL_FALSE, vals);
				free(vals);

				pi++;
			}
		}

		glDrawArraysInstanced(GL_QUADS, 0, 4, INSTANCE_GROUP_SIZE);
	}
	free(uniformName);
}

void render_emitters(mat4f cameraMatrix, mat4f perspectiveMatrix, int renderColor) {
	if (emittersInitialized) {
		glDisable(GL_CULL_FACE); // don't cull either side of a particle;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i < numEmitters; i++) {
			Particle_Emitter emitter = *(emitters + i);
			render_emitter(emitter, cameraMatrix, perspectiveMatrix, renderColor);
		}
		glDisable(GL_BLEND);
	}
}

void init_emitters() {
	time_t t;
	srand((unsigned)time(&t));

	emitterArrSize = 10;
	emitters = calloc(emitterArrSize, sizeof(Particle_Emitter));
	numEmitters = 0;


	pVAO = calloc(1, sizeof(GLuint));
	pVBO = calloc(2, sizeof(GLuint));

	glGenVertexArrays(1, pVAO);
	glBindVertexArray(*pVAO);
	glGenBuffers(2, pVBO);
	CHECK_GL_ERRORS;

	float quadPositions[] = {
		-.5f, .5f,
		-.5f, -.5f,
		.5f, -.5f,
		.5f, .5f
	};

	glBindBuffer(GL_ARRAY_BUFFER, *pVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositions), quadPositions, GL_DYNAMIC_DRAW);
	CHECK_GL_ERRORS;
	glBindBuffer(GL_ARRAY_BUFFER, *(pVBO + 1));
	glBufferData(GL_ARRAY_BUFFER, sizeof(DEFAULT_QUAD_CORNERS), DEFAULT_QUAD_CORNERS, GL_STATIC_DRAW);
	CHECK_GL_ERRORS;

	GLuint pvs = create_vertex_shader("shaders\\particle.vs");
	GLuint pfs = create_fragment_shader("shaders\\particle.fs"); 

	GLuint quadShaders[] = { pvs, pfs };
	particleProg = create_program(quadShaders, 2);


	emittersInitialized = 1;
}

Particle_Emitter* add_emitter(Particle_Emitter emitter) {
	if (numEmitters + 1 <= emitterArrSize) {
		emitterArrSize *= 2;
		emitters = realloc(emitters, emitterArrSize * sizeof(Particle_Emitter));
	}

	Particle_Emitter* ptr = emitters + numEmitters;
	*ptr = emitter;
	numEmitters++;

	return ptr;
}

Particle_Emitter* create_particle_emitter(int maxParticles, float rate, float speed, vec3f dir, GLuint tex) {
	if (!emittersInitialized) {
		init_emitters();
	}
	
	Particle_Emitter emitter = { 0 };

	emitter.maxParticles = maxParticles;
	emitter.emissionRate = rate;
	emitter.direction = normalize_3f(dir);
	emitter.speed = speed;
	emitter.numParticlesF = 0.0f;
	emitter.numParticles = 0;
	emitter.texId = tex;
	emitter.particles = calloc(maxParticles, sizeof(Particle));

	return add_emitter(emitter);
}