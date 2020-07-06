#include "renderer.h"
#include "assetLoader.h"
#include "glExtensions.h"
#include "matrixMath.h"
#include "stringUtil.h"
#include "program.h"
#include "timer.h"
#include "winUtil.h"
#include "errors.h"

#pragma comment(lib, "Vfw32.lib")

#ifndef PI
#define PI 3.14159265359
#endif
timer t = { 0 };

/* general settings */
vec3f lightDir = { 1, -1, 0 };
vec3f backgroundColor = { 1, 0, 0 };

/* MSAA settings */
int antialiased = 1;
int MSAASamples = 2;

/* Bloom settings */
float threshold = .8f;
int bloomSize = 15;
float bloomOffsetScale = 4.0f;
float intensity = 4.0f;

int bufferWidth, bufferHeight;
GLuint prog, skyboxProg, blurProg, ssaaProg, bloomProg;
GLuint rbo, tcb;
GLuint brbo, bfbo, btex;
GLuint trbo, tfbo, ttex;

GLuint planeVAO, planeVBO;

int numPBOs = 3;
GLuint* pbo = NULL;
GLuint framebuffer = 0;

GLuint skyboxTexture;

drawable skybox = { 0 };
drawable model = { 0 };

unsigned char* lpBits = NULL;

int initialized = 0;

mat4f perspectiveMatrix = { 0 };

HDC glContextHDC;
void use_rc(HDC* hdc, HGLRC* hrc) {
	BOOL success = wglMakeCurrent(*hdc, *hrc);
	glContextHDC = *hdc;

	if (success == FALSE) {
		CHECK_ERRORS;
	}
}

float fov = 70.0f;

void set_fov(const float newFov) {
	fov = newFov;
}

float get_fov() {
	return fov;
}

void set_current_skybox(GLuint tex) {
	skyboxTexture = tex;
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

	GLuint ssvs = create_vertex_shader("shaders\\ssaa.vs");
	GLuint ssfs = create_fragment_shader("shaders\\ssaa.fs");
	GLuint ssshaders[] = { ssvs, ssfs };
	ssaaProg = create_program(ssshaders, 2);

	GLuint sbvs = create_vertex_shader("shaders\\skybox.vs");
	GLuint sbfs = create_fragment_shader("shaders\\skybox.fs");
	GLuint shaders2[] = { sbvs, sbfs };
	skyboxProg = create_program(shaders2, 2);

	GLuint bloomvs = create_vertex_shader("shaders\\bloom.vs");
	GLuint bloomfs = create_fragment_shader("shaders\\bloom.fs");
	GLuint bloomshaders[] = { bloomvs, bloomfs };
	bloomProg = create_program(bloomshaders, 2);

	//obj_data od = read_obj_file("models\\small_complex_world.obj", "models\\small_complex_world.mtl");
	//obj_data od = read_obj_file("models\\hexWorld.obj", "models\\hexWorld.mtl");
	//obj_data od = read_obj_file("models\\cursed_world.obj", "models\\cursed_world.mtl");
	//obj_data od = read_obj_file("models\\sphere.obj", NULL);
	obj_data od = read_obj_file("models\\emitters.obj", "models\\emitters.mtl");
	model = obj_to_drawable(&od);
	free_obj_data(&od);
	model.hProgram = prog;
	model.scale = (vec3f){ .2, .2, .2 };
	model.rotation = (vec3f){ 0, PI/2, 0 };

	od = read_obj_file("models\\cube.obj", NULL);
	skybox = obj_to_drawable(&od);
	free_obj_data(&od);
	skybox.hProgram = skyboxProg;
	skybox.scale = (vec3f){ 10.0f, 10.0f, 10.0f };
	skybox.position = (vec3f){ 0, 0, 0 };
	skybox.rotation = (vec3f){ 0, 0, 0 };

	timer_start(&t);

	currentCamera = (camera){ 0 };
	currentCamera.position = (vec3f){ 0.0f, 0, 8.0f };
	currentCamera.rotation = (vec3f){ 0, 0, 0 };

	wglSwapIntervalEXT(0);

	lpBits = calloc(4 * width * height, sizeof(unsigned char));

	if (antialiased) {
		/* bloom effect buffer */
		glGenTextures(1, &btex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, btex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAASamples, GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glGenRenderbuffers(1, &brbo);
		glBindRenderbuffer(GL_RENDERBUFFER, brbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASamples, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glGenFramebuffers(1, &bfbo);
		glBindFramebuffer(GL_FRAMEBUFFER, bfbo);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D_MULTISAMPLE,
			btex,
			0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			brbo);

		/* generate temp buffers */
		glGenTextures(1, &ttex);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ttex);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAASamples, GL_RGB, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glGenRenderbuffers(1, &trbo);
		glBindRenderbuffer(GL_RENDERBUFFER, trbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASamples, GL_DEPTH24_STENCIL8, width, height);

		glGenFramebuffers(1, &tfbo);
		glBindFramebuffer(GL_FRAMEBUFFER, tfbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D_MULTISAMPLE,
			ttex,
			0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			trbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else {
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

		/* generate temp buffers */
		glGenTextures(1, &ttex);
		glBindTexture(GL_TEXTURE_2D, ttex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffers(1, &trbo);
		glBindRenderbuffer(GL_RENDERBUFFER, trbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glGenFramebuffers(1, &tfbo);
		glBindFramebuffer(GL_FRAMEBUFFER, tfbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			ttex,
			0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			trbo);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	

	/* generate plane vertex object */
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

	/* generate the framebuffer that's read from onto the screen */
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

void draw_skybox(mat4f perspectiveMatrix) {
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	static mat4f cameraMatrix;
	cameraMatrix = rotate_xyz(-currentCamera.rotation.x, -currentCamera.rotation.y, -currentCamera.rotation.z);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glUseProgram(skyboxProg);
	GLuint gldLoc = glGetUniformLocation(skyboxProg, "globalLightDir");
	glUniform3f(gldLoc, lightDir.x, lightDir.y, lightDir.z);

	if (skyboxTexture > 0) {
		drawable_draw(&skybox, perspectiveMatrix, cameraMatrix, skyboxTexture);
	}
}

float draw(int dWidth, int dHeight) {
	float dt = timer_reset(&t);
	onFrame(dt);

	// calculate universal matrices for this frame
	static mat4f cameraMatrix;

	cameraMatrix = from_position_and_rotation(inverse_vec3f(currentCamera.position), inverse_vec3f(currentCamera.rotation));
	cameraMatrix = mat_mul_mat(new_identity(), cameraMatrix);

	perspectiveMatrix = new_perspective(.1, 10000, fov, ((float)dWidth / (float)dHeight));

	glBindFramebuffer(GL_FRAMEBUFFER, bfbo);

	// clear buffers
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	model.bloomThreshold = threshold;
	glUseProgram(prog);
	static GLuint skyboxLoc, bgcLoc, gldLoc;

	gldLoc = glGetUniformLocation(prog, "globalLightDir");
	skyboxLoc = glGetUniformLocation(prog, "skyboxHandle");
	bgcLoc = glGetUniformLocation(prog, "backgroundColor");

	glUniform3f(gldLoc, lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(bgcLoc, backgroundColor.x, backgroundColor.y, backgroundColor.z);
	glUniform1i(skyboxLoc, skyboxTexture);

	drawable_draw(&model, perspectiveMatrix, cameraMatrix, skyboxTexture);
	model.bloomThreshold = 0;

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

	drawable_draw(&model, perspectiveMatrix, cameraMatrix, skyboxTexture);

	/* combine the temp fbo and the bloom fbo */
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glClearColor(0, 1.0f, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);

	glUseProgram(bloomProg);

	GLuint swLoc, shLoc, sampLoc, osLoc, iLoc, msaaLoc, numSamplesLoc;
	swLoc = glGetUniformLocation(bloomProg, "sWidth");
	shLoc = glGetUniformLocation(bloomProg, "sHeight");
	sampLoc = glGetUniformLocation(bloomProg, "sampleNum");
	osLoc = glGetUniformLocation(bloomProg, "offsetScale");
	iLoc = glGetUniformLocation(bloomProg, "intensity");
	msaaLoc = glGetUniformLocation(bloomProg, "msaa");
	numSamplesLoc = glGetUniformLocation(bloomProg, "numSamples");

	glUniform1i(swLoc, dWidth);
	glUniform1i(shLoc, dHeight);
	glUniform1i(msaaLoc, antialiased);
	glUniform1i(sampLoc, bloomSize);
	glUniform1i(numSamplesLoc, MSAASamples);
	glUniform1f(osLoc, bloomOffsetScale);
	glUniform1f(iLoc, intensity);


	if (antialiased) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, btex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ttex);
	}
	else {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, btex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ttex);
	}

	glBindVertexArray(planeVAO);
	CHECK_GL_ERRORS;

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	CHECK_GL_ERRORS;

	glDrawArrays(GL_QUADS, 0, 4);
	CHECK_GL_ERRORS;

	if (!antialiased) {
		SwapBuffers(glContextHDC);
	}

	return dt;
}

unsigned int numDownloads = 0;
int dx = 0;
unsigned char* ptr;
void display(HDRAWDIB hdd, HDC hdc, int dWidth, int dHeight, int destPosX, int destPosY) {
	if (initialized) {
		static HBITMAP hbmp;
		static HDC hdcMem;
		static HGDIOBJ hOld;
	
		glViewport(0, 0, bufferWidth, bufferHeight);
		float dt = draw(bufferWidth, bufferHeight);
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
		 
		++dx;
		dx = dx % numPBOs;

		numDownloads++;
		if (numDownloads == 200000) {
			numDownloads = numPBOs;
		}
	}
}

void end() {
	free(lpBits);
	
	free_drawable(&model);
	free_drawable(&skybox);
}