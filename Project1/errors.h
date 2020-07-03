void check_errors(const char* fileName, int lineNumber);
void check_std_err(const char* desc, const int e);
void check_gl_err(const char* file, const int line);
void error(char* desc);

#define CHECK_GL_ERRORS check_gl_err(__FILE__, __LINE__)
#define CHECK_ERRORS check_errors(__FILE__, __LINE__)