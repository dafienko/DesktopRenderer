#include "renderer.h"
#include "assetLoader.h"
#include "glExtensions.h"
#include "matrixMath.h"
#include "stringUtil.h"
#include "program.h"
#include "timer.h"
#include "winUtil.h"

#ifndef PI
#define PI 3.14159265359
#endif
timer t = { 0 };

int vpWidth = 0;
int vpHeight = 0;
int vWidth, vHeight;
GLuint prog;
GLuint rbo = 0;
GLuint tcb = 0;
GLuint framebuffer = 0;
drawable d = { 0 };

float* floatBuffer = NULL;
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

int renderToRenderBuffer = 0;

void init() {
	GLuint vs = create_vertex_shader("shaders\\basic.vs");
	GLuint fs = create_fragment_shader("shaders\\basic.fs");
	GLuint shaders[] = { vs, fs };
	prog = create_program(shaders, 2);
	
	obj_data od = get_obj_data("models\\monkey.obj");

	d = obj_to_drawable(&od);
	d.hProgram = prog;
	free_obj_data(&od);

	timer_start(&t);

	currentCamera = (camera){ 0 };
	currentCamera.position = (vec3f){ 0.0f, 0, 0 };
	currentCamera.rotation = (vec3f){ 0, 0, 0 };

	wglSwapIntervalEXT(0);

	if (renderToRenderBuffer) {
		glGenFramebuffers(1, &framebuffer);

		glGenTextures(1, &tcb);
		glBindTexture(GL_TEXTURE_2D, tcb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tcb, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}


	vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	floatBuffer = calloc(4 * vWidth * vHeight, sizeof(float));
	lpBits = calloc(4 * vWidth * vHeight, sizeof(unsigned char));

	initialized = 1;
}

void resize(int width, int height) {
	glViewport(0, 0, width, height);

	if (renderToRenderBuffer) {
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	}

	vpWidth = width;
	vpHeight = height;
}

static int glOffset, bmOffset;
static float b, g, r, a;
void float_image_to_char_image(int width, int height) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			glOffset = y * width + x;
			b = *(floatBuffer + glOffset * 4 + 0);
			g = *(floatBuffer + glOffset * 4 + 1);
			r = *(floatBuffer + glOffset * 4 + 2);
			a = *(floatBuffer + glOffset * 4 + 3);

			bmOffset = ((height - 1) - y) * width + x;
			*(lpBits + bmOffset * 4 + 0) = (unsigned char)(b * 255);
			*(lpBits + bmOffset * 4 + 1) = (unsigned char)(g * 255);
			*(lpBits + bmOffset * 4 + 2) = (unsigned char)(r * 255);
			*(lpBits + bmOffset * 4 + 3) = (unsigned char)(a * 255);
		}
	}
}

float draw(int dWidth, int dHeight) {
	float dt = timer_reset(&t);
	onFrame(dt);

	int fps = (int)(1.0f / dt);

	/* draw instances to framebuffer */
	static mat4f cameraMatrix;

	glClearColor(.5f, .5f, .5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	cameraMatrix = from_position_and_rotation(inverse_vec3f(currentCamera.position), inverse_vec3f(currentCamera.rotation));
	cameraMatrix = mat_mul_mat(new_identity(), cameraMatrix);

	perspectiveMatrix = new_perspective(2, 10000, 70.0f, ((float)dWidth / (float)dHeight));

	drawable_draw(&d, perspectiveMatrix, cameraMatrix);

	SwapBuffers(glContextHDC);

	
	return dt;
}

void display(HDC hdc, int dWidth, int dHeight) {
	if (initialized) {
		static HBITMAP hbmp;
		static HDC hdcMem;
		static HGDIOBJ hOld;

		float dt = draw(dWidth, dHeight);

		/* bitblt framebuffer to hdc */
		///*
		
		memset(floatBuffer, 0, 4 * vpWidth * vpHeight);
		memset(lpBits, 0, 4 * vpWidth * vpHeight);

		glReadPixels(0, 0, vpWidth, vpHeight, GL_BGRA, GL_FLOAT, floatBuffer);
		float_image_to_char_image(vpWidth, vpHeight);
		hbmp = CreateCompatibleBitmap(hdc, vpWidth, vpHeight);
		SetBitmapBits(hbmp, vpWidth * vpHeight * 4, lpBits);

		hdcMem = CreateCompatibleDC(hdc);
		hOld = SelectObject(hdcMem, hbmp);
		DeleteObject(hOld);
		

		//BitBlt(hdc, 0, 0, vpWidth, vpHeight, hdcMem, 0, 0, SRCCOPY);
		StretchBlt(hdc, 0, 0, dWidth, dHeight, hdcMem, 0, 0, vpWidth, vpHeight, SRCCOPY);
	
		DeleteDC(hdcMem);
		DeleteObject(hbmp);

		char* text = calloc(50, sizeof(char));
		sprintf_s(text, 50, "dt: %0.5f", dt);
		TextOutA(hdc, 100, 20, text, strlen(text));
		free(text);
		//*/
		 
		
	}
}

void end() {
	free(floatBuffer);
	free(lpBits);
}