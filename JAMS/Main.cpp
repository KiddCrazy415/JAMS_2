#define NOMINMAX

#include <windows.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <commdlg.h>

#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <utility>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <filesystem>

#include "Globals.h"
#include "Theme.h"
#include "ListEdit.h"
#include "Controls.h"

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
// Globals and theme now live in Globals.h/Globals.cpp and Theme.h/Theme.cpp
// This file (Main.cpp) contains application logic and function implementations only.

// ============================================================
// Forward declarations
// ============================================================

void ResetPattern();
void ResetToDefaultPattern();
void StartPattern();

void MoveMouse(
    int dx,
    int dy
);

void UpdatePattern();

void RefreshList();
void UpdateStatus();
void UpdateRuntimeDisplay();

bool SaveConfig(
    const std::wstring& filename
);

bool LoadConfig(
    const std::wstring& filename
);

void SaveConfigDialog();
void LoadConfigDialog();

std::wstring GetApplicationDirectory();
std::wstring GetConfigDirectory();
std::wstring GetConfigSlotPath(int slot);

void LoadConfigSlot(int slot);
void AssignConfigToSlot(int slot);
void UpdateConfigAssignmentLabels();

void AddMovement();
void DeleteMovement();

void MoveSelectedUp();
void MoveSelectedDown();

void UpdateTimer();

void CreateControls(
    HWND window
);

void BeginCellEdit(
    int row,
    int column
);

void CommitCellEdit();
void CancelCellEdit();

void UpdatePreview();

LRESULT CALLBACK CellEditProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK WindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK PreviewProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);


// ============================================================
// Get integer from edit
// ============================================================

int GetEditInteger(
    HWND edit,
    int defaultValue
)
{
    if (!edit)
        return defaultValue;

    wchar_t buffer[64]{};

    GetWindowTextW(
        edit,
        buffer,
        64
    );

    if (buffer[0] == L'\0')
        return defaultValue;

    wchar_t* end = nullptr;

    long value =
        wcstol(
            buffer,
            &end,
            10
        );

    if (end == buffer)
        return defaultValue;

    if (value > 1000000)
        value = 1000000;

    if (value < -1000000)
        value = -1000000;

    return static_cast<int>(value);
}

// ============================================================
// Set integer
// ============================================================

void SetEditInteger(
    HWND edit,
    int value
)
{
    if (!edit)
        return;

    wchar_t buffer[64]{};

    swprintf_s(
        buffer,
        L"%d",
        value
    );

    SetWindowTextW(
        edit,
        buffer
    );

    // Force the standard EDIT control to repaint its current
    // contents immediately. This prevents stale text from
    // remaining visually underneath newly entered text.
    InvalidateRect(
        edit,
        nullptr,
        TRUE
    );

    UpdateWindow(
        edit
    );
}


// ============================================================
// Format milliseconds
// ============================================================

std::wstring FormatMilliseconds(
    long long milliseconds
)
{
    if (milliseconds < 0)
        milliseconds = 0;

    double seconds =
        static_cast<double>(
            milliseconds
            ) / 1000.0;

    std::wstringstream stream;

    stream
        << std::fixed
        << std::setprecision(3)
        << seconds
        << L" seconds";

    return stream.str();
}


// ============================================================
// Format clock time
// ============================================================

std::wstring FormatClockTime(
    SteadyClock::time_point
)
{
    auto now =
        std::chrono::system_clock::now();

    std::time_t currentTime =
        std::chrono::system_clock::to_time_t(
            now
        );

    tm localTime{};

    localtime_s(
        &localTime,
        &currentTime
    );

    wchar_t buffer[64]{};

    wcsftime(
        buffer,
        64,
        L"%I:%M:%S %p",
        &localTime
    );

    return std::wstring(
        buffer
    );
}


// ============================================================
// Estimated runtime
// ============================================================

long long GetEstimatedRuntimeMs()
{
    if (
        pattern.empty() ||
        tickDelay < 1 ||
        subSteps < 1
        )
    {
        return 0;
    }

    long long total =
        static_cast<long long>(
            pattern.size()
            ) *
        static_cast<long long>(
            subSteps
            ) *
        static_cast<long long>(
            tickDelay
            );

    for (
        size_t i = 0;
        i + 1 < pattern.size();
        ++i
        )
    {
        total +=
            std::max(
                0,
                pattern[i].sleepMs
            );
    }

    return total;
}


// ============================================================
// Default pattern
// ============================================================

void ResetToDefaultPattern()
{
    pattern.assign(
        40,
        Movement{ 0, 0, 74 }
    );

    tickDelay = 15;
    subSteps = 5;

    if (hTickEdit)
    {
        SetWindowTextW(
            hTickEdit,
            L"15"
        );
    }

    if (hSubEdit)
    {
        SetWindowTextW(
            hSubEdit,
            L"5"
        );
    }

    ResetPattern();
    RefreshList();
}


// ============================================================
// Reset runtime
// ============================================================

void ResetPattern()
{
    if (
        patternActive &&
        haveStartTime
        )
    {
        patternFinishTime =
            SteadyClock::now();

        haveFinishTime = true;
    }

    patternActive = false;
    patternFinished = false;

    patternIndex = 0;
    subIndex = 0;

    dxAcc = 0.0;
    dyAcc = 0.0;

    if (!haveStartTime)
        haveFinishTime = false;

    UpdateStatus();
    UpdateRuntimeDisplay();
}


// ============================================================
// Start pattern
// ============================================================

void StartPattern()
{
    patternActive = true;
    patternFinished = false;

    patternIndex = 0;
    subIndex = 0;

    dxAcc = 0.0;
    dyAcc = 0.0;

    patternStartTime =
        SteadyClock::now();

    haveStartTime = true;

    long long runtimeMs =
        GetEstimatedRuntimeMs();

    patternFinishTime =
        patternStartTime +
        std::chrono::milliseconds(
            runtimeMs
        );

    haveFinishTime = true;

    UpdateStatus();
    UpdateRuntimeDisplay();
}


// ============================================================
// Send mouse movement
// ============================================================

void MoveMouse(
    int dx,
    int dy
)
{
    if (
        dx == 0 &&
        dy == 0
        )
    {
        return;
    }

    INPUT input{};

    input.type =
        INPUT_MOUSE;

    input.mi.dx =
        dx;

    input.mi.dy =
        dy;

    input.mi.dwFlags =
        MOUSEEVENTF_MOVE;

    SendInput(
        1,
        &input,
        sizeof(INPUT)
    );
}


// ============================================================
// Update pattern
// ============================================================

void UpdatePattern()
{
    if (
        !leftDown ||
        !rightDown
        )
    {
        if (
            patternActive ||
            patternFinished
            )
        {
            ResetPattern();
        }

        return;
    }

    if (pattern.empty())
        return;

    if (subSteps < 1)
        subSteps = 1;

    if (
        !patternActive &&
        !patternFinished
        )
    {
        StartPattern();
    }

    if (patternFinished)
    {
        UpdateRuntimeDisplay();
        return;
    }

    if (
        patternIndex >=
        pattern.size()
        )
    {
        patternActive = false;
        patternFinished = true;

        patternFinishTime =
            SteadyClock::now();

        haveFinishTime = true;

        UpdateStatus();
        UpdateRuntimeDisplay();

        return;
    }

    const Movement& current =
        pattern[patternIndex];

    double dxStep =
        static_cast<double>(
            current.dx
            ) /
        static_cast<double>(
            subSteps
            );

    double dyStep =
        static_cast<double>(
            current.dy
            ) /
        static_cast<double>(
            subSteps
            );

    dxAcc += dxStep;
    dyAcc += dyStep;

    int moveX =
        static_cast<int>(
            std::round(dxAcc)
            );

    int moveY =
        static_cast<int>(
            std::round(dyAcc)
            );

    dxAcc -= moveX;
    dyAcc -= moveY;

    MoveMouse(
        moveX,
        moveY
    );

    subIndex++;

    if (
        subIndex >=
        subSteps
        )
    {
        subIndex = 0;

        patternIndex++;

        dxAcc = 0.0;
        dyAcc = 0.0;
    }

    if (
        patternIndex >=
        pattern.size()
        )
    {
        patternActive = false;
        patternFinished = true;

        patternFinishTime =
            SteadyClock::now();

        haveFinishTime = true;
    }

    UpdateStatus();
    UpdateRuntimeDisplay();
}


// ============================================================
// Update timer
// ============================================================

void UpdateTimer()
{
    int newDelay =
        GetEditInteger(
            hTickEdit,
            tickDelay
        );

    if (newDelay < 1)
        newDelay = 1;

    tickDelay =
        newDelay;

    KillTimer(
        hwnd,
        1
    );

    SetTimer(
        hwnd,
        1,
        static_cast<UINT>(
            tickDelay
            ),
        nullptr
    );

    SetTimer(
        hwnd,
        TIMER_DISPLAY,
        50,
        nullptr
    );

    UpdateRuntimeDisplay();
}


// ============================================================
// Status
// ============================================================

void UpdateStatus()
{
    if (hStatus)
    {
        if (patternActive)
        {
            SetWindowTextW(
                hStatus,
                L"Status: RUNNING"
            );
        }
        else if (patternFinished)
        {
            SetWindowTextW(
                hStatus,
                L"Status: FINISHED"
            );
        }
        else
        {
            SetWindowTextW(
                hStatus,
                L"Status: WAITING"
            );
        }
    }

    if (hStepStatus)
    {
        wchar_t buffer[128]{};

        size_t currentStep = 0;

        if (!pattern.empty())
        {
            if (
                patternIndex <
                pattern.size()
                )
            {
                currentStep =
                    patternIndex + 1;
            }
            else
            {
                currentStep =
                    pattern.size();
            }
        }

        swprintf_s(
            buffer,
            L"Step: %zu / %zu",
            currentStep,
            pattern.size()
        );

        SetWindowTextW(
            hStepStatus,
            buffer
        );
    }
}


// ============================================================
// Runtime display
// ============================================================

void UpdateRuntimeDisplay()
{
    long long runtimeMs =
        GetEstimatedRuntimeMs();

    if (hRuntimeStatus)
    {
        std::wstring text =
            L"Estimated Runtime: " +
            FormatMilliseconds(
                runtimeMs
            );

        SetWindowTextW(
            hRuntimeStatus,
            text.c_str()
        );
    }

    if (hElapsedStatus)
    {
        std::wstring text;

        if (haveStartTime)
        {
            auto endPoint =
                (
                    patternFinished &&
                    haveFinishTime
                    )
                ? patternFinishTime
                : SteadyClock::now();

            auto elapsed =
                std::chrono::duration_cast<
                std::chrono::milliseconds
                >(
                    endPoint -
                    patternStartTime
                ).count();

            if (elapsed < 0)
                elapsed = 0;

            std::wstringstream stream;

            stream
                << L"Elapsed: "
                << std::fixed
                << std::setprecision(3)
                << (
                    static_cast<double>(
                        elapsed
                        ) / 1000.0
                    )
                << L" seconds";

            text =
                stream.str();
        }
        else
        {
            text =
                L"Elapsed: 0.000 seconds";
        }

        SetWindowTextW(
            hElapsedStatus,
            text.c_str()
        );
    }

    if (hRemainingStatus)
    {
        std::wstring text;

        if (
            patternActive &&
            haveStartTime
            )
        {
            auto now =
                SteadyClock::now();

            auto elapsed =
                std::chrono::duration_cast<
                std::chrono::milliseconds
                >(
                    now -
                    patternStartTime
                ).count();

            long long remaining =
                runtimeMs -
                elapsed;

            if (remaining < 0)
                remaining = 0;

            text =
                L"Remaining: " +
                FormatMilliseconds(
                    remaining
                );
        }
        else if (
            patternFinished ||
            (
                !patternActive &&
                haveFinishTime &&
                haveStartTime
                )
            )
        {
            text =
                L"Remaining: 0.000 seconds";
        }
        else
        {
            text =
                L"Remaining: " +
                FormatMilliseconds(
                    runtimeMs
                );
        }

        SetWindowTextW(
            hRemainingStatus,
            text.c_str()
        );
    }

    if (hFinishStatus)
    {
        std::wstring text;

        if (haveFinishTime)
        {
            if (patternFinished)
            {
                text =
                    L"Finished: " +
                    FormatClockTime(
                        patternFinishTime
                    );
            }
            else
            {
                text =
                    L"Estimated Finish: " +
                    FormatClockTime(
                        patternFinishTime
                    );
            }
        }
        else
        {
            text =
                L"Estimated Finish: --:--:--";
        }

        SetWindowTextW(
            hFinishStatus,
            text.c_str()
        );
    }
}


// ============================================================
// Refresh movement list
// ============================================================

void RefreshList()
{
    if (!hList)
        return;

    int selected =
        ListView_GetNextItem(
            hList,
            -1,
            LVNI_SELECTED
        );

    ListView_DeleteAllItems(
        hList
    );

    for (
        size_t i = 0;
        i < pattern.size();
        ++i
        )
    {
        wchar_t stepText[32]{};
        wchar_t xText[32]{};
        wchar_t yText[32]{};
        wchar_t sleepText[32]{};

        swprintf_s(
            stepText,
            L"%zu",
            i + 1
        );

        swprintf_s(
            xText,
            L"%d",
            pattern[i].dx
        );

        swprintf_s(
            yText,
            L"%d",
            pattern[i].dy
        );

        swprintf_s(
            sleepText,
            L"%d",
            pattern[i].sleepMs
        );

        LVITEMW item{};

        item.mask =
            LVIF_TEXT;

        item.iItem =
            static_cast<int>(i);

        item.pszText =
            stepText;

        ListView_InsertItem(
            hList,
            &item
        );

        ListView_SetItemText(
            hList,
            static_cast<int>(i),
            1,
            xText
        );

        ListView_SetItemText(
            hList,
            static_cast<int>(i),
            2,
            yText
        );

        ListView_SetItemText(
            hList,
            static_cast<int>(i),
            3,
            sleepText
        );
    }

    if (!pattern.empty())
    {
        if (selected < 0)
            selected = 0;

        if (
            selected >=
            static_cast<int>(
                pattern.size()
                )
            )
        {
            selected =
                static_cast<int>(
                    pattern.size()
                    ) - 1;
        }

        ListView_SetItemState(
            hList,
            selected,
            LVIS_SELECTED |
            LVIS_FOCUSED,
            LVIS_SELECTED |
            LVIS_FOCUSED
        );

        ListView_EnsureVisible(
            hList,
            selected,
            FALSE
        );
    }

    UpdateStatus();
    UpdateRuntimeDisplay();
    UpdatePreview();
}


// BeginCellEdit moved to ListEdit.cpp


// ============================================================
// Commit cell edit
// ============================================================

// CommitCellEdit moved to ListEdit.cpp


// ============================================================
// Cancel cell edit
// ============================================================

// CancelCellEdit moved to ListEdit.cpp


// ============================================================
// Cell edit procedure
// ============================================================

// CellEditProc moved to ListEdit.cpp


// ============================================================
// Add movement
// ============================================================

void AddMovement()
{
    if (hCellEdit)
        CommitCellEdit();

    int selected =
        ListView_GetNextItem(
            hList,
            -1,
            LVNI_SELECTED
        );

    size_t insertPosition;

    if (selected < 0)
    {
        insertPosition =
            pattern.size();
    }
    else
    {
        insertPosition =
            static_cast<size_t>(
                selected + 1
                );
    }

    pattern.insert(
        pattern.begin() +
        static_cast<
        std::vector<Movement>::difference_type
        >(insertPosition),
        Movement{ 0, 0, 74 }
    );

    ResetPattern();

    RefreshList();

    int newIndex =
        static_cast<int>(
            insertPosition
            );

    ListView_SetItemState(
        hList,
        newIndex,
        LVIS_SELECTED |
        LVIS_FOCUSED,
        LVIS_SELECTED |
        LVIS_FOCUSED
    );

    ListView_EnsureVisible(
        hList,
        newIndex,
        FALSE
    );
}


// ============================================================
// Delete movement
// ============================================================

void DeleteMovement()
{
    if (hCellEdit)
        CommitCellEdit();

    if (pattern.empty())
        return;

    int selected =
        ListView_GetNextItem(
            hList,
            -1,
            LVNI_SELECTED
        );

    if (
        selected < 0 ||
        selected >=
        static_cast<int>(
            pattern.size()
            )
        )
    {
        return;
    }

    pattern.erase(
        pattern.begin() +
        selected
    );

    ResetPattern();

    RefreshList();

    if (!pattern.empty())
    {
        int newSelection =
            selected;

        if (
            newSelection >=
            static_cast<int>(
                pattern.size()
                )
            )
        {
            newSelection =
                static_cast<int>(
                    pattern.size()
                    ) - 1;
        }

        ListView_SetItemState(
            hList,
            newSelection,
            LVIS_SELECTED |
            LVIS_FOCUSED,
            LVIS_SELECTED |
            LVIS_FOCUSED
        );

        ListView_EnsureVisible(
            hList,
            newSelection,
            FALSE
        );
    }
}


// ============================================================
// Move selected step up
// ============================================================

void MoveSelectedUp()
{
    if (hCellEdit)
        CommitCellEdit();

    int selected =
        ListView_GetNextItem(
            hList,
            -1,
            LVNI_SELECTED
        );

    if (
        selected <= 0 ||
        selected >=
        static_cast<int>(
            pattern.size()
            )
        )
    {
        return;
    }

    std::swap(
        pattern[selected],
        pattern[selected - 1]
    );

    ResetPattern();

    RefreshList();

    ListView_SetItemState(
        hList,
        selected - 1,
        LVIS_SELECTED |
        LVIS_FOCUSED,
        LVIS_SELECTED |
        LVIS_FOCUSED
    );
}


// ============================================================
// Move selected step down
// ============================================================

void MoveSelectedDown()
{
    if (hCellEdit)
        CommitCellEdit();

    int selected =
        ListView_GetNextItem(
            hList,
            -1,
            LVNI_SELECTED
        );

    if (
        selected < 0 ||
        selected >=
        static_cast<int>(
            pattern.size()
            ) - 1
        )
    {
        return;
    }

    std::swap(
        pattern[selected],
        pattern[selected + 1]
    );

    ResetPattern();

    RefreshList();

    ListView_SetItemState(
        hList,
        selected + 1,
        LVIS_SELECTED |
        LVIS_FOCUSED,
        LVIS_SELECTED |
        LVIS_FOCUSED
    );
}


// ============================================================
// Application/config paths
// ============================================================

std::wstring GetApplicationDirectory()
{
    wchar_t path[MAX_PATH]{};

    DWORD length =
        GetModuleFileNameW(
            nullptr,
            path,
            MAX_PATH
        );

    if (
        length == 0 ||
        length >= MAX_PATH
        )
    {
        return L".";
    }

    std::wstring fullPath(
        path,
        length
    );

    size_t slash =
        fullPath.find_last_of(
            L"\\/"
        );

    if (
        slash ==
        std::wstring::npos
        )
    {
        return L".";
    }

    return fullPath.substr(
        0,
        slash
    );
}


std::wstring GetConfigDirectory()
{
    std::wstring directory =
        GetApplicationDirectory() +
        L"\\configs";

    CreateDirectoryW(
        directory.c_str(),
        nullptr
    );

    return directory;
}


std::wstring GetConfigSlotPath(
    int slot
)
{
    if (
        slot < 1 ||
        slot > CONFIG_SLOT_COUNT
        )
    {
        return L"";
    }

    return GetConfigDirectory() +
        L"\\" +
        std::to_wstring(slot) +
        L".cfg";
}


// ============================================================
// Config assignment labels
// ============================================================

void UpdateConfigAssignmentLabels()
{
    for (
        int i = 0;
        i < CONFIG_SLOT_COUNT;
        ++i
        )
    {
        if (!hConfigAssignmentLabel[i])
            continue;

        std::wstring text =
            configAssignments[i].empty()
            ? L"Not assigned (zeros)"
            : configAssignments[i];

        size_t slash =
            text.find_last_of(
                L"\\/"
            );

        if (
            slash !=
            std::wstring::npos
            )
        {
            text =
                text.substr(
                    slash + 1
                );
        }

        SetWindowTextW(
            hConfigAssignmentLabel[i],
            text.c_str()
        );
    }
}


// ============================================================
// Assign config to slot
// ============================================================

void AssignConfigToSlot(
    int slot
)
{
    if (
        slot < 1 ||
        slot > CONFIG_SLOT_COUNT
        )
    {
        return;
    }

    wchar_t filename[MAX_PATH]{};

    OPENFILENAMEW ofn{};

    ofn.lStructSize =
        sizeof(ofn);

    ofn.hwndOwner =
        hwnd;

    ofn.lpstrFile =
        filename;

    ofn.nMaxFile =
        MAX_PATH;

    ofn.lpstrFilter =
        L"Config Files (*.cfg)\0*.cfg\0"
        L"All Files (*.*)\0*.*\0";

    ofn.nFilterIndex =
        1;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST;

    ofn.lpstrDefExt =
        L"cfg";

    if (GetOpenFileNameW(&ofn))
    {
        configAssignments[slot - 1] =
            filename;

        UpdateConfigAssignmentLabels();

        wchar_t message[512]{};

        swprintf_s(
            message,
            L"Config assigned to hotkey %d.\n\n%s",
            slot == 10 ? 0 : slot,
            filename
        );

        SetWindowTextW(
            hStatus,
            message
        );
    }
}


// ============================================================
// Load config assigned to slot
// ============================================================

void LoadConfigSlot(
    int slot
)
{
    if (
        slot < 1 ||
        slot > CONFIG_SLOT_COUNT
        )
    {
        return;
    }

    const std::wstring& filename =
        configAssignments[slot - 1];

    if (filename.empty())
    {
        ResetToDefaultPattern();

        wchar_t message[128]{};

        swprintf_s(
            message,
            L"Hotkey %d: default zero pattern.",
            slot == 10 ? 0 : slot
        );

        if (hStatus)
        {
            SetWindowTextW(
                hStatus,
                message
            );
        }

        return;
    }

    DWORD attributes =
        GetFileAttributesW(
            filename.c_str()
        );

    if (
        attributes ==
        INVALID_FILE_ATTRIBUTES ||
        (
            attributes &
            FILE_ATTRIBUTE_DIRECTORY
            )
        )
    {
        ResetToDefaultPattern();

        wchar_t message[512]{};

        swprintf_s(
            message,
            L"Hotkey %d config is missing.\n"
            L"Loaded default zero pattern instead.\n\n"
            L"%s",
            slot == 10 ? 0 : slot,
            filename.c_str()
        );

        MessageBoxW(
            hwnd,
            message,
            L"Config Missing",
            MB_OK |
            MB_ICONINFORMATION
        );

        return;
    }

    if (!LoadConfig(filename))
    {
        ResetToDefaultPattern();

        wchar_t message[256]{};

        swprintf_s(
            message,
            L"Could not load the config assigned to hotkey %d.\n"
            L"Loaded default zero pattern instead.",
            slot == 10 ? 0 : slot
        );

        MessageBoxW(
            hwnd,
            message,
            L"Load Error",
            MB_OK |
            MB_ICONERROR
        );

        return;
    }

    wchar_t message[512]{};

    swprintf_s(
        message,
        L"Hotkey %d: %s",
        slot == 10 ? 0 : slot,
        filename.c_str()
    );

    if (hStatus)
    {
        SetWindowTextW(
            hStatus,
            message
        );
    }
}


// ============================================================
// Save configuration
// ============================================================

bool SaveConfig(
    const std::wstring& filename
)
{
    if (hCellEdit)
        CommitCellEdit();

    tickDelay =
        GetEditInteger(
            hTickEdit,
            tickDelay
        );

    subSteps =
        GetEditInteger(
            hSubEdit,
            subSteps
        );

    if (tickDelay < 1)
        tickDelay = 1;

    if (subSteps < 1)
        subSteps = 1;

    std::wofstream file(
        filename
    );

    if (!file.is_open())
        return false;

    file
        << L"TickDelay="
        << tickDelay
        << L"\n";

    file
        << L"SubSteps="
        << subSteps
        << L"\n";

    file
        << L"MovementCount="
        << pattern.size()
        << L"\n";

    for (
        const Movement& movement :
        pattern
        )
    {
        file
            << movement.dx
            << L","
            << movement.dy
            << L","
            << movement.sleepMs
            << L"\n";
    }

    return true;
}


// ============================================================
// Load configuration
// ============================================================

bool LoadConfig(
    const std::wstring& filename
)
{
    if (hCellEdit)
        CommitCellEdit();

    std::wifstream file(
        filename
    );

    if (!file.is_open())
        return false;

    std::wstring line;

    int newTickDelay = 15;
    int newSubSteps = 5;

    std::vector<Movement> newPattern;

    while (
        std::getline(
            file,
            line
        )
        )
    {
        if (line.empty())
            continue;

        if (
            line.rfind(
                L"TickDelay=",
                0
            ) == 0
            )
        {
            newTickDelay =
                _wtoi(
                    line.substr(
                        10
                    ).c_str()
                );

            continue;
        }

        if (
            line.rfind(
                L"SubSteps=",
                0
            ) == 0
            )
        {
            newSubSteps =
                _wtoi(
                    line.substr(
                        9
                    ).c_str()
                );

            continue;
        }

        if (
            line.rfind(
                L"MovementCount=",
                0
            ) == 0
            )
        {
            continue;
        }

        size_t comma1 =
            line.find(
                L','
            );

        if (
            comma1 ==
            std::wstring::npos
            )
        {
            continue;
        }

        size_t comma2 =
            line.find(
                L',',
                comma1 + 1
            );

        try
        {
            int x =
                std::stoi(
                    line.substr(
                        0,
                        comma1
                    )
                );

            int y = 0;
            int sleepMs = 74;

            if (
                comma2 ==
                std::wstring::npos
                )
            {
                y =
                    std::stoi(
                        line.substr(
                            comma1 + 1
                        )
                    );
            }
            else
            {
                y =
                    std::stoi(
                        line.substr(
                            comma1 + 1,
                            comma2 -
                            comma1 -
                            1
                        )
                    );

                sleepMs =
                    std::stoi(
                        line.substr(
                            comma2 + 1
                        )
                    );
            }

            x =
                std::max(
                    -100,
                    std::min(
                        100,
                        x
                    )
                );

            y =
                std::max(
                    -100,
                    std::min(
                        100,
                        y
                    )
                );

            sleepMs =
                std::max(
                    0,
                    std::min(
                        200,
                        sleepMs
                    )
                );

            newPattern.push_back(
                Movement{
                    x,
                    y,
                    sleepMs
                }
            );
        }
        catch (...)
        {
            return false;
        }
    }

    file.close();

    if (newPattern.empty())
        return false;

    if (newTickDelay < 1)
        newTickDelay = 1;

    if (newSubSteps < 1)
        newSubSteps = 1;

    tickDelay =
        newTickDelay;

    subSteps =
        newSubSteps;

    pattern =
        std::move(
            newPattern
        );

    ResetPattern();

    SetEditInteger(
        hTickEdit,
        tickDelay
    );

    SetEditInteger(
        hSubEdit,
        subSteps
    );

    UpdateTimer();

    RefreshList();

    // Provide brief status so user can confirm the file parsed correctly
    if (hStatus)
    {
        wchar_t msg[512]{};
        swprintf_s(msg, L"Loaded %zu movements.", pattern.size());
        if (!pattern.empty())
        {
            swprintf_s(msg + wcslen(msg), 512 - wcslen(msg), L" First: %d,%d,%d",
                pattern[0].dx, pattern[0].dy, pattern[0].sleepMs);
        }
        SetWindowTextW(hStatus, msg);
    }

    return true;
}


void SaveConfigDialog()
{
    std::wstring configDirectory =
        GetConfigDirectory();

    wchar_t filename[MAX_PATH]{};

    std::wstring defaultFilename =
        configDirectory +
        L"\\config.cfg";

    wcsncpy_s(
        filename,
        defaultFilename.c_str(),
        _TRUNCATE
    );

    OPENFILENAMEW ofn{};

    ofn.lStructSize =
        sizeof(ofn);

    ofn.hwndOwner =
        hwnd;

    ofn.lpstrFile =
        filename;

    ofn.nMaxFile =
        MAX_PATH;

    ofn.lpstrFilter =
        L"Config Files (*.cfg)\0*.cfg\0"
        L"All Files (*.*)\0*.*\0";

    ofn.nFilterIndex =
        1;

    ofn.Flags =
        OFN_OVERWRITEPROMPT |
        OFN_PATHMUSTEXIST;

    ofn.lpstrDefExt =
        L"cfg";

    ofn.lpstrInitialDir =
        configDirectory.c_str();

    if (GetSaveFileNameW(&ofn))
    {
        if (!SaveConfig(filename))
        {
            MessageBoxW(
                hwnd,
                L"Could not save the configuration.",
                L"Save Error",
                MB_OK |
                MB_ICONERROR
            );
        }
    }
}


void LoadConfigDialog()
{
    if (hCellEdit)
        CommitCellEdit();

    std::wstring configDirectory =
        GetConfigDirectory();

    wchar_t filename[MAX_PATH]{};

    OPENFILENAMEW ofn{};

    ofn.lStructSize =
        sizeof(ofn);

    ofn.hwndOwner =
        hwnd;

    ofn.lpstrFile =
        filename;

    ofn.nMaxFile =
        MAX_PATH;

    ofn.lpstrFilter =
        L"Config Files (*.cfg)\0*.cfg\0"
        L"All Files (*.*)\0*.*\0";

    ofn.nFilterIndex =
        1;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST;

    ofn.lpstrDefExt =
        L"cfg";

    ofn.lpstrInitialDir =
        configDirectory.c_str();

    if (GetOpenFileNameW(&ofn))
    {
        if (!LoadConfig(filename))
        {
            MessageBoxW(
                hwnd,
                L"Could not load the configuration.\n\n"
                L"Make sure the file is a valid .cfg file.",
                L"Load Error",
                MB_OK |
                MB_ICONERROR
            );
        }
    }
}


static HHOOK g_keyboardHook = nullptr;


bool IsJAMSEditControlFocused()
{
    HWND focus =
        GetFocus();

    if (!focus)
        return false;

    if (
        focus == hXEdit ||
        focus == hYEdit ||
        focus == hSleepEdit ||
        focus == hTickEdit ||
        focus == hSubEdit ||
        focus == hCellEdit
        )
    {
        return true;
    }

    return false;
}


// ============================================================
// Low-level keyboard hook
// ============================================================

LRESULT CALLBACK LowLevelKeyboardProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
)
{
    if (
        nCode == HC_ACTION &&
        (
            wParam == WM_KEYDOWN ||
            wParam == WM_SYSKEYDOWN
            )
        )
    {
        // IMPORTANT:
        // Do not process 1-0 config hotkeys while the user
        // is typing inside any JAMS edit control.

        if (IsJAMSEditControlFocused())
        {
            return CallNextHookEx(
                g_keyboardHook,
                nCode,
                wParam,
                lParam
            );
        }

        const KBDLLHOOKSTRUCT* key =
            reinterpret_cast<
            const KBDLLHOOKSTRUCT*
            >(lParam);

        int slot = 0;

        switch (key->vkCode)
        {
        case '1':
            slot = 1;
            break;

        case '2':
            slot = 2;
            break;

        case '3':
            slot = 3;
            break;

        case '4':
            slot = 4;
            break;

        case '5':
            slot = 5;
            break;

        case '6':
            slot = 6;
            break;

        case '7':
            slot = 7;
            break;

        case '8':
            slot = 8;
            break;

        case '9':
            slot = 9;
            break;

        case '0':
            slot = 10;
            break;

        default:
            break;
        }

        if (slot != 0)
        {
            selectedConfigSlot = slot;

            // Do not call UI functions directly from the hook thread. Post a
            // message to the main window to load the assigned slot if present.
            if (hwnd)
            {
                constexpr UINT WM_APP_LOAD_SLOT = WM_APP + 100;
                PostMessageW(hwnd, WM_APP_LOAD_SLOT, static_cast<WPARAM>(slot), 0);
            }
        }
    }

    return CallNextHookEx(
        g_keyboardHook,
        nCode,
        wParam,
        lParam
    );
}


void InstallKeyboardHook()
{
    g_keyboardHook =
        SetWindowsHookExW(
            WH_KEYBOARD_LL,
            LowLevelKeyboardProc,
            GetModuleHandleW(nullptr),
            0
        );
}


void RemoveKeyboardHook()
{
    if (g_keyboardHook)
    {
        UnhookWindowsHookEx(
            g_keyboardHook
        );

        g_keyboardHook =
            nullptr;
    }
}


void RegisterRawMouse(
    HWND window
)
{
    RAWINPUTDEVICE rid{};

    rid.usUsagePage =
        0x01;

    rid.usUsage =
        0x02;

    rid.dwFlags =
        RIDEV_INPUTSINK;

    rid.hwndTarget =
        window;

    RegisterRawInputDevices(
        &rid,
        1,
        sizeof(
            RAWINPUTDEVICE
            )
    );
}


// DrawPreview implementation moved to Preview.cpp (declaration in Preview.h)


// UpdatePreview moved to Preview.cpp



// PreviewProc moved to Preview.cpp



// end of PreviewProc


// CreateControls implementation moved to Controls.cpp

// Minimal WindowProc implementation to satisfy the linker.
LRESULT CALLBACK WindowProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        switch (id)
        {
        case IDC_ADD:
            AddMovement();
            return 0;
        case IDC_DELETE:
            DeleteMovement();
            return 0;
        case IDC_UP:
            MoveSelectedUp();
            return 0;
        case IDC_DOWN:
            MoveSelectedDown();
            return 0;
        case IDC_SAVE:
            SaveConfigDialog();
            return 0;
        case IDC_ASSIGN_CONFIG1:
        case IDC_ASSIGN_CONFIG1 + 1:
        case IDC_ASSIGN_CONFIG1 + 2:
        case IDC_ASSIGN_CONFIG1 + 3:
        case IDC_ASSIGN_CONFIG1 + 4:
        case IDC_ASSIGN_CONFIG1 + 5:
        case IDC_ASSIGN_CONFIG1 + 6:
        case IDC_ASSIGN_CONFIG1 + 7:
        case IDC_ASSIGN_CONFIG1 + 8:
        case IDC_ASSIGN_CONFIG1 + 9:
        {
            int slot = id - IDC_ASSIGN_CONFIG1 + 1;
            AssignConfigToSlot(slot);
            return 0;
        }
        case IDC_LOAD:
            LoadConfigDialog();
            return 0;
        case IDC_RESET:
            ResetToDefaultPattern();
            return 0;
        case IDC_PREVIEW:
            // ignore
            return 0;
        default:
            break;
        }

        break;
    }

    case WM_HOTKEY:
    {
        // ID 1 is registered for INSERT
        if (wParam == 1)
        {
            // If a config slot is selected, open the Assign dialog so the user
            // can pick an existing config file to assign to that slot. Otherwise
            // open the general Load dialog to load a config into the application.
            if (selectedConfigSlot >= 1 && selectedConfigSlot <= CONFIG_SLOT_COUNT)
            {
                AssignConfigToSlot(selectedConfigSlot);
                return 0;
            }

            LoadConfigDialog();
            return 0;
        }

        break;
    }

    case WM_DRAWITEM:
    {
        const DRAWITEMSTRUCT* dis = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
        DrawThemedButton(dis);
        return TRUE;
    }

    // Posted by keyboard hook when user presses 1-0
    case WM_APP + 100:
    {
        int slot = static_cast<int>(wParam);
        if (slot >= 1 && slot <= CONFIG_SLOT_COUNT)
        {
            const std::wstring& assigned = configAssignments[slot - 1];
            if (!assigned.empty())
            {
                LoadConfigSlot(slot);
            }
            else
            {
                // No assignment: reset to default zero pattern so the slot
                // does not keep the last-loaded configuration.
                ResetToDefaultPattern();

                wchar_t message[128]{};
                swprintf_s(message, L"Hotkey %d: default zero pattern.", slot == 10 ? 0 : slot);
                if (hStatus) SetWindowTextW(hStatus, message);
            }
        }

        break;
    }

    case WM_NOTIFY:
    {
        NMHDR* hdr = reinterpret_cast<NMHDR*>(lParam);

        if (hdr && hdr->hwndFrom == hList)
        {
            if (hdr->code == NM_DBLCLK)
            {
                // Perform subitem hit test and begin cell edit
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hList, &pt);

                LVHITTESTINFO hit{};
                hit.pt = pt;
                int item = ListView_HitTest(hList, &hit);

                int sub = hit.iSubItem;
                if (item >= 0)
                {
                    // If subitem is 0 (step column) do not edit
                    if (sub < 1) sub = 1;
                    BeginCellEdit(item, sub);
                }
            }
        }

        break;
    }

    case WM_CTLCOLOREDIT:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, JAMSTHEME::Text);
        SetBkMode(hdc, TRANSPARENT);
        HBRUSH brush = gEditBrush ? gEditBrush : gBackgroundBrush;
        return reinterpret_cast<LRESULT>(brush);
    }

    case WM_INPUT:
    {
        // Handle raw mouse button events to update left/right button state
        UINT dwSize = 0;
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) != (UINT)-1 && dwSize > 0)
        {
            std::vector<BYTE> buffer(dwSize);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
            {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    USHORT flags = raw->data.mouse.usButtonFlags;

                    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) leftDown = true;
                    if (flags & RI_MOUSE_LEFT_BUTTON_UP) leftDown = false;
                    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) rightDown = true;
                    if (flags & RI_MOUSE_RIGHT_BUTTON_UP) rightDown = false;
                }
            }
        }

        break;
    }

    case WM_TIMER:
    {
        if (wParam == 1)
        {
            UpdatePattern();
        }
        else if (wParam == TIMER_DISPLAY)
        {
            UpdateRuntimeDisplay();
            UpdatePreview();
        }

        break;
    }

    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }

    return 0;
}


int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR,
    int nCmdShow
)
{
    InitializeJAMSTheme();

    INITCOMMONCONTROLSEX icc{};

    icc.dwSize =
        sizeof(
            INITCOMMONCONTROLSEX
            );

    icc.dwICC =
        ICC_LISTVIEW_CLASSES |
        ICC_BAR_CLASSES;

    InitCommonControlsEx(
        &icc
    );


    // ========================================================
    // Preview class
    // ========================================================

    const wchar_t PREVIEW_CLASS[] =
        L"CatchAllPreview";

    WNDCLASSW previewClass{};

    previewClass.lpfnWndProc =
        PreviewProc;

    previewClass.hInstance =
        hInstance;

    previewClass.lpszClassName =
        PREVIEW_CLASS;

    previewClass.hCursor =
        LoadCursorW(
            nullptr,
            IDC_ARROW
        );

    previewClass.hbrBackground =
        gBackgroundBrush;

    RegisterClassW(
        &previewClass
    );


    // ========================================================
    // Main window class
    // ========================================================

    const wchar_t CLASS_NAME[] =
        L"EditableMousePatternWindow";

    WNDCLASSW wc{};

    wc.lpfnWndProc =
        WindowProc;

    wc.hInstance =
        hInstance;

    wc.lpszClassName =
        CLASS_NAME;

    wc.hCursor =
        LoadCursorW(
            nullptr,
            IDC_ARROW
        );

    wc.hbrBackground =
        gBackgroundBrush;

    if (
        !RegisterClassW(
            &wc
        )
        )
    {
        MessageBoxW(
            nullptr,
            L"Could not register window class.",
            L"Error",
            MB_OK |
            MB_ICONERROR
        );

        return 1;
    }


    // ========================================================
    // Create main GUI
    // ========================================================

    hwnd =
        CreateWindowExW(
            0,
            CLASS_NAME,
            L"(JAMS) Just Another Mouse Simulator",

            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU |
            WS_MINIMIZEBOX |
            WS_CLIPCHILDREN,

            CW_USEDEFAULT,
            CW_USEDEFAULT,

            1600,
            750,

            nullptr,
            nullptr,
            hInstance,
            nullptr
        );

    if (!hwnd)
    {
        MessageBoxW(
            nullptr,
            L"Could not create window.",
            L"Error",
            MB_OK |
            MB_ICONERROR
        );

        return 1;
    }


    // ========================================================
    // Dark title bar
    // ========================================================

    BOOL darkMode = TRUE;

    DwmSetWindowAttribute(
        hwnd,
        20,
        &darkMode,
        sizeof(darkMode)
    );

    InvalidateRect(
        hwnd,
        nullptr,
        TRUE
    );


    // ========================================================
    // Clear config assignments on startup
    // ========================================================

    for (
        int i = 0;
        i < CONFIG_SLOT_COUNT;
        ++i
        )
    {
        configAssignments[i].clear();
    }

    selectedConfigSlot = -1;


    // ========================================================
    // Create controls
    // ========================================================

    CreateControls(
        hwnd
    );

    UpdateConfigAssignmentLabels();


    // ========================================================
    // Raw mouse
    // ========================================================

    RegisterRawMouse(
        hwnd
    );


    // ========================================================
    // INSERT hotkey
    // ========================================================

    RegisterHotKey(
        hwnd,
        1,
        MOD_NOREPEAT,
        VK_INSERT
    );


    // ========================================================
    // Global number-key hook
    // ========================================================

    InstallKeyboardHook();


    // ========================================================
    // Timers
    // ========================================================

    SetTimer(
        hwnd,
        1,
        static_cast<UINT>(
            tickDelay
            ),
        nullptr
    );

    SetTimer(
        hwnd,
        TIMER_DISPLAY,
        50,
        nullptr
    );


    // ========================================================
    // Show GUI
    // ========================================================

    ShowWindow(
        hwnd,
        nCmdShow
    );

    UpdateWindow(
        hwnd
    );


    // ========================================================
    // Message loop
    // ========================================================

    MSG msg{};

    while (
        GetMessageW(
            &msg,
            nullptr,
            0,
            0
        )
        )
    {
        TranslateMessage(
            &msg
        );

        DispatchMessageW(
            &msg
        );
    }


    // ========================================================
    // Cleanup
    // ========================================================

    KillTimer(
        hwnd,
        1
    );

    KillTimer(
        hwnd,
        TIMER_DISPLAY
    );

    UnregisterHotKey(
        hwnd,
        1
    );

    ShutdownJAMSTheme();

    return 0;
}