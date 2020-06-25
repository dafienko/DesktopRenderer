#include "keyboard.h"
#include <Windows.h>

int isKeyDown(int nVirtKey) {
	return GetKeyState(nVirtKey) < 0;
}