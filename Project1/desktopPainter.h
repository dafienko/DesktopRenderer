#ifndef DESKTOPPAINTER_H
#define DESKTOPPAINTER_H

#include <Windows.h>

void getProgMan();
void createWorkerWindow();
void findWorkerWindow();
BOOL CALLBACK enumChildWindowProc(HWND hTopWindow, LPARAM lParam);

void on_paint_desktop(HDC); // implemented by user, never directly called by user
void paintDesktop(); // gets the workerw hdc and calls on_paint_desktop(). this function is called by user

#endif
