#ifndef WINUTIL_H
#define WINUTIL_H

#include <Windows.h>
#include "vectorMath.h"

void check_errors(LPCSTR fileName, int lineNumber);
#define CHECK_ERRORS check_errors(__FILE__, __LINE__)

HWND create_independent_window(LPCWSTR wndName, vec2i* size, vec2i* position, WNDCLASS* wndClass);

HWND create_independent_opengl_window(LPCWSTR, vec2i* size, vec2i* position, WNDCLASS*);
HWND create_child_opengl_window(HINSTANCE, LPCWSTR, HWND);

HGLRC create_rc_from_window(HDC*);

void setup_hdc_for_opengl(HDC hdc);


#endif