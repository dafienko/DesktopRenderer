#include "winUtil.h"
#include "glExtensions.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "errors.h"

#pragma comment(lib, "opengl32")

void* getProc(const char* name) {
    void* p = wglGetProcAddress(name);
    if (p == NULL) {
        CHECK_ERRORS;
        return NULL;
    }
    if(p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1) )
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        if (module == NULL) {
            CHECK_ERRORS;
            return NULL;
        }
        p = GetProcAddress(module, name);
        if (p == NULL) {
            CHECK_ERRORS;
            return NULL;
        }
    }

    return p;
}

void GLEInit() {
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProc("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)getProc("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)getProc("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)getProc("glCompileShader");
    glAttachShader = (PFNGLATTACHSHADERPROC)getProc("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)getProc("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)getProc("glUseProgram");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProc("glGetShaderInfoLog");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getProc("glGetProgramInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)getProc("glDetachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)getProc("glDetachShader");
    glIsProgram = (PFNGLISPROGRAMPROC)getProc("glIsProgram");
    glGenBuffers = (PFNGLGENBUFFERSPROC)getProc("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)getProc("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)getProc("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getProc("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getProc("glVertexAttribPointer");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)getProc("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)getProc("glBindVertexArray");
    glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)getProc("glGetIntegeri_v");
    glGetStringi = (PFNGLGETSTRINGIPROC)getProc("glGetStringi");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)getProc("wglSwapIntervalEXT");
    wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)getProc("wglGetSwapIntervalEXT");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)getProc("glUniformMatrix4fv");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getProc("glGetUniformLocation");
    glUniform3f = (PFNGLUNIFORM3FPROC)getProc("glUniform3f");
    glUniform1i = (PFNGLUNIFORM1IPROC)getProc("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)getProc("glUniform1f");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)getProc("glGetShaderiv");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getProc("glGetProgramiv");
    glIsProgram = (PFNGLISPROGRAMPROC)getProc("glIsProgram");
    glIsShader = (PFNGLISSHADERPROC)getProc("glIsShader");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)getProc("glGenRenderbuffers");
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)getProc("glBindRenderbuffer");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)getProc("glRenderbufferStorage");
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)getProc("glBindRenderbuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getProc("glFramebufferTexture2D");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)getProc("glFramebufferRenderbuffer");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getProc("glBindFramebuffer");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getProc("glGenFramebuffers");
    glMapBuffer = (PFNGLMAPBUFFERPROC)getProc("glMapBuffer");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)getProc("glUnmapBuffer");

    initialized = 1;
}
