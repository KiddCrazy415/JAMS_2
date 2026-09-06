#include "Theme.h"
#include <windows.h>

// Theme constants are defined in Theme.h

void InitializeJAMSTheme()
{
	gThemeFont = CreateFontW(
		-16,
		0,
		0,
		0,
		FW_SEMIBOLD,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Segoe UI"
	);

	gLogoFont = CreateFontW(
		-150,
		0,
		-8,
		0,
		FW_BOLD,
		TRUE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Segoe UI"
	);

	gBackgroundBrush =
		CreateSolidBrush(
			JAMSTHEME::Background
		);

	gPanelBrush =
		CreateSolidBrush(
			JAMSTHEME::Panel
		);

	gEditBrush =
		CreateSolidBrush(
			JAMSTHEME::Panel2
		);
}

void ShutdownJAMSTheme()
{
	if (gThemeFont)
		DeleteObject(gThemeFont);

	if (gLogoFont)
		DeleteObject(gLogoFont);

	if (gBackgroundBrush)
		DeleteObject(gBackgroundBrush);

	if (gPanelBrush)
		DeleteObject(gPanelBrush);

	if (gEditBrush)
		DeleteObject(gEditBrush);
}

void DrawNeonLogo(
	HDC hdc,
	const RECT& rect
)
{
	if (!gLogoFont)
		return;

	RECT logoRect = rect;

	logoRect.left += 390;
	logoRect.right -= 390;
	logoRect.top += 205;
	logoRect.bottom = logoRect.top + 190;

	SetBkMode(
		hdc,
		TRANSPARENT
	);

	HFONT oldFont =
		static_cast<HFONT>(
			SelectObject(
				hdc,
				gLogoFont
			)
		);

	const int offsets[] =
	{
		14,
		10,
		7,
		4,
		2,
		0
	};

	const COLORREF glowColors[] =
	{
		RGB(18, 12, 65),
		RGB(25, 18, 105),
		RGB(0, 55, 125),
		RGB(0, 100, 190),
		RGB(0, 170, 235),
		RGB(0, 220, 255)
	};

	for (int i = 0; i < 6; ++i)
	{
		RECT r = logoRect;

		r.left += offsets[i];
		r.top += offsets[i] / 2;
		r.right += offsets[i];
		r.bottom += offsets[i] / 2;

		SetTextColor(
			hdc,
			glowColors[i]
		);

		DrawTextW(
			hdc,
			L"JAMS",
			-1,
			&r,
			DT_CENTER |
			DT_VCENTER |
			DT_SINGLELINE |
			DT_NOPREFIX
		);
	}

	RECT purpleRect = logoRect;

	purpleRect.left += 3;
	purpleRect.top += 5;
	purpleRect.right += 3;
	purpleRect.bottom += 5;

	SetTextColor(
		hdc,
		JAMSTHEME::Purple
	);

	DrawTextW(
		hdc,
		L"JAMS",
		-1,
		&purpleRect,
		DT_CENTER |
		DT_VCENTER |
		DT_SINGLELINE |
		DT_NOPREFIX
	);

	SetTextColor(
		hdc,
		JAMSTHEME::Cyan
	);

	DrawTextW(
		hdc,
		L"JAMS",
		-1,
		&logoRect,
		DT_CENTER |
		DT_VCENTER |
		DT_SINGLELINE |
		DT_NOPREFIX
	);

	SelectObject(
		hdc,
		oldFont
	);
}

void DrawNeonBackground(
	HDC hdc,
	const RECT& rect
)
{
	FillRect(
		hdc,
		&rect,
		gBackgroundBrush
	);

	RECT center = rect;

	center.left = 405;
	center.right = 1180;
	center.top = 90;
	center.bottom = 585;

	HBRUSH panel =
		CreateSolidBrush(
			RGB(7, 10, 21)
		);

	FillRect(
		hdc,
		&center,
		panel
	);

	DeleteObject(
		panel
	);

	HPEN cyanPen =
		CreatePen(
			PS_SOLID,
			1,
			RGB(0, 70, 105)
		);

	HPEN oldPen =
		static_cast<HPEN>(
			SelectObject(
				hdc,
				cyanPen
			)
		);

	MoveToEx(
		hdc,
		410,
		92,
		nullptr
	);

	LineTo(
		hdc,
		1175,
		92
	);

	MoveToEx(
		hdc,
		410,
		585,
		nullptr
	);

	LineTo(
		hdc,
		1175,
		585
	);

	SelectObject(
		hdc,
		oldPen
	);

	DeleteObject(
		cyanPen
	);

	DrawNeonLogo(
		hdc,
		rect
	);
}

void DrawThemedButton(
	const DRAWITEMSTRUCT* dis
)
{
	if (!dis || !dis->hwndItem)
		return;

	HDC hdc = dis->hDC;
	RECT r = dis->rcItem;

	bool pressed =
		(dis->itemState & ODS_SELECTED) != 0;

	bool disabled =
		(dis->itemState & ODS_DISABLED) != 0;

	COLORREF accent =
		JAMSTHEME::Cyan;

	int id =
		GetDlgCtrlID(
			dis->hwndItem
		);

	if (id == IDC_DELETE)
	{
		accent =
			JAMSTHEME::Magenta;
	}
	else if (
		id == IDC_UP ||
		id == IDC_DOWN
	)
	{
		accent =
			JAMSTHEME::Green;
	}

	// Background
	HBRUSH fill =
		CreateSolidBrush(
			gPanelBrush ? JAMSTHEME::Panel2 : JAMSTHEME::Panel
		);

	FillRect(
		hdc,
		&r,
		fill
	);

	DeleteObject(fill);

	// Border
	HPEN pen =
		CreatePen(
			PS_SOLID,
			1,
			JAMSTHEME::Grid
		);

	HPEN oldPen =
		static_cast<HPEN>(SelectObject(hdc, pen));

	RECT br = r;
	br.left += 1;
	br.top += 1;
	br.right -= 1;
	br.bottom -= 1;

	FrameRect(hdc, &br, CreateSolidBrush(JAMSTHEME::Grid));

	SelectObject(hdc, oldPen);
	DeleteObject(pen);

	// Accent bar
	RECT a = r;
	a.left += 1;
	a.top += 1;
	a.bottom = a.top + 6;

	FillRect(hdc, &a, CreateSolidBrush(accent));

	// Text
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, disabled ? JAMSTHEME::MutedText : JAMSTHEME::Text);

	wchar_t text[128]{};
	GetWindowTextW(dis->hwndItem, text, _countof(text));

	DrawTextW(hdc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}
