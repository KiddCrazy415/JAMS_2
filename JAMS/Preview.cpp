#include "Preview.h"
#include "Globals.h"
#include "Theme.h"

#include <windows.h>
#include <algorithm>
#include <vector>

// Ensure Windows' min/max macros don't collide with std::min/std::max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

void DrawPreview(HDC hdc, RECT clientRect)
{
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	if (width <= 0 || height <= 0)
		return;

	HBRUSH backgroundBrush = CreateSolidBrush(JAMSTHEME::Background);
	FillRect(hdc, &clientRect, backgroundBrush);
	DeleteObject(backgroundBrush);

	HBRUSH borderBrush = CreateSolidBrush(JAMSTHEME::Cyan);
	FrameRect(hdc, &clientRect, borderBrush);
	DeleteObject(borderBrush);

	std::vector<POINT> points;
	points.push_back({0,0});

	long long x=0,y=0, minX=0,maxX=0,minY=0,maxY=0;
	for (const Movement& movement : pattern)
	{
		x += movement.dx; y += movement.dy;
		minX = std::min(minX, x); maxX = std::max(maxX, x);
		minY = std::min(minY, y); maxY = std::max(maxY, y);
		POINT p; p.x = static_cast<LONG>(-x); p.y = static_cast<LONG>(-y);
		points.push_back(p);
	}

	const int padding = 30;
	double availableWidth = static_cast<double>(width - padding*2);
	double availableHeight = static_cast<double>(height - padding*2);
	if (availableWidth < 10) availableWidth = 10;
	if (availableHeight < 10) availableHeight = 10;

	double rangeX = static_cast<double>(maxX - minX);
	double rangeY = static_cast<double>(maxY - minY);
	if (rangeX < 1.0) rangeX = 1.0;
	if (rangeY < 1.0) rangeY = 1.0;

	double scaleX = availableWidth / rangeX;
	double scaleY = availableHeight / rangeY;
	double scale = std::min(scaleX, scaleY);
	if (scale <= 0.0) scale = 1.0;
	scale *= 0.90;

	const double startScreenX = static_cast<double>(width) / 2.0;
	const double startScreenY = static_cast<double>(height - padding);

	auto ConvertPoint = [&](const POINT& p){ POINT r{}; double px=static_cast<double>(p.x), py=static_cast<double>(p.y); r.x = static_cast<LONG>(startScreenX + px*scale); r.y = static_cast<LONG>(startScreenY + py*scale); return r; };

	HPEN gridPen = CreatePen(PS_SOLID,1,JAMSTHEME::Grid);
	HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, gridPen));
	MoveToEx(hdc, width/2, padding, nullptr); LineTo(hdc, width/2, height - padding);
	MoveToEx(hdc, padding, height - padding, nullptr); LineTo(hdc, width - padding, height - padding);
	SelectObject(hdc, oldPen); DeleteObject(gridPen);

	if (points.size() >= 2)
	{
		HPEN pathPen = CreatePen(PS_SOLID,2,JAMSTHEME::Cyan);
		oldPen = static_cast<HPEN>(SelectObject(hdc, pathPen));
		POINT first = ConvertPoint(points[0]); MoveToEx(hdc, first.x, first.y, nullptr);
		for (size_t i=1;i<points.size();++i){ POINT p = ConvertPoint(points[i]); LineTo(hdc, p.x, p.y); }
		SelectObject(hdc, oldPen); DeleteObject(pathPen);
	}

	POINT start = ConvertPoint(points.front());
	HBRUSH startBrush = CreateSolidBrush(JAMSTHEME::Green); HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, startBrush)); Ellipse(hdc, start.x-5, start.y-5, start.x+5, start.y+5); SelectObject(hdc, oldBrush); DeleteObject(startBrush);

	POINT finish = ConvertPoint(points.back());
	HBRUSH finishBrush = CreateSolidBrush(JAMSTHEME::Magenta); oldBrush = static_cast<HBRUSH>(SelectObject(hdc, finishBrush)); Ellipse(hdc, finish.x-5, finish.y-5, finish.x+5, finish.y+5); SelectObject(hdc, oldBrush); DeleteObject(finishBrush);

	SetBkMode(hdc, TRANSPARENT);
	HFONT font = gThemeFont ? gThemeFont : static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)); HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
	SetTextColor(hdc, JAMSTHEME::Green); TextOutW(hdc, start.x+8, start.y-20, L"START", 5);
	SetTextColor(hdc, JAMSTHEME::Magenta); TextOutW(hdc, finish.x+8, finish.y-8, L"END", 3);
	SelectObject(hdc, oldFont);
}

void UpdatePreview()
{
	if (hPreview)
	{
		InvalidateRect(hPreview, nullptr, TRUE);
		UpdateWindow(hPreview);
	}
}

LRESULT CALLBACK PreviewProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps{}; HDC hdc = BeginPaint(window, &ps); RECT rect{}; GetClientRect(window, &rect); DrawPreview(hdc, rect); EndPaint(window, &ps); return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	default:
		return DefWindowProcW(window, message, wParam, lParam);
	}
}
