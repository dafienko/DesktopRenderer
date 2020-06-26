#include "renderer.h"
#include "assetLoader.h"
#include "glExtensions.h"
#include "errors.h"

void check_shader_compile_errors(GLuint hShader) {
	GLint compiled = 0;
	glGetShaderiv(hShader, GL_COMPILE_STATUS, &compiled);
	
	if (compiled == GL_FALSE) { /* compilation failed */
		GLint logSize = 0;
		glGetShaderiv(hShader, GL_INFO_LOG_LENGTH, &logSize);

		char* log = calloc(logSize, sizeof(char));
		glGetShaderInfoLog(hShader, logSize, &logSize, log);

		MessageBoxA(NULL, log, "Shader Compilation Error", MB_OK);

		free(log);
	}
}

void check_program_link_errors(GLuint hProgram) {
	GLint linked = 0;
	glGetProgramiv(hProgram, GL_LINK_STATUS, &linked);

	if (linked == GL_FALSE) { /* compilation failed */
		GLint logSize = 0;
		glGetProgramiv(hProgram, GL_INFO_LOG_LENGTH, &logSize);

		char* log = calloc(logSize, sizeof(char));
		glGetProgramInfoLog(hProgram, logSize, &logSize, log);

		MessageBoxA(NULL, log, "Shader Compilation Error", MB_OK);

		free(log);
	}
}

GLuint create_shader(const char* shaderName, GLenum shaderType) {
	GLuint hShader = glCreateShader(shaderType);
	CHECK_GL_ERRORS;

	lines_data ld = get_file_lines(shaderName);

	glShaderSource(hShader, ld.numLines, ld.lines, ld.lengths);
	free_lines_data(&ld);
	CHECK_GL_ERRORS;
	
	glCompileShader(hShader);
	check_shader_compile_errors(hShader);

	return hShader;
}

GLuint create_vertex_shader(const char* shaderName) {
	return create_shader(shaderName, GL_VERTEX_SHADER);
}

GLuint create_fragment_shader(const char* shaderName) {
	return create_shader(shaderName, GL_FRAGMENT_SHADER);
}

GLuint create_program(GLuint* hShaders, int numShaders) {
	GLuint hProgram = glCreateProgram();
	CHECK_GL_ERRORS;

	GLboolean isProg = glIsProgram(hProgram);

	for (int i = 0; i < numShaders; i++) {
		GLuint hShader = *(hShaders + i);
		GLboolean isShader = glIsShader(hShader);

		glAttachShader(hProgram, hShader);
		CHECK_GL_ERRORS;
	}
	
	glLinkProgram(hProgram);
	CHECK_GL_ERRORS;
	check_program_link_errors(hProgram);

	for (int i = 0; i < numShaders; i++) {
		glDetachShader(hProgram, *(hShaders + i));
		CHECK_GL_ERRORS;
	}

	return hProgram;
}