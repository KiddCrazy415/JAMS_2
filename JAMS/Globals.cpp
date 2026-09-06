#include "Globals.h"

#include <vector>

std::vector<Movement> pattern(
	40,
	Movement{ 0, 0, 74 }
);

int tickDelay = 15;
int subSteps = 5;

bool leftDown = false;
bool rightDown = false;

bool patternActive = false;
bool patternFinished = false;

size_t patternIndex = 0;
int subIndex = 0;

double dxAcc = 0.0;
double dyAcc = 0.0;

SteadyClock::time_point patternStartTime;
SteadyClock::time_point patternFinishTime;

bool haveStartTime = false;
bool haveFinishTime = false;

HWND hwnd = nullptr;

// GUI controls
HWND hList = nullptr;

HWND hTickEdit = nullptr;
HWND hSubEdit = nullptr;

HWND hStatus = nullptr;
HWND hStepStatus = nullptr;

HWND hRuntimeStatus = nullptr;
HWND hElapsedStatus = nullptr;
HWND hRemainingStatus = nullptr;
HWND hFinishStatus = nullptr;

HWND hAddButton = nullptr;
HWND hDeleteButton = nullptr;
HWND hUpButton = nullptr;
HWND hDownButton = nullptr;

HWND hSaveButton = nullptr;
HWND hLoadButton = nullptr;
HWND hResetButton = nullptr;

HWND hAssignConfigButton[CONFIG_SLOT_COUNT] = {};
HWND hConfigAssignmentLabel[CONFIG_SLOT_COUNT] = {};

std::wstring configAssignments[CONFIG_SLOT_COUNT];

int selectedConfigSlot = -1;

HWND hPreview = nullptr;

HWND hSelectedStep = nullptr;

HWND hXSlider = nullptr;
HWND hYSlider = nullptr;
HWND hSleepSlider = nullptr;

HWND hXEdit = nullptr;
HWND hYEdit = nullptr;
HWND hSleepEdit = nullptr;

HWND hCellEdit = nullptr;

int editingRow = -1;
int editingColumn = -1;

bool cancelCellEdit = false;

WNDPROC originalCellEditProc = nullptr;

HFONT gThemeFont = nullptr;
HFONT gLogoFont = nullptr;

HBRUSH gBackgroundBrush = nullptr;
HBRUSH gPanelBrush = nullptr;
HBRUSH gEditBrush = nullptr;
