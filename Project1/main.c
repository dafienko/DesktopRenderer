#include "winUtil.h"
#include "glExtensions.h"
#include "desktopPainter.h"
#include "renderer.h"
#include "program.h"
#include <Vfw.h>

#pragma comment(lib, "Vfw32.lib")

WNDPROC mainWndProc(HWND, UINT, WPARAM, LPARAM);

HBITMAP hbmp;
HDC painterDC;
HWND hMainWnd, hOpenglWnd; 

HDRAWDIB hdd;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
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



	int scale = 1;

	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	
	vec2i wndSize = (vec2i){ vWidth / scale, vHeight / scale };

	

	hMainWnd = create_independent_window(L"Main Window", &wndSize, NULL, &wndClass);

	hOpenglWnd = create_child_opengl_window(hInstance, L"Opengl Window", hMainWnd);
	HDC hdc = GetDC(hOpenglWnd);
	HGLRC hrc = create_rc_from_window(&hdc);
	use_rc(&hdc, &hrc);

	getProgMan();
	createWorkerWindow();
	findWorkerWindow();

	GLEInit(); // must be initialized after a context has been made current
	
	init();

	resize(wndSize.x, wndSize.y);
	//ShowWindow(hMainWnd, nShowCmd);
	//ShowWindow(hOpenglWnd, nShowCmd);


	hdd = DrawDibOpen();
	if (hdd == NULL) {
		error("Couldn't init an hdrawdib");
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

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				return msg.wParam;
			}
		}

		GetClientRect(hOpenglWnd, &rect);
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;
		//draw(w, h);

		///*
		hdc = GetDC(hMainWnd);
		GetClientRect(hMainWnd, &rect);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
		display(hdd, hdc, w, h);
		ReleaseDC(hMainWnd, hdc);
		//*/

		paintDesktop();
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
		//resize(LOWORD(lParam), HIWORD(lParam));
		
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
	///*
	int vWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	//*/
	/*
	int vWidth = GetSystemMetrics(SM_CXSCREEN);
	int vHeight = GetSystemMetrics(SM_CYSCREEN);
	*/

	display(hdd, desktopHDC, vWidth, vHeight);
}