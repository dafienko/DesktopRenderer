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

int bufferWidth, bufferHeight;
GLuint prog, skyboxProg, blurProg, ssaaProg;
GLuint rbo, tcb;
GLuint aarbo, aafbo, aatex;
GLuint aavao, aavbo;
GLuint tex;

int numPBOs = 3;
GLuint* pbo = NULL;
GLuint framebuffer = 0;

GLuint lightbo = 0;

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

pointlight pl = { 0 };

int antialiased = 1;
int scale = 2;

void init(int width, int height) {
	int sampleSize = 2; // samples per pixel is sampleSize^2


	GLuint vs = create_vertex_shader("shaders\\brdf.vs");
	GLuint fs = create_fragment_shader("shaders\\brdf.fs");
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
	
	obj_data cubeObj= get_obj_data("models\\cube.obj");
	skybox = obj_to_drawable(&cubeObj);
	skybox.hProgram = skyboxProg;
	skybox.scale = (vec3f){ 10.0f, 10.0f, 10.0f };
	skybox.position = (vec3f){ 0, 0, 0 };
	skybox.rotation = (vec3f){ 0, 0, 0 };
	free_obj_data(&cubeObj);

	obj_data modelObj = get_obj_data("models\\sphere.obj");
	model = obj_to_drawable(&modelObj);
	model.hProgram = prog;
	model.scale = (vec3f){ 2.0f, 2.0f, 2.0f };
	model.position = (vec3f){ 0, 0, -4.0f };
	model.rotation = (vec3f){ 0, 0, 0 };
	free_obj_data(&modelObj);

	timer_start(&t);

	currentCamera = (camera){ 0 };
	currentCamera.position = (vec3f){ 0.0f, 0, 8.0f };
	currentCamera.rotation = (vec3f){ 0, 0, 0 };

	wglSwapIntervalEXT(0);

	lpBits = calloc(4 * width * height, sizeof(unsigned char));

	if (antialiased) {
		glGenTextures(1, &aatex);
		glBindTexture(GL_TEXTURE_2D, aatex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width * scale, scale * height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// create a renderbuffer object to store depth info
		glGenRenderbuffers(1, &aarbo);
		glBindRenderbuffer(GL_RENDERBUFFER, aarbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width * scale, scale * height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// create a framebuffer object
		glGenFramebuffers(1, &aafbo);
		glBindFramebuffer(GL_FRAMEBUFFER, aafbo);

		// attach the texture to FBO color attachment point
		glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER
			GL_COLOR_ATTACHMENT0,  // 2. attachment point
			GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
			aatex,				   // 4. tex ID
			0);                    // 5. mipmap level: 0(base)

		// attach the renderbuffer to depth attachment point
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
			aarbo);              // 4. rbo ID

		// check FBO status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			MessageBoxA(NULL, "Error", "You screwed something up making a framebuffer", MB_OK);
			exit(69);
		}

		// switch back to window-system-provided framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/* generate plane vertex object */
		glGenVertexArrays(1, &aavao);
		glBindVertexArray(aavao);

		glGenBuffers(1, &aavbo);
		glBindBuffer(GL_ARRAY_BUFFER, aavbo);
		float p = 1.0f;
		vec2f qPositions[] = {
			{-p, -p},
			{p, -p},
			{p, p},
			{-p, p}
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f) * 4, qPositions, GL_STATIC_DRAW);
	}


	glGenFramebuffers(1, &framebuffer);

	glGenTextures(1, &tcb);
	glBindTexture(GL_TEXTURE_2D, tcb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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


	skyboxTexture = create_skybox_texture("assets/skybox.png");

	maxPointLights = 500;
	pointlights = calloc(maxPointLights, sizeof(pointlight*));
	pl.position = (vec4f){ 4.0f, 0.0f, -4.0f, 0.0f };
	pl.color = (vec4f){ 1.0f, 0.0f, 0.0f, 1.0f };
	pl.intensity = 1.0f;
	pl.range = 10.0f;
	*(pointlights + 0) = &pl;

	initialized = 1;
}

pointlight* get_closest_pointlights(vec4f position) {
	pointlight closestPointlights[MAX_POINTLIGHTS_PER_OBJECT];
	float minDist = 0;
	float maxDist = 999999.0f;

	for (int i = 0; i < MAX_POINTLIGHTS_PER_OBJECT; i++) {
		/* Pointer to a Pointer of a PointLight */
		for (pointlight* pppl = *pointlights; pppl != NULL;  pppl++ ) {
			float dist = magnitude_4f(vector_sub_4f(pppl->position, position));
			if (dist > minDist&& dist < maxDist) {
				maxDist = dist;
				closestPointlights[i] = *pppl;
			}
		}

		minDist = magnitude_4f(vector_sub_4f(closestPointlights[i].position, position));
	}

	return closestPointlights;
}

void buffer_pointlight_data(vec4f position) {
	pointlight buffer[MAX_POINTLIGHTS_PER_OBJECT];
	pointlight* closestPointlights = get_closest_pointlights(position);
	
	int numPointlights = 0;
	for (pointlight* pppl = *pointlights; pppl != NULL;  pppl++) {
		if (numPointlights >= MAX_POINTLIGHTS_PER_OBJECT || pppl == NULL) {
			break;
		}
		
		buffer[numPointlights] = *(closestPointlights + numPointlights);

		numPointlights++;
	}

	if (numPointlights > 0) {
		glBindBuffer(GL_UNIFORM_BUFFER, lightbo);
		glBufferData(GL_UNIFORM_BUFFER, numPointlights * sizeof(pointlight), buffer, GL_DYNAMIC_COPY);
	}

}

void resize(int width, int height) {
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	bufferWidth = width;
	bufferHeight = height;
}

void draw_skybox(mat4f perspectiveMatrix) {
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	static mat4f cameraMatrix;
	cameraMatrix = rotate_xyz(-currentCamera.rotation.x, -currentCamera.rotation.y, -currentCamera.rotation.z);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	drawable_draw(&skybox, perspectiveMatrix, cameraMatrix, skyboxTexture);
}

float fov = 50.0f;

float draw(int dWidth, int dHeight) {
	float dt = timer_reset(&t);
	onFrame(dt);

	int fps = (int)(1.0f / dt);

	// calculate universal matrices for this frame
	static mat4f cameraMatrix;

	cameraMatrix = from_position_and_rotation(inverse_vec3f(currentCamera.position), inverse_vec3f(currentCamera.rotation));
	cameraMatrix = mat_mul_mat(new_identity(), cameraMatrix);

	perspectiveMatrix = new_perspective(.1, 10000, fov, ((float)dWidth / (float)dHeight));

	// draw everything else 
	glClearColor(.1f, .1f, .1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//draw the skybox 
	draw_skybox(perspectiveMatrix, cameraMatrix);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	int cx = 5;
	int cy = 5;
	float s = 2.1f;
	for (int y = 0; y < cx; y++) {
		for (int x = 0; x < cy; x++) {
			model.position = (vec3f){s * (x - ((cx-1) / 2.0f)), s * (y - ((cy-1) / 2.0f)), -4.0f};
			model.scale = (vec3f){ s / 2, s / 2, s / 2};
			
			model.material.color = (vec3f){ 1.0f, 0.0f, 0.0f };
			model.material.ao = .15f;
			model.material.metallic = min(1, y / (float)cy + .05f);
			model.material.roughness = min(1, x / (float)cx + .05f);

			drawable_draw(&model, perspectiveMatrix, cameraMatrix, skyboxTexture);
		}
	}


	if (!antialiased) {
		SwapBuffers(glContextHDC);
	}

	return dt;
}

unsigned int numDownloads = 0;
int dx = 0;
unsigned char* ptr;
void display(HDRAWDIB hdd, HDC hdc, int dWidth, int dHeight) {
	if (initialized) {
		static HBITMAP hbmp;
		static HDC hdcMem;
		static HGDIOBJ hOld;

		if (antialiased) {
			glBindFramebuffer(GL_FRAMEBUFFER, aafbo);
			glViewport(0, 0, dWidth * scale, dHeight * scale);
		}
		else {
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		}

		float dt = draw(dWidth, dHeight);

		if (antialiased) {
			glViewport(0, 0, dWidth, dHeight);

			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			
			glClearColor(0, 1.0f, 0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glDepthFunc(GL_ALWAYS);

			glUseProgram(ssaaProg);

			GLuint sLoc, swLoc, shLoc, sampLoc;
			sLoc = glGetUniformLocation(ssaaProg, "scale");
			swLoc = glGetUniformLocation(ssaaProg, "sWidth");
			shLoc = glGetUniformLocation(ssaaProg, "sHeight");
			sampLoc = glGetUniformLocation(ssaaProg, "sampleNum");

			glUniform1f(sLoc, (float)scale);
			glUniform1i(swLoc, dWidth * scale);
			glUniform1i(shLoc, dHeight * scale);
			glUniform1i(sampLoc, scale);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, aatex);


			glBindVertexArray(aavao);

			glBindBuffer(GL_ARRAY_BUFFER, aavbo);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

			glDrawArrays(GL_QUADS, 0, 4);
			SwapBuffers(glContextHDC);
		}
		CHECK_GL_ERRORS;

		memset(lpBits, 0, 4 * bufferWidth * bufferHeight);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
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
			0, 0,  // dest pos 
			dWidth, dHeight, // dest size 
			&bih,
			lpBits,
			0, 0, // source pos 
			bufferWidth, bufferHeight, // source size
			NULL);

		char* text = calloc(50, sizeof(char));
		int fps = (int)(1.0f / dt);
		sprintf_s(text, 50, "%ifps", fps);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 180, 0));
		TextOutA(hdc, 0, 0, text, strlen(text));
		free(text);

		 
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
}