#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <chrono>

struct Movement
{
	int dx;
	int dy;
	int sleepMs;
};

extern std::vector<Movement> pattern;

extern int tickDelay;
extern int subSteps;

extern bool leftDown;
extern bool rightDown;

extern bool patternActive;
extern bool patternFinished;

extern size_t patternIndex;
extern int subIndex;

extern double dxAcc;
extern double dyAcc;

using SteadyClock = std::chrono::steady_clock;

extern SteadyClock::time_point patternStartTime;
extern SteadyClock::time_point patternFinishTime;

extern bool haveStartTime;
extern bool haveFinishTime;

extern HWND hwnd;

// GUI controls
extern HWND hList;

extern HWND hTickEdit;
extern HWND hSubEdit;

extern HWND hStatus;
extern HWND hStepStatus;

extern HWND hRuntimeStatus;
extern HWND hElapsedStatus;
extern HWND hRemainingStatus;
extern HWND hFinishStatus;

extern HWND hAddButton;
extern HWND hDeleteButton;
extern HWND hUpButton;
extern HWND hDownButton;

extern HWND hSaveButton;
extern HWND hLoadButton;
extern HWND hResetButton;

// Config hotkey assignments
constexpr int CONFIG_SLOT_COUNT = 10;

extern HWND hAssignConfigButton[CONFIG_SLOT_COUNT];
extern HWND hConfigAssignmentLabel[CONFIG_SLOT_COUNT];

extern std::wstring configAssignments[CONFIG_SLOT_COUNT];

extern int selectedConfigSlot;

// Preview area
extern HWND hPreview;

extern HWND hSelectedStep;

extern HWND hXSlider;
extern HWND hYSlider;
extern HWND hSleepSlider;

extern HWND hXEdit;
extern HWND hYEdit;
extern HWND hSleepEdit;

// Inline cell editor
extern HWND hCellEdit;

extern int editingRow;
extern int editingColumn;

extern bool cancelCellEdit;

extern WNDPROC originalCellEditProc;

// Control IDs
constexpr int IDC_LIST = 1001;
constexpr int IDC_TICK = 1002;
constexpr int IDC_SUBSTEPS = 1003;

constexpr int IDC_ADD = 1006;
constexpr int IDC_DELETE = 1007;
constexpr int IDC_UP = 1008;
constexpr int IDC_DOWN = 1009;

constexpr int IDC_SAVE = 1011;
constexpr int IDC_LOAD = 1012;
constexpr int IDC_RESET = 1013;

constexpr int IDC_ASSIGN_CONFIG1 = 1030;

constexpr int IDC_PREVIEW = 1014;

constexpr int IDC_CELL_EDIT = 2000;

constexpr int IDC_XSLIDER = 1020;
constexpr int IDC_YSLIDER = 1021;
constexpr int IDC_SLEEPSLIDER = 1022;

constexpr int IDC_XEDIT = 1023;
constexpr int IDC_YEDIT = 1024;
constexpr int IDC_SLEEPEDIT = 1025;

constexpr UINT TIMER_DISPLAY = 2;

// Theme resources declared here
extern HFONT gThemeFont;
extern HFONT gLogoFont;

extern HBRUSH gBackgroundBrush;
extern HBRUSH gPanelBrush;
extern HBRUSH gEditBrush;

// Preview class name (inline to allow header inclusion in multiple TUs)
inline const wchar_t PREVIEW_CLASS[] = L"CatchAllPreview";

// Function declarations used across modules
void ResetPattern();
void ResetToDefaultPattern();
void StartPattern();

void RefreshList();
void UpdateStatus();
void UpdateRuntimeDisplay();

void UpdatePreview();
