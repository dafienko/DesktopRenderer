#pragma once

#include "winUtil.h"

#pragma comment(lib, "gdi32")

HWND create_independent_window(LPCWSTR wndName, vec2i* size, vec2i* pos, WNDCLASS* wndClass) {
	/*default null arguments*/
	if (size == NULL) {
		vec2i sizeVar = { 640, 480 };
		size = &sizeVar;
	}
	if (pos == NULL) {
		vec2i posVar;
		vec2i screenDimensions = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
		vec2i remainingDimensions = vector_sub_2i(screenDimensions, *size);
		posVar = vector_div_2i(remainingDimensions, 2);
		pos = &posVar;
	}

	/*create the window*/
	HWND hWnd = CreateWindow(wndClass->lpszClassName, wndName,
		WS_OVERLAPPEDWINDOW, 
		pos->x, pos->y,
		size->x, size->y,
		NULL, NULL, wndClass->hInstance, NULL);

	if (!hWnd) {
		CHECK_ERRORS;
		return NULL;
	}

	return hWnd;
}

LPCSTR openglWndClassName = "openglWindow";
WNDCLASS openglWndClass = { 0 };
int openglWndClassInitialized = 0;

void initOpenglWndClass(HINSTANCE hInstance) {
	openglWndClass.style = CS_OWNDC;
	openglWndClass.lpfnWndProc = DefWindowProc;
	openglWndClass.cbClsExtra = 0;
	openglWndClass.cbWndExtra = 0;
	openglWndClass.hInstance = hInstance;
	openglWndClass.hIcon = NULL;
	openglWndClass.hCursor = NULL;
	openglWndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
	openglWndClass.lpszMenuName = NULL;
	openglWndClass.lpszClassName = openglWndClassName;

	if (!RegisterClass(&openglWndClass)) {
		CHECK_ERRORS;
	}

	openglWndClassInitialized = 1;
}

void setup_window_for_opengl(HWND hWnd) {
	HDC hdc;
	hdc = GetDC(hWnd);

	setup_hdc_for_opengl(hdc);
	CHECK_ERRORS;

	ReleaseDC(hWnd, hdc);
	CHECK_ERRORS;
}

void setup_hdc_for_opengl(HDC hdc) {
	CHECK_ERRORS;
	PIXELFORMATDESCRIPTOR pfd;
	int pf;

	/* there is no guarantee that the contents of the stack that become
	   the pfd are zeroed, therefore _make sure_ to clear these bits. */
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(hdc, &pfd);
	if (pf == 0) {
		CHECK_ERRORS;
		return 0;
	}

	if (SetPixelFormat(hdc, pf, &pfd) == FALSE) {
		CHECK_ERRORS;
		return 0;
	}

	DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	CHECK_ERRORS;
}

HWND create_independent_opengl_window(LPCWSTR name, vec2i* size, vec2i* pos, WNDCLASS* wndClass) {
	/*default null arguments*/
	if (size == NULL) {
		vec2i sizeVar = { 640, 480 };
		size = &sizeVar;
	}
	if (pos == NULL) {
		vec2i posVar;
		vec2i screenDimensions = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
		vec2i remainingDimensions = vector_sub_2i(screenDimensions, *size);
		posVar = vector_div_2i(remainingDimensions, 2);
		pos = &posVar;
	}

	HWND hWnd = CreateWindow(wndClass->lpszClassName, name, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		pos->x, pos->y,
		size->x, size->y,
		NULL, NULL, wndClass->hInstance, NULL);

	if (hWnd == NULL) {
		CHECK_ERRORS;
	}

	setup_window_for_opengl(hWnd);

	return hWnd;
}


HWND create_child_opengl_window(HINSTANCE hInstance, LPCWSTR name, HWND hParentWnd) {
	if (!openglWndClassInitialized) {
		initOpenglWndClass(hInstance);
	}

	RECT rect;
	GetWindowRect(hParentWnd, &rect);

	HWND hWnd = CreateWindowW(openglWndClassName, name, WS_CHILD,
		0, 0,
		(rect.right - rect.left), (rect.bottom - rect.top),
		hParentWnd, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		CHECK_ERRORS;
		return NULL;
	}

	setup_window_for_opengl(hWnd);

	return hWnd;
}

