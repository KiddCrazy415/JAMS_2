#pragma once

#include <windows.h>

void DrawPreview(HDC hdc, RECT clientRect);

LRESULT CALLBACK PreviewProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

void UpdatePreview();
