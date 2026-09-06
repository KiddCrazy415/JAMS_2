#include "Globals.h"
#include "Theme.h"
#include "ListEdit.h"
#include "Controls.h"

#include <windows.h>
#include <commctrl.h>
#include <string>

void CreateControls(
	HWND window
)
{
	HFONT font =
		static_cast<HFONT>(
			GetStockObject(
				DEFAULT_GUI_FONT
			)
		);

	hStatus =
		CreateWindowW(
			L"STATIC",
			L"Status: WAITING",
			WS_CHILD |
			WS_VISIBLE,
			20,
			12,
			180,
			24,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	hStepStatus =
		CreateWindowW(
			L"STATIC",
			L"Step: 0 / 40",
			WS_CHILD |
			WS_VISIBLE,
			210,
			12,
			150,
			24,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"Tick Delay (ms):",
		WS_CHILD |
		WS_VISIBLE,
		20,
		42,
		120,
		24,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	hTickEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"15",
			WS_CHILD |
			WS_VISIBLE |
			ES_NUMBER |
			ES_AUTOHSCROLL,
			145,
			39,
			70,
			25,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_TICK
				)
			),
			nullptr,
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"Sub Steps:",
		WS_CHILD |
		WS_VISIBLE,
		235,
		42,
		75,
		24,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	hSubEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"5",
			WS_CHILD |
			WS_VISIBLE |
			ES_NUMBER |
			ES_AUTOHSCROLL,
			310,
			39,
			70,
			25,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_SUBSTEPS
				)
			),
			nullptr,
			nullptr
		);

	hRuntimeStatus =
		CreateWindowW(
			L"STATIC",
			L"Estimated Runtime: 2.250 seconds",
			WS_CHILD |
			WS_VISIBLE,
			20,
			72,
			210,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	hElapsedStatus =
		CreateWindowW(
			L"STATIC",
			L"Elapsed: 0.000 seconds",
			WS_CHILD |
			WS_VISIBLE,
			235,
			72,
			190,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	hRemainingStatus =
		CreateWindowW(
			L"STATIC",
			L"Remaining: 2.250 seconds",
			WS_CHILD |
			WS_VISIBLE,
			20,
			96,
			210,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	hFinishStatus =
		CreateWindowW(
			L"STATIC",
			L"Estimated Finish: --:--:--",
			WS_CHILD |
			WS_VISIBLE,
			235,
			96,
			220,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	hList =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			WC_LISTVIEWW,
			L"",
			WS_CHILD |
			WS_VISIBLE |
			LVS_REPORT |
			LVS_SINGLESEL |
			LVS_SHOWSELALWAYS,
			20,
			125,
			430,
			300,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_LIST
				)
			),
			nullptr,
			nullptr
		);

	ListView_SetExtendedListViewStyle(
		hList,
		LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES
	);

	ListView_SetBkColor(
		hList,
		JAMSTHEME::Panel
	);

	ListView_SetTextBkColor(
		hList,
		JAMSTHEME::Panel
	);

	ListView_SetTextColor(
		hList,
		JAMSTHEME::Text
	);

	LVCOLUMNW column{};

	column.mask =
		LVCF_TEXT |
		LVCF_WIDTH |
		LVCF_FMT;

	column.fmt =
		LVCFMT_CENTER;

	column.cx = 90;

	column.pszText =
		const_cast<LPWSTR>(
			L"Step"
		);

	ListView_InsertColumn(
		hList,
		0,
		&column
	);

	column.cx = 150;

	column.pszText =
		const_cast<LPWSTR>(
			L"X"
		);

	ListView_InsertColumn(
		hList,
		1,
		&column
	);

	column.cx = 120;

	column.pszText =
		const_cast<LPWSTR>(
			L"Y"
		);

	ListView_InsertColumn(
		hList,
		2,
		&column
	);

	column.cx = 120;

	column.pszText =
		const_cast<LPWSTR>(
			L"Sleep (ms)"
		);

	ListView_InsertColumn(
		hList,
		3,
		&column
	);

	hSelectedStep =
		CreateWindowW(
			L"STATIC",
			L"Editing Step 1 / 40",
			WS_CHILD |
			WS_VISIBLE,
			470,
			435,
			300,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"X:",
		WS_CHILD |
		WS_VISIBLE,
		470,
		460,
		25,
		22,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	hXSlider =
		CreateWindowExW(
			0,
			TRACKBAR_CLASSW,
			L"",
			WS_CHILD |
			WS_VISIBLE |
			TBS_AUTOTICKS |
			TBS_HORZ,
			495,
			455,
			220,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_XSLIDER
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	SendMessageW(
		hXSlider,
		TBM_SETRANGE,
		TRUE,
		MAKELONG(
			-100,
			100
		)
	);

	SendMessageW(
		hXSlider,
		TBM_SETPOS,
		TRUE,
		0
	);

	hXEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"0",
			WS_CHILD |
			WS_VISIBLE |
			ES_AUTOHSCROLL,
			720,
			455,
			55,
			25,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_XEDIT
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"Y:",
		WS_CHILD |
		WS_VISIBLE,
		470,
		495,
		25,
		22,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	hYSlider =
		CreateWindowExW(
			0,
			TRACKBAR_CLASSW,
			L"",
			WS_CHILD |
			WS_VISIBLE |
			TBS_AUTOTICKS |
			TBS_HORZ,
			495,
			490,
			220,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_YSLIDER
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	SendMessageW(
		hYSlider,
		TBM_SETRANGE,
		TRUE,
		MAKELONG(
			-100,
			100
		)
	);

	SendMessageW(
		hYSlider,
		TBM_SETPOS,
		TRUE,
		0
	);

	hYEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"0",
			WS_CHILD |
			WS_VISIBLE |
			ES_AUTOHSCROLL,
			720,
			490,
			55,
			25,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_YEDIT
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"Sleep:",
		WS_CHILD |
		WS_VISIBLE,
		800,
		460,
		55,
		22,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	hSleepSlider =
		CreateWindowExW(
			0,
			TRACKBAR_CLASSW,
			L"",
			WS_CHILD |
			WS_VISIBLE |
			TBS_AUTOTICKS |
			TBS_HORZ,
			855,
			455,
			220,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_SLEEPSLIDER
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	SendMessageW(
		hSleepSlider,
		TBM_SETRANGE,
		TRUE,
		MAKELONG(
			0,
			200
		)
	);

	SendMessageW(
		hSleepSlider,
		TBM_SETPOS,
		TRUE,
		74
	);

	hSleepEdit =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"EDIT",
			L"74",
			WS_CHILD |
			WS_VISIBLE |
			ES_AUTOHSCROLL,
			1080,
			455,
			55,
			25,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_SLEEPEDIT
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	hPreview =
		CreateWindowExW(
			WS_EX_CLIENTEDGE,
			L"CatchAllPreview",
			L"",
			WS_CHILD |
			WS_VISIBLE,
			470,
			20,
			450,
			405,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_PREVIEW
				)
			),
			GetModuleHandleW(
				nullptr
			),
			nullptr
		);

	hAddButton =
		CreateWindowW(
			L"BUTTON",
			L"Add Step",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			20,
			440,
			85,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_ADD
				)
			),
			nullptr,
			nullptr
		);

	hDeleteButton =
		CreateWindowW(
			L"BUTTON",
			L"Delete",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			115,
			440,
			85,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_DELETE
				)
			),
			nullptr,
			nullptr
		);

	hUpButton =
		CreateWindowW(
			L"BUTTON",
			L"Move Up",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			210,
			440,
			85,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_UP
				)
			),
			nullptr,
			nullptr
		);

	hDownButton =
		CreateWindowW(
			L"BUTTON",
			L"Move Down",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			305,
			440,
			95,
			30,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_DOWN
				)
			),
			nullptr,
			nullptr
		);

	hSaveButton =
		CreateWindowW(
			L"BUTTON",
			L"Save Config",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			20,
			480,
			120,
			32,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_SAVE
				)
			),
			nullptr,
			nullptr
		);

	hLoadButton =
		CreateWindowW(
			L"BUTTON",
			L"Load Config",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			150,
			480,
			120,
			32,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_LOAD
				)
			),
			nullptr,
			nullptr
		);

	hResetButton =
		CreateWindowW(
			L"BUTTON",
			L"Reset Pattern",
			WS_CHILD |
			WS_VISIBLE |
			BS_OWNERDRAW,
			280,
			480,
			120,
			32,
			window,
			reinterpret_cast<HMENU>(
				static_cast<INT_PTR>(
					IDC_RESET
				)
			),
			nullptr,
			nullptr
		);

	CreateWindowW(
		L"STATIC",
		L"Config Hotkeys",
		WS_CHILD |
		WS_VISIBLE,
		1180,
		25,
		350,
		22,
		window,
		nullptr,
		nullptr,
		nullptr
	);

	for (
		int i = 0;
		i < CONFIG_SLOT_COUNT;
		++i
		)
	{
		int y =
			55 +
			i * 50;

		int displayKey =
			(i == 9)
			? 0
			: (i + 1);

		wchar_t hotkeyText[32]{};

		swprintf_s(
			hotkeyText,
			L"Key %d",
			displayKey
		);

		CreateWindowW(
			L"STATIC",
			hotkeyText,
			WS_CHILD |
			WS_VISIBLE,
			1180,
			y + 5,
			55,
			22,
			window,
			nullptr,
			nullptr,
			nullptr
		);

		hAssignConfigButton[i] =
			CreateWindowW(
				L"BUTTON",
				L"Assign...",
				WS_CHILD |
				WS_VISIBLE |
				BS_OWNERDRAW,
				1235,
				y,
				85,
				27,
				window,
				reinterpret_cast<HMENU>(
					static_cast<INT_PTR>(
						IDC_ASSIGN_CONFIG1 + i
					)
				),
				nullptr,
				nullptr
			);

		hConfigAssignmentLabel[i] =
			CreateWindowW(
				L"STATIC",
				L"Not assigned (zeros)",
				WS_CHILD |
				WS_VISIBLE |
				SS_LEFT,
				1325,
				y + 3,
				240,
				35,
				window,
				nullptr,
				nullptr,
				nullptr
			);
	}

	HWND controls[] =
	{
		hStatus,
		hStepStatus,
		hRuntimeStatus,
		hElapsedStatus,
		hRemainingStatus,
		hFinishStatus,

		hTickEdit,
		hSubEdit,

		hList,
		hPreview,

		hSelectedStep,

		hXSlider,
		hYSlider,
		hSleepSlider,

		hXEdit,
		hYEdit,
		hSleepEdit,

		hAddButton,
		hDeleteButton,
		hUpButton,
		hDownButton,

		hSaveButton,
		hLoadButton,
		hResetButton,

		hAssignConfigButton[0],
		hAssignConfigButton[1],
		hAssignConfigButton[2],
		hAssignConfigButton[3],
		hAssignConfigButton[4],
		hAssignConfigButton[5],
		hAssignConfigButton[6],
		hAssignConfigButton[7],
		hAssignConfigButton[8],
		hAssignConfigButton[9],

		hConfigAssignmentLabel[0],
		hConfigAssignmentLabel[1],
		hConfigAssignmentLabel[2],
		hConfigAssignmentLabel[3],
		hConfigAssignmentLabel[4],
		hConfigAssignmentLabel[5],
		hConfigAssignmentLabel[6],
		hConfigAssignmentLabel[7],
		hConfigAssignmentLabel[8],
		hConfigAssignmentLabel[9]
	};

	for (HWND control : controls)
	{
		if (control)
		{
			SendMessageW(
				control,
				WM_SETFONT,
				reinterpret_cast<WPARAM>(
					font
				),
				TRUE
			);
		}
	}

	RefreshList();
}
