#include "Globals.h"
#include "ListEdit.h"

#include <windows.h>
#include <commctrl.h>
#include <algorithm>

LRESULT CALLBACK CellEditProc(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (
		message == WM_KEYDOWN &&
		wParam == VK_RETURN
		)
	{
		CommitCellEdit();
		return 0;
	}

	if (
		message == WM_KEYDOWN &&
		wParam == VK_ESCAPE
		)
	{
		cancelCellEdit = true;

		CancelCellEdit();

		return 0;
	}

	if (originalCellEditProc)
	{
		return CallWindowProcW(
			originalCellEditProc,
			window,
			message,
			wParam,
			lParam
		);
	}

	return DefWindowProcW(
		window,
		message,
		wParam,
		lParam
	);
}


void BeginCellEdit(
	int row,
	int column
)
{
	if (!hList)
		return;

	if (
		column < 1 ||
		column > 3
		)
	{
		return;
	}

	if (
		row < 0 ||
		row >=
		static_cast<int>(
			pattern.size()
			)
		)
	{
		return;
	}

	if (hCellEdit)
		CommitCellEdit();

	RECT rect{};

	if (
		!ListView_GetSubItemRect(
			hList,
			row,
			column,
			LVIR_BOUNDS,
			&rect
		)
		)
	{
		return;
	}

	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;

	wchar_t text[64]{};

	if (column == 1)
	{
		swprintf_s(
			text,
			L"%d",
			pattern[row].dx
		);
	}
	else if (column == 2)
	{
		swprintf_s(
			text,
			L"%d",
			pattern[row].dy
		);
	}
	else
	{
		swprintf_s(
			text,
			L"%d",
			pattern[row].sleepMs
		);
	}

	RECT rectScreen = rect;
	MapWindowPoints(
		hList,
		hwnd,
		reinterpret_cast<POINT*>(&rectScreen),
		2
	);

	LONG editLeft = rectScreen.left - 3;
	LONG editTop = rectScreen.top - 2;
	LONG editWidth = (rectScreen.right - rectScreen.left) + 6;
	LONG editHeight = (rectScreen.bottom - rectScreen.top) + 4;

	RECT mainClient{};
	GetClientRect(hwnd, &mainClient);

	if (editLeft < 0) editLeft = 0;
	if (editTop < 0) editTop = 0;
	if (editLeft + editWidth > mainClient.right)
		editWidth = mainClient.right - editLeft;
	if (editTop + editHeight > mainClient.bottom)
		editHeight = mainClient.bottom - editTop;

	hCellEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			text,
			WS_CHILD |
			WS_VISIBLE |
			WS_TABSTOP |
			WS_CLIPSIBLINGS |
			ES_AUTOHSCROLL,
			editLeft,
			editTop,
			editWidth,
			editHeight,
			hwnd,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_CELL_EDIT
					)
				),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	if (hCellEdit)
	{
		SetWindowPos(
			hCellEdit,
			HWND_TOP,
			editLeft,
			editTop,
			editWidth,
			editHeight,
			SWP_SHOWWINDOW
		);
	}

	if (!hCellEdit)
		return;

	editingRow = row;
	editingColumn = column;
	cancelCellEdit = false;

	HFONT font =
		gThemeFont
		? gThemeFont
		: static_cast<HFONT>(
			GetStockObject(
				DEFAULT_GUI_FONT
			)
			);

	SendMessageW(
		hCellEdit,
		WM_SETFONT,
		reinterpret_cast<WPARAM>(
			font
			),
		TRUE
	);

	{
		HDC hdc = GetDC(hCellEdit);

		if (hdc)
		{
			HGDIOBJ prev = SelectObject(hdc, font);

			SIZE sz{};

			if (wcslen(text) > 0)
			{
				GetTextExtentPoint32W(hdc, text, static_cast<int>(wcslen(text)), &sz);
			}
			else
			{
				const wchar_t sample[] = L"-1234567890";
				GetTextExtentPoint32W(hdc, sample, static_cast<int>(wcslen(sample)), &sz);
			}

			int desiredWidth = sz.cx + 20;
			int desiredHeight = sz.cy + 12;

			if (desiredWidth > editWidth || desiredHeight > editHeight)
			{
				int newW = std::max<int>(editWidth, desiredWidth);
				int newH = std::max<int>(editHeight, desiredHeight);

				SetWindowPos(
					hCellEdit,
					HWND_TOP,
					editLeft,
					editTop,
					newW,
					newH,
					SWP_SHOWWINDOW
				);
			}

			SelectObject(hdc, prev);
			ReleaseDC(hCellEdit, hdc);
		}
	}

	originalCellEditProc =
		reinterpret_cast<WNDPROC>(
			SetWindowLongPtrW(
				hCellEdit,
				GWLP_WNDPROC,
				reinterpret_cast<LONG_PTR>(
					CellEditProc
					)
			)
			);

	SetFocus(
		hCellEdit
	);

	SendMessageW(
		hCellEdit,
		EM_SETSEL,
		0,
		-1
	);
}


void CommitCellEdit()
{
	if (!hCellEdit)
		return;

	if (cancelCellEdit)
	{
		CancelCellEdit();
		return;
	}

	int row =
		editingRow;

	int column =
		editingColumn;

	if (
		row >= 0 &&
		row <
		static_cast<int>(
			pattern.size()
			) &&
		column >= 1 &&
		column <= 3
		)
	{
		wchar_t buffer[64]{};

		GetWindowTextW(
			hCellEdit,
			buffer,
			64
		);

		wchar_t* end = nullptr;

		long value =
			wcstol(
				buffer,
				&end,
				10
			);

		if (end != buffer)
		{
			if (value > 1000000)
				value = 1000000;

			if (value < -1000000)
				value = -1000000;

			if (column == 1)
			{
				pattern[row].dx =
					static_cast<int>(
						value
						);
			}
			else if (column == 2)
			{
				pattern[row].dy =
					static_cast<int>(
						value
						);
			}
			else
			{
				if (value < 0)
					value = 0;

				if (value > 200)
					value = 200;

				pattern[row].sleepMs =
					static_cast<int>(
						value
						);
			}

			ResetPattern();
		}
	}

	HWND edit =
		hCellEdit;

	hCellEdit = nullptr;

	editingRow = -1;
	editingColumn = -1;

	cancelCellEdit = false;

	if (originalCellEditProc)
	{
		SetWindowLongPtrW(
			edit,
			GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(
				originalCellEditProc
				)
		);

		originalCellEditProc =
			nullptr;
	}

	DestroyWindow(
		edit
	);

	RefreshList();

	if (
		row >= 0 &&
		row <
		static_cast<int>(
			pattern.size()
			)
	)
	{
		ListView_SetItemState(
			hList,
			row,
			LVIS_SELECTED |
			LVIS_FOCUSED,
			LVIS_SELECTED |
			LVIS_FOCUSED
		);

		ListView_EnsureVisible(
			hList,
			row,
			FALSE
		);
	}

	SetFocus(
		hList
	);
}


void CancelCellEdit()
{
	if (!hCellEdit)
		return;

	HWND edit =
		hCellEdit;

	hCellEdit = nullptr;

	editingRow = -1;
	editingColumn = -1;

	cancelCellEdit = false;

	if (originalCellEditProc)
	{
		SetWindowLongPtrW(
			edit,
			GWLP_WNDPROC,
			reinterpret_cast<LONG_PTR>(
				originalCellEditProc
				)
		);

		originalCellEditProc =
			nullptr;
	}

	DestroyWindow(
		edit
	);

	SetFocus(
		hList
	);
}
