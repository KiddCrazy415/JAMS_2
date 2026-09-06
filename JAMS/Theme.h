#pragma once

#include "Globals.h"

namespace JAMSTHEME
{
	constexpr COLORREF Background = RGB(5, 7, 14);
	constexpr COLORREF Panel = RGB(10, 13, 24);
	constexpr COLORREF Panel2 = RGB(14, 18, 32);

	constexpr COLORREF Cyan = RGB(0, 220, 255);
	constexpr COLORREF Blue = RGB(40, 120, 255);
	constexpr COLORREF Purple = RGB(150, 50, 255);
	constexpr COLORREF Magenta = RGB(255, 30, 180);
	constexpr COLORREF Green = RGB(0, 255, 120);
	constexpr COLORREF Orange = RGB(255, 150, 30);

	constexpr COLORREF Text = RGB(235, 245, 255);
	constexpr COLORREF MutedText = RGB(145, 165, 190);
	constexpr COLORREF Grid = RGB(25, 55, 85);
	constexpr COLORREF Selection = RGB(0, 105, 175);
}

void InitializeJAMSTheme();
void ShutdownJAMSTheme();

void DrawNeonLogo(HDC hdc, const RECT& rect);
void DrawNeonBackground(HDC hdc, const RECT& rect);
void DrawThemedButton(const DRAWITEMSTRUCT* dis);
