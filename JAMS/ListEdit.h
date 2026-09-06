#pragma once

#include <windows.h>

void BeginCellEdit(int row, int column);
void CommitCellEdit();
void CancelCellEdit();

LRESULT CALLBACK CellEditProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
