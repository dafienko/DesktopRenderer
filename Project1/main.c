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

#define RENDER_TO_WINDOW 0

int biggestWidth, biggestHeight;

BOOL compare_monitor_dimensions(HMONITOR hMonitor, HDC hdc, LPRECT pRect, LPARAM lParam) {
	biggestWidth = max(biggestWidth, pRect->right - pRect->left);
	biggestHeight = max(biggestHeight, pRect->bottom - pRect->top);
}

void get_biggest_monitor_size() {
	biggestWidth = 0;
	biggestHeight = 0;

	EnumDisplayMonitors(NULL, NULL, compare_monitor_dimensions, NULL);
}

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

	float scale = 1.0f;

	get_biggest_monitor_size();

	vec2i wndSize = { 0 };
	if (RENDER_TO_WINDOW) {
		wndSize = (vec2i){ 800, 600 };
	}
	else {
		wndSize = (vec2i){ biggestWidth * scale, biggestHeight * scale };
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
	
	init(wndSize.x * scale, wndSize.y * scale);

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

	unsigned long lastSecond = get_current_ms();
	unsigned long last = lastSecond;
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

			draw(w, h);
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
		int secondDiff = now - lastSecond;
		last = now;

		if (secondDiff >= 1000) {
			lastFPS = numFrames;

			numFrames = 0;
			lastSecond = now;
		}	

		int timeToWait = 23 - diff;
		if (timeToWait > 0) {
			Sleep(timeToWait);
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



HDC desktopHDC;
BOOL paint_monitor(HMONITOR hMonitor, HDC hdc, LPRECT pRect, LPARAM lParam) {
	display(hdd, desktopHDC, pRect->right - pRect->left, pRect->bottom - pRect->top, pRect->left, pRect->top);
}

void on_paint_desktop(HDC dHDC) {
	desktopHDC = dHDC;

	draw(biggestWidth, biggestHeight);

	EnumDisplayMonitors(NULL, NULL, paint_monitor, NULL);
}