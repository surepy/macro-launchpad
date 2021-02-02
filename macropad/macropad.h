#pragma once
#include "resource.h"

#define MAX_LOADSTRING 200

namespace macropad {

    // pasted from tempate stuff

    extern HINSTANCE hInst;                                // current instance
    extern WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    extern WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

    ATOM                MyRegisterClass(HINSTANCE hInstance);
    BOOL                InitInstance(HINSTANCE, int);
    LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
    INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
    INT_PTR CALLBACK    FormDlgproc(HWND Arg1, UINT Arg2, WPARAM Arg3, LPARAM Arg4);

    extern HWND hMainWindow;
    extern HWND hMainForm;

    extern HWND hButton_whatever_this_is;
    extern HWND hList_debug_help;
}

void _DebugString(std::wstring s);
void _DebugString(std::string s);