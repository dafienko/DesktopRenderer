#include "desktopPainter.h"

#define WM_CREATE_WORKER_WINDOW 0x052C

HWND workerw;
HWND hProgMan;
HWND hShellDefView;

void getProgMan() {
	hProgMan = FindWindow(TEXT("Progman"), NULL);
}

void createWorkerWindow() {
	int* result = 0;
	SendMessageTimeout(hProgMan,
		WM_CREATE_WORKER_WINDOW,
		NULL,
		NULL,
		SMTO_NORMAL,
		1000,
		(PDWORD_PTR)(&result));
}

void findWorkerWindow() {
	EnumWindows(enumChildWindowProc, NULL);
}

BOOL CALLBACK enumChildWindowProc(HWND hTopWindow, LPARAM lParam)
{
	hShellDefView = FindWindowEx(hTopWindow,
		NULL,
		TEXT("SHELLDLL_DefView"),
		NULL);

	if (hShellDefView != NULL) {
		workerw = FindWindowEx(NULL,
			hTopWindow,
			TEXT("WorkerW"),
			NULL);
	}

	return TRUE;
};

void paintDesktop() {
	//HDC hdc = GetDCEx(workerw, NULL, 0x403);
	static HDC hdc;
	hdc = GetDC(workerw);

	if (hdc != NULL) {
		on_paint_desktop(hdc);
	}

	ReleaseDC(workerw, hdc);
}