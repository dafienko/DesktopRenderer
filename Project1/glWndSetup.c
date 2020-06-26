#include "winUtil.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "errors.h"

HGLRC create_rc_from_window(HDC* hdc) {
	HGLRC hrc;
	hrc = wglCreateContext(*hdc);
	
	if (hrc == NULL) {
		CHECK_ERRORS;
	}

	return hrc;
}