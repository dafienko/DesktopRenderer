#pragma once

#include <stdio.h>
#include "winUtil.h"
#include "assetLoader.h"
#include <gl/GL.h>
#include <gl/GLU.h>

void check_errors(LPCSTR fileName, int lineNumber) {
	int e = GetLastError();

	if (e != 0) {
		LPCSTR lpMsgBuffer;

		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			e,
			NULL,
			&lpMsgBuffer,
			0,
			NULL);

		int len = strlen(lpMsgBuffer) + 500;
		char* lpDisplayBuffer = calloc(len, sizeof(char));

		sprintf_s(lpDisplayBuffer, len, "%s line %i: %s", fileName, lineNumber, lpMsgBuffer);

		MessageBoxA(NULL, lpDisplayBuffer, "Error", MB_OK);

		free(lpDisplayBuffer);

		exit(69);
	}
}

void check_std_err(const char* desc, const int e) {
	int bufferSize = 200;	
	char* buff = calloc(bufferSize, sizeof(char));
	
	strerror_s(buff, bufferSize, e);
	
	int captionSize = bufferSize + strlen(desc);
	char* mbCaption = calloc(captionSize, sizeof(char));
	sprintf_s(mbCaption, captionSize, "%s: %s\n", desc, buff);

	MessageBoxA(NULL, mbCaption, "Error", MB_OK);

	free(mbCaption);
	free(buff);
}

void check_gl_err(const char* file, const int line) {
	int e = glGetError();

	if (e != 0) {
		const char* errName = "";
		const char* errDesc = "";
		int errFound = 1;

		switch (e) {
		case GL_INVALID_ENUM:
			errName = "GL_INVALID_ENUM";
			errDesc = "An unacceptable value is specified for an enumerated argument";
			break;
		case GL_INVALID_VALUE:
			errName = "GL_INVALID_VALUE";
			errDesc = "A numeric argument is out of range";
			break;
		case GL_INVALID_OPERATION:
			errName = "GL_INVALID_OPERATION";
			errDesc = "The specified operation is not allowed in the current state";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errName = "GL_INVALID_FRAMEBUFFER_OPERATION";
			errDesc = "The framebuffer object is not complete";
			break;
		case GL_OUT_OF_MEMORY:
			errName = "GL_OUT_OF_MEMORY";
			errDesc = "There is not enough memory left to execute the command";
			break;
		case GL_STACK_UNDERFLOW:
			errName = "GL_STACK_UNDERFLOW";
			errDesc = "An attempt has been made to perform an operation that would cause an internal stack to underflow";
			break;
		case GL_STACK_OVERFLOW:
			errName = "GL_STACK_OVERFLOW";
			errDesc = "An attempt has been made to perform an operation that would cause an internal stack to overflow";
			break;
		default:
			errFound = 0;
			break;
		}

		if (errFound) {
			int bufferSize = 1000;
			char* errText = calloc(bufferSize, sizeof(char));
			sprintf_s(errText, bufferSize, "%s %i | %s:\n%s", file, line, errName, errDesc);

			MessageBoxA(NULL, errText, "Error", MB_OK);

			free(errText);
			exit(69);
		}
	}
}

void error(char* desc) {
	MessageBoxA(NULL, "Error", desc, MB_OK);
	exit(69);
}