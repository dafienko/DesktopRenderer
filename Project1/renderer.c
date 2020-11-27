#include "renderer.h"
#include "assetLoader.h"
#include "glExtensions.h"
#include "matrixMath.h"
#include "stringUtil.h"
#include "program.h"
#include "timer.h"
#include "winUtil.h"
#include "errors.h"
#include "particleEmitter.h"

#pragma comment(lib, "Vfw32.lib")

#ifndef PI
#define PI 3.14159265359
#endif
timer t = { 0 };

/* general settings */
vec3f lightDir = { 0, -.43f, 1 };
vec3f backgroundColor = { 1, 0, 0 };
vec3f sunColor = { 0, 0, 0 };
float fov = 105.0f;

/* MSAA settings */
int antialiased = 1;
int MSAASamples = 4;

/* Bloom settings */
float threshold = .7f;
int bloomSize = 15;
float bloomOffsetScale = 1.0f;
float intensity = 3.5f;

int bufferWidth, bufferHeight;
unsigned char* lpBits = NULL;

GLuint prog, skyboxProg, blurProg, bloomProg, gskyboxProg;
GLuint framebuffer, rbo, tcb;
GLuint brbo, bfbo, btex;
GLuint brbo2, bfbo2, btex2;
GLuint trbo, tfbo, ttex;

int numPBOs = 2;
GLuint* pbo = NULL;

GLuint planeVAO, planeVBO;
GLuint skyboxTexture;
drawable skybox = { 0 };
object_model skyboxOM = { 0 };

object_model* objectModels = NULL;
int numObjectModels = 0;
int objectModelSpace = 0;

drawable* meshes = NULL;
int numMeshes = 0;
int meshSpace = 0;

int initialized = 0;

HDC glContextHDC;
void use_rc(HDC* hdc, HGLRC* hrc) {
	BOOL success = wglMakeCurrent(*hdc, *hrc);
	glContextHDC = *hdc;

	if (success == FALSE) {
		CHECK_ERRORS;
	}
}

void set_sun_color(float r, float g, float b) {
	sunColor = (vec3f){ r, g, b };
}

void set_fov(const float newFov) {
	fov = newFov;
}

float get_fov() {
	return fov;
}

void set_current_skybox(GLuint tex) {
	skyboxTexture = tex;
}

void set_sun_pos(float x, float y, float z) {
	lightDir.x = x;
	lightDir.y = y;
	lightDir.z = z;
}

void set_sb_rotation(float x, float y, float z) {
	skyboxOM.rotation = (vec3f){ x, y, z };
}

void set_background_color(float r, float g, float b) {
	backgroundColor = (vec3f){ r, g, b };
}

void init(int width, int height) {
	int sampleSize = 2; // samples per pixel is sampleSize^2

	GLuint vs = create_vertex_shader("shaders\\basic.vs");
	GLuint fs = create_fragment_shader("shaders\\basic.fs");
	GLuint shaders[] = { vs, fs };
	prog = create_program(shaders, 2);

	GLuint bvs = create_vertex_shader("shaders\\blur.vs");
	GLuint bfs = create_fragment_shader("shaders\\blur.fs");
	GLuint bshaders[] = { bvs, bfs };
	blurProg = create_program(bshaders, 2);

	GLuint sbvs = create_vertex_shader("shaders\\skybox.vs");
	GLuint sbfs = create_fragment_shader("shaders\\skybox.fs");
	GLuint skyboxShaders[] = { sbvs, sbfs };
	skyboxProg = create_program(skyboxShaders, 2);

	GLuint gsbvs = create_vertex_shader("shaders\\generated_skybox.vs");
	GLuint gsbfs = create_fragment_shader("shaders\\generated_skybox.fs");
	GLuint generatedSkyboxShaders[] = { gsbvs, gsbfs };
	gskyboxProg = create_program(generatedSkyboxShaders, 2);

	GLuint bloomvs = create_vertex_shader("shaders\\bloom.vs");
	GLuint bloomfs = create_fragment_shader("shaders\\bloom.fs");
	GLuint bloomshaders[] = { bloomvs, bloomfs };
	bloomProg = create_program(bloomshaders, 2);

	obj_data od = read_obj_file("models\\cube.obj", NULL);
	skybox = obj_to_drawable(&od);
	free_obj_data(&od);
	skybox.hProgram = skyboxProg;
	skyboxOM.position = (vec3f){ 0, 0, 0 };
	skyboxOM.rotation = (vec3f){ 0, 0, 0 };
	skyboxOM.scale = (vec3f){ 100, 100, 100 };

	timer_start(&t);

	currentCamera = (camera){ 0 };

	wglSwapIntervalEXT(0);

	lpBits = calloc(4 * width * height, sizeof(unsigned char));

	/* generate buffers for bloom effect*/
	glGenTextures(1, &btex);
	glBindTexture(GL_TEXTURE_2D, btex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &brbo);
	glBindRenderbuffer(GL_RENDERBUFFER, brbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &bfbo);
	glBindFramebuffer(GL_FRAMEBUFFER, bfbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		btex,
		0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		brbo);

	glGenTextures(1, &btex2);
	glBindTexture(GL_TEXTURE_2D, btex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*
	glGenRenderbuffers(1, &brbo2);
	glBindRenderbuffer(GL_RENDERBUFFER, brbo2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	*/

	glGenFramebuffers(1, &bfbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, bfbo2);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		btex2,
		0);

	/*
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		brbo2);
	*/

	/* generate temp buffers-- used for rendering scene w/o any post processing */
	glGenTextures(1, &ttex);
	glGenRenderbuffers(1, &trbo);
	glGenFramebuffers(1, &tfbo);
	glBindRenderbuffer(GL_RENDERBUFFER, trbo);
	glBindFramebuffer(GL_FRAMEBUFFER, tfbo);
	if (antialiased) {
		
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ttex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAASamples, GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASamples, GL_DEPTH24_STENCIL8, width, height);


		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D_MULTISAMPLE,
			ttex,
			0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			trbo);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, ttex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, trbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			ttex,
			0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			trbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/* generate plane vertex object for rendering the whole scene onto a texture */
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	float p = 1.0f;
	vec2f qPositions[] = {
		{-p, -p},
		{p, -p},
		{p, p},
		{-p, p}
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f) * 4, qPositions, GL_STATIC_DRAW);


	/* generate the framebuffer that's read from onto the screen-- this framebuffer is the last framebuffer in this rendering pipeline */
	glGenFramebuffers(1, &framebuffer);

	glGenTextures(1, &tcb); // texture color buffer
	glBindTexture(GL_TEXTURE_2D, tcb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	bufferWidth = width;
	bufferHeight = height;

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tcb, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	pbo = calloc(numPBOs, sizeof(GLuint));
	glGenBuffers(numPBOs, pbo);

	for (int i = numPBOs - 1; i >= 0; i--) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + i));
		glBufferData(GL_PIXEL_PACK_BUFFER, width * height * 4, NULL, GL_STREAM_READ);
	}

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	initialized = 1;
}

HMESH add_drawable(drawable d) {
	if (meshes == NULL) {
		meshSpace = 10;
		meshes = calloc(meshSpace, sizeof(drawable));
	}

	if (meshSpace <= numMeshes + 1) {
		meshSpace *= 2;
		meshes = realloc(meshes, meshSpace * sizeof(drawable));
	}

	HMESH hMesh = numMeshes;
	*(meshes + numMeshes) = d;
	numMeshes++;

	return hMesh;
}

HMESH load_mesh(const char* meshName, const char* mtlFileName) {
	obj_data od = read_obj_file(meshName, mtlFileName);
	drawable d = obj_to_drawable(&od);
	d.hProgram = prog;
	free_obj_data(&od);

	return add_drawable(d);
}

object_model* create_model_from_mesh(HMESH hMesh) {
	object_model om = { 0 };
	om.hModel = hMesh;
	
	om.position = (vec3f){ 0 };
	om.rotation = (vec3f){ 0 };
	om.scale = (vec3f){ 1, 1, 1 };
	om.visible = 1;

	if (objectModels == NULL) {
		objectModelSpace = 10;
		objectModels = calloc(objectModelSpace, sizeof(object_model));
	}

	if (objectModelSpace <= numObjectModels + 1) {
		objectModelSpace *= 2;
		objectModels = realloc(objectModels, objectModelSpace * sizeof(object_model));
	}

	*(objectModels + numObjectModels) = om;
	numObjectModels++;

	return (objectModels + numObjectModels - 1);
}

HMESH init_dynamic_mesh(GLenum drawType) {
	drawable d = { 0 };
	d.hProgram = prog;
	d.drawType = drawType;

	d.vao = calloc(1, sizeof(GLuint));
	d.vbo = calloc(3, sizeof(GLuint));

	glGenVertexArrays(1, d.vao);
	glBindVertexArray(*d.vao);
	glGenBuffers(3, d.vbo);
	d.bloomThreshold = threshold;

	HMESH hMesh = add_drawable(d);

	return hMesh;
}

void edit_mesh_indices(HMESH hMesh, int* indices, int numIndices) {
	drawable* d = (meshes + hMesh);

	glBindVertexArray(*d->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *(d->vbo + 2));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(int), indices, GL_DYNAMIC_DRAW);
}

void edit_mesh_positions(HMESH hMesh, vec3f* positions, int numPositions) {
	drawable* d = (meshes + hMesh);

	glBindVertexArray(*d->vao);
	glBindBuffer(GL_ARRAY_BUFFER, *(d->vbo + 0));
	glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(vec3f), positions, GL_DYNAMIC_DRAW);
}

void edit_mesh_normals(HMESH hMesh, vec3f* normals, int numNormals) {
	drawable* d = (meshes + hMesh);

	glBindVertexArray(*d->vao);
	glBindBuffer(GL_ARRAY_BUFFER, *(d->vbo + 1));
	glBufferData(GL_ARRAY_BUFFER, numNormals * sizeof(vec3f), normals, GL_DYNAMIC_DRAW);
}

void edit_mesh_mtl_data(HMESH hMesh, mtllib mlib, int* groupBounds, int numGroups) {
	drawable* d = (meshes + hMesh);

	d->materialBounds = groupBounds;
	d->numMaterials = numGroups;
	d->materials = mlib;
}

void draw_skybox(mat4f perspectiveMatrix) {
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	static mat4f cameraMatrix;
	cameraMatrix = rotate_xyz(-currentCamera.rotation.x, -currentCamera.rotation.y, -currentCamera.rotation.z);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glUseProgram(skyboxProg);
	static GLuint gldLoc, scLoc;
	//static GLuint bgcLoc;
	gldLoc = glGetUniformLocation(skyboxProg, "globalLightDir");
	//bgcLoc = glGetUniformLocation(gskyboxProg, "backgroundColor");
	scLoc = glGetUniformLocation(skyboxProg, "sunColor");

	glUniform3f(gldLoc, lightDir.x, lightDir.y, lightDir.z);
	//glUniform3f(bgcLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);
	glUniform3f(scLoc, sunColor.x, sunColor.y, sunColor.z);

	draw_model(&skyboxOM, &skybox, perspectiveMatrix, cameraMatrix, skyboxTexture);
}

void draw_plane() {
	glBindVertexArray(planeVAO);
	CHECK_GL_ERRORS;

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	CHECK_GL_ERRORS;

	glDrawArrays(GL_QUADS, 0, 4);
	CHECK_GL_ERRORS;
}

unsigned int numDownloads = 0;
int dx = 0;
unsigned char* ptr;
float draw(int dWidth, int dHeight) {
	if (initialized) {
		static mat4f perspectiveMatrix;
		float dt = timer_reset(&t);
		onFrame(dt);
		updateEmitters(dt);

		glViewport(0, 0, bufferWidth, bufferHeight);

		// calculate universal matrices for this frame
		static mat4f cameraMatrix;

		//cameraMatrix = from_position_and_rotation(inverse_vec3f(currentCamera.position), inverse_vec3f(currentCamera.rotation));
		cameraMatrix = mat_mul_mat(
			rotate_xyz(-currentCamera.rotation.x, -currentCamera.rotation.y, -currentCamera.rotation.z),
			from_translation(-currentCamera.position.x, -currentCamera.position.y, -currentCamera.position.z)
		);
		cameraMatrix = mat_mul_mat(new_identity(), cameraMatrix);

		perspectiveMatrix = new_perspective(5, 10000, fov, ((float)dWidth / (float)dHeight));

		// render bloom highlights first
		glBindFramebuffer(GL_FRAMEBUFFER, bfbo);

		// clear buffers
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glUseProgram(prog);
		static GLuint skyboxLoc, bgcLoc, gldLoc;

		gldLoc = glGetUniformLocation(prog, "globalLightDir");
		skyboxLoc = glGetUniformLocation(prog, "skyboxHandle");
		bgcLoc = glGetUniformLocation(prog, "backgroundColor");

		for (int i = 0; i < numObjectModels; i++) {
			object_model om = *(objectModels + i);
			drawable d = *(meshes + (int)om.hModel);

			glUniform3f(gldLoc, lightDir.x, lightDir.y, lightDir.z);
			glUniform3f(bgcLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);
			glUniform1i(skyboxLoc, skyboxTexture);

			d.bloomThreshold = threshold;
			draw_model(&om, &d, perspectiveMatrix, cameraMatrix, skyboxTexture);
		}
		render_emitters(cameraMatrix, perspectiveMatrix, 0);

		// blur the bloom highlights
		glUseProgram(blurProg);

		GLuint swLoc, shLoc, sampLoc, osLoc, blurDirLoc;
		swLoc = glGetUniformLocation(blurProg, "sWidth");
		shLoc = glGetUniformLocation(blurProg, "sHeight");
		sampLoc = glGetUniformLocation(blurProg, "sampleNum");
		osLoc = glGetUniformLocation(blurProg, "offsetScale");
		blurDirLoc = glGetUniformLocation(blurProg, "blurDir");

		glUniform1i(swLoc, bufferWidth);
		glUniform1i(shLoc, bufferHeight);
		glUniform1i(sampLoc, bloomSize);
		glUniform1f(osLoc, bloomOffsetScale);
		glUniform2f(blurDirLoc, 1, 0); // horizontal blur first

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, btex);

		glBindFramebuffer(GL_FRAMEBUFFER, bfbo2);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		draw_plane();

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, btex2);

		glUniform2f(blurDirLoc, 0, 1); // vertical blur second

		// override original threshold-only image with fully blurred image
		glBindFramebuffer(GL_FRAMEBUFFER, bfbo);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		draw_plane();


		/* render normal scene to temp fbo */
		glBindFramebuffer(GL_FRAMEBUFFER, tfbo);

		// clear buffers
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		//draw the skybox 
		draw_skybox(perspectiveMatrix, cameraMatrix);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// draw models
		for (int i = 0; i < numObjectModels; i++) {
			object_model om = *(objectModels + i);
			drawable d = *(meshes + (int)om.hModel);

			d.bloomThreshold = 0;
			draw_model(&om, &d, perspectiveMatrix, cameraMatrix, skyboxTexture);
		}

		render_emitters(cameraMatrix, perspectiveMatrix, 1);

		/* combine the temp fbo and the bloom fbo */
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glClearColor(0, 1.0f, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDepthFunc(GL_ALWAYS);

		glUseProgram(bloomProg);
		GLuint msaaLoc, numSamplesLoc, iLoc;
		msaaLoc = glGetUniformLocation(bloomProg, "msaa");
		swLoc = glGetUniformLocation(bloomProg, "sWidth");
		shLoc = glGetUniformLocation(bloomProg, "sHeight");
		iLoc = glGetUniformLocation(bloomProg, "intensity");
		
		glUniform1i(msaaLoc, antialiased ? MSAASamples : 0);
		glUniform1i(swLoc, bufferWidth);
		glUniform1i(shLoc, bufferHeight);
		glUniform1f(iLoc, intensity);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, btex);

		if (antialiased) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ttex);
		}
		else {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, ttex);
		}

		draw_plane();

		if (!antialiased) {
			SwapBuffers(glContextHDC);
		}

		/* */
		memset(lpBits, 0, 4 * bufferWidth * bufferHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		if (numDownloads < numPBOs) {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + dx));
			CHECK_GL_ERRORS;
			glReadPixels(0, 0, bufferWidth, bufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
			CHECK_GL_ERRORS;
		}
		else {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + dx));
			ptr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);

			if (ptr != NULL) {
				int nBytes = bufferWidth * bufferHeight * 4;
				memcpy(lpBits, ptr, nBytes);
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
				glReadPixels(0, 0, bufferWidth, bufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
			}
			else {
				CHECK_GL_ERRORS;
			}
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		++dx;
		dx = dx % numPBOs;

		numDownloads++;
		if (numDownloads == 200000) {
			numDownloads = numPBOs;
		}

		CHECK_GL_ERRORS;

		return dt;
	}

	return 0;
}

void display(HDRAWDIB hdd, HDC hdc, int dWidth, int dHeight, int destPosX, int destPosY) {
	if (initialized) {
		static HBITMAP hbmp;
		static HDC hdcMem;
		static HGDIOBJ hOld;

		BITMAPINFOHEADER bih = { 0 };
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bufferWidth;
		bih.biHeight = bufferHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = 0;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;
		
		DrawDibDraw(
			hdd, hdc,
			destPosX, destPosY,  // dest pos 
			dWidth, dHeight, // dest size 
			&bih,
			lpBits,
			0, 0, // source pos 
			bufferWidth, bufferHeight, // source size
			NULL);
	}
}

void end() {
	free(lpBits);

	free_drawable(&skybox);

	for (int i = 0; i < numMeshes; i++) {
		free_drawable(meshes + i);
		CHECK_GL_ERRORS;
	}

	free(meshes);
	free(objectModels);

	glDeleteBuffers(1, &framebuffer);
	glDeleteBuffers(1, &rbo);
	glDeleteBuffers(numPBOs, pbo);
	glDeleteTextures(1, &tcb);

	glDeleteBuffers(1, &brbo);
	glDeleteBuffers(1, &bfbo);
	glDeleteTextures(1, &btex);

	glDeleteBuffers(1, &brbo2);
	glDeleteBuffers(1, &bfbo2);
	glDeleteTextures(1, &btex2);

	glDeleteBuffers(1, &trbo);
	glDeleteBuffers(1, &tfbo);
	glDeleteTextures(1, &ttex);

	glDeleteProgram(prog);
	glDeleteProgram(skyboxProg);
	glDeleteProgram(gskyboxProg);
	glDeleteProgram(blurProg);
	glDeleteProgram(bloomProg);
}
