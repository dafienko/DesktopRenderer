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

int vpWidth = 0;
int vpHeight = 0;
int vWidth, vHeight;
GLuint prog, skyboxProg;
GLuint rbo, tcb;

int numPBOs = 3;
GLuint* pbo = NULL;
GLuint framebuffer = 0;

GLuint skyboxTexture;

drawable skybox = { 0 };
drawable model = { 0 };

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

int renderToRenderBuffer = 1;
int renderToPixelbuffer = 1;

image_bit_data ibd = { 0 };

image_bit_data ibd, top;

void init() {
	GLuint vs = create_vertex_shader("shaders\\basic.vs");
	GLuint fs = create_fragment_shader("shaders\\basic.fs");
	GLuint shaders[] = { vs, fs };
	prog = create_program(shaders, 2);

	GLuint sbvs = create_vertex_shader("shaders\\skybox.vs");
	GLuint sbfs = create_fragment_shader("shaders\\skybox.fs");
	GLuint shaders2[] = { sbvs, sbfs };
	skyboxProg = create_program(shaders2, 2);
	
	obj_data cubeObj = get_obj_data("models\\cube.obj");
	skybox = obj_to_drawable(&cubeObj);
	skybox.hProgram = skyboxProg;
	skybox.scale = (vec3f){ 10.0f, 10.0f, 10.0f };
	skybox.position = (vec3f){ 0, 0, 0 };
	skybox.rotation = (vec3f){ 0, 0, 0 };
	free_obj_data(&cubeObj);

	obj_data modelObj = get_obj_data("models\\monkeySmooth.obj");
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

	vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	floatBuffer = calloc(4 * vWidth * vHeight, sizeof(float));
	lpBits = calloc(4 * vWidth * vHeight, sizeof(unsigned char));

	if (renderToRenderBuffer) {
		glGenFramebuffers(1, &framebuffer);

		glGenTextures(1, &tcb);
		glBindTexture(GL_TEXTURE_2D, tcb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vWidth, vHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, vWidth, vHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tcb, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}
	if (renderToPixelbuffer) {
		pbo = calloc(numPBOs, sizeof(GLuint));
		glGenBuffers(numPBOs, pbo);


		for (int i = numPBOs - 1; i >= 0; i--) {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + i));
			glBufferData(GL_PIXEL_PACK_BUFFER, vWidth * vHeight * 4, NULL, GL_STREAM_READ);
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	skyboxTexture = create_skybox_texture("assets/skybox.png");

	ibd = read_png_file_simple("assets/skybox.png");
	top = get_image_rect(ibd, 512, 0, 512, 512);

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
void float_image_to_char_image(int width, int height, int reverse) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			glOffset = y * width + x;
			b = *(floatBuffer + glOffset * 4 + 0);
			g = *(floatBuffer + glOffset * 4 + 1);
			r = *(floatBuffer + glOffset * 4 + 2);
			a = *(floatBuffer + glOffset * 4 + 3);

			if (reverse) {
				bmOffset = ((height - 1) - y) * width + x;
			}
			else {
				bmOffset = glOffset;
			}
			*(lpBits + bmOffset * 4 + 0) = (unsigned char)(b * 255);
			*(lpBits + bmOffset * 4 + 1) = (unsigned char)(g * 255);
			*(lpBits + bmOffset * 4 + 2) = (unsigned char)(r * 255);
			*(lpBits + bmOffset * 4 + 3) = (unsigned char)(a * 255);
		}
	}
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

	/* calculate universal matrices for this frame*/
	static mat4f cameraMatrix;

	cameraMatrix = from_position_and_rotation(inverse_vec3f(currentCamera.position), inverse_vec3f(currentCamera.rotation));
	cameraMatrix = mat_mul_mat(new_identity(), cameraMatrix);

	perspectiveMatrix = new_perspective(2, 10000, fov, ((float)dWidth / (float)dHeight));

	/* draw everything else */
	glClearColor(.5f, .5f, .5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	/* draw the skybox */
	draw_skybox(perspectiveMatrix, cameraMatrix);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	drawable_draw(&model, perspectiveMatrix, cameraMatrix, skyboxTexture);

	SwapBuffers(glContextHDC);
	
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

		float dt = draw(dWidth, dHeight);

		memset(floatBuffer, 0, 4 * vpWidth * vpHeight);
		memset(lpBits, 0, 4 * vpWidth * vpHeight);
		
		if (renderToPixelbuffer) {
			if (numDownloads < numPBOs) {
				glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + dx));
				CHECK_GL_ERRORS;
				glReadPixels(0, 0, vpWidth, vpHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
				CHECK_GL_ERRORS;
			}
			else {
				glBindBuffer(GL_PIXEL_PACK_BUFFER, *(pbo + dx));
				ptr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);

				if (ptr != NULL) {
					int nBytes = vpWidth * vpHeight * 4;
					memcpy(lpBits, ptr, nBytes);
					glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
					glReadPixels(0, 0, vpWidth, vpHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
				}
				else {
					CHECK_GL_ERRORS;
				}
			}

			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}
		else {
			glReadPixels(0, 0, vpWidth, vpHeight, GL_BGRA, GL_FLOAT, floatBuffer);
			float_image_to_char_image(vpWidth, vpHeight, 0);
		}

		

		BITMAPINFOHEADER bih = { 0 };
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = vpWidth;
		bih.biHeight = vpHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = 0;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;
		
		///*
		DrawDibDraw(
			hdd, hdc,
			0, 0,  // dest pos 
			dWidth, dHeight, // dest size 
			&bih,
			lpBits,
			0, 0, // source pos 
			vpWidth, vpHeight, // source size
			DDF_NOTKEYFRAME);
		//*/

		/*
		//hbmp = CreateCompatibleBitmap(hdc, vpWidth, vpHeight);
		hbmp = CreateBitmap(top.width, top.height, 1, 32, top.lpBits);
		////SetBitmapBits(hbmp, ibd.height * ibd.width * 4, ibd.lpBits);

		hdcMem = CreateCompatibleDC(hdc);
		hOld = SelectObject(hdcMem, hbmp);
		DeleteObject(hOld);


		BitBlt(hdc, 0, 0, top.width, top.height, hdcMem, 0, 0, SRCCOPY);
		////StretchBlt(hdc, 0, 0, dWidth, dHeight, hdcMem, 0, 0, vpWidth, vpHeight, SRCCOPY);

		DeleteDC(hdcMem);
		DeleteObject(hbmp);
		*/


		char* text = calloc(50, sizeof(char));
		sprintf_s(text, 50, "dt: %0.5f", dt);
		TextOutA(hdc, 100, 20, text, strlen(text));
		free(text);
		//*/
		 
		++dx;
		dx = dx % numPBOs;

		numDownloads++;
		if (numDownloads == 200000) {
			numDownloads = numPBOs;
		}
	}
}

void end() {
	free(floatBuffer);
	free(lpBits);
}