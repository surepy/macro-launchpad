#pragma once
#include "resource.h"

#define MAX_LOADSTRING 200
#define CALLBACK_PROC(name) \
    name##Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

namespace macropad {

    // pasted from tempate stuff

    extern HINSTANCE hInst;                                // current instance
    extern WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    extern WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

    ATOM                MyRegisterClass(HINSTANCE hInstance);
    BOOL                InitInstance(HINSTANCE, int);
    LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
    LRESULT CALLBACK    FormDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    extern HWND hMainWindow;
    extern HWND hMainForm;

    extern HWND hButton_whatever_this_is;
    extern HWND hList_debug_help;

    extern HWND hCombo_Midi_Ins;
    extern HWND hCombo_Midi_Outs;

    extern RtMidiIn* midi_in;
    extern RtMidiOut* midi_out;

    void RefreshButtonList();
    void ClearButtonList();

    void RefreshDevicesList();
}

void _DebugString(std::wstring s);
void _DebugString(std::string s);