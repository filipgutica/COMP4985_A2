#ifndef EZ_UI_H
#define EZ_UI_H

#include <Windows.h>
#include <CommCtrl.h>
#include "Menu.h"


HWND CreateLabel(char*, int, int, int, int, HWND hwnd);
HWND CreateEditCtrl(char *text, int x, int y, int w, int h, HWND hwnd);
HWND CreateTextBox(int x, int y, int w, int h, HWND hwnd);
HWND CreateRadioBtn(char *text, int x, int y, int w, int h, HMENU id, HWND hwnd);
HWND CreateBtn(char *text, int x, int y, int w, int h, HMENU id, HWND hwnd);
HWND CreateDropeDownList(int x, int y, int w, int h, HWND hwnd);

#endif