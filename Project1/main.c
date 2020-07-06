#include "winUtil.h"
#include "glExtensions.h"
#include "desktopPainter.h"
#include "renderer.h"
#include "program.h"
#include "assetLoader.h"
#include <Vfw.h>
#include "timer.h"

#pragma comment(lib, "Vfw32.lib")

WNDPROC mainWndProc(HWND, UINT, WPARAM, LPARAM);

HBITMAP hbmp;
HDC painterDC;
HWND hMainWnd, hOpenglWnd; 

#define RENDER_TO_WINDOW 1

HDRAWDIB hdd;
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	WNDCLASS wndClass = { 0 };
	wndClass.style = CS_OWNDC;
	wndClass.lpfnWndProc = mainWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = NULL;
	wndClass.hCursor = NULL;
	wndClass.hbrBackground = GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"Independent Window";

	RegisterClass(&wndClass);

	float scale = 1;

	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int sWidth = GetSystemMetrics(SM_CXSCREEN);
	int sHeight = GetSystemMetrics(SM_CYSCREEN);

	vec2i wndSize = { 0 };
	if (RENDER_TO_WINDOW) {
		wndSize = (vec2i){ 800, 600 };
	}
	else {
		wndSize = (vec2i){ sWidth / scale, sHeight / scale };
	}

	hMainWnd = create_independent_window(L"Main Window", &wndSize, NULL, &wndClass);

	hOpenglWnd = create_child_opengl_window(hInstance, L"Opengl Window", hMainWnd);
	HDC hdc = GetDC(hOpenglWnd);
	HGLRC hrc = create_rc_from_window(&hdc);
	use_rc(&hdc, &hrc);

	getProgMan();
	createWorkerWindow();
	findWorkerWindow();

	GLEInit(); // must be initialized after a context has been made current
	
	init(wndSize.x / scale, wndSize.y / scale);

	if (RENDER_TO_WINDOW) {
		ShowWindow(hMainWnd, nShowCmd);
	}

	hdd = DrawDibOpen();
	if (hdd == NULL) {
		error("Couldn't init a drawdib");
	}
	
	programInit();

	int exitCode = run_message_loop();

	programClose();

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(hdc, hOpenglWnd);
	wglDeleteContext(hrc);

	end();

	return exitCode;
}

int run_message_loop() {
	MSG msg;
	HDC hdc;
	RECT rect;

	unsigned long last = get_current_ms();
	int numFrames = 0;
	int lastFPS = 0;

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				return msg.wParam;
			}
		}

		
		if (RENDER_TO_WINDOW) {
			hdc = GetDC(hMainWnd);
			GetWindowRect(hMainWnd, &rect);
			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;

			display(hdd, hdc, w, h, 0, 0);

			char* text = calloc(50, sizeof(char));
			sprintf_s(text, 50, "%ifps", lastFPS);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(0, 180, 0));
			TextOutA(hdc, 0, 0, text, strlen(text));
			free(text);

			ReleaseDC(hMainWnd, hdc);
		}
		else {
			paintDesktop();
		}
		numFrames++;

		unsigned long now = get_current_ms();
		int diff = now - last;
		if (diff >= 1000) {
			lastFPS = numFrames;

			numFrames = 0;
			last = now;
		}	
	}
}

WNDPROC mainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static PAINTSTRUCT ps;
	static HDC hdc;
	static RECT rect;

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(69);
		return 0;
	case WM_SIZE:
		GetClientRect(hMainWnd, &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;

		SetWindowPos(hOpenglWnd, HWND_TOP, 0, 0, w, h, SWP_HIDEWINDOW);


		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void on_paint_desktop(HDC desktopHDC) {

	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	int sWidth = GetSystemMetrics(SM_CXSCREEN);
	int sHeight = GetSystemMetrics(SM_CYSCREEN);
	
	display(hdd, desktopHDC, sWidth, sHeight, 0, 0);
	display(hdd, desktopHDC, sWidth, sHeight, sWidth, 0);
}