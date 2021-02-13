// macropad.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "macropad.h"
#include "Launchpad.h"
#include "Config.h"
#include <array>
#include <Dbt.h>

namespace macropad {
    HINSTANCE hInst;                                // current instance
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

    HWND hMainWindow;
    HWND hMainForm;
    HWND hList_debug_help; // lol

    HWND hCombo_Midi_Ins;
    HWND hCombo_Midi_Outs;

    // used exclusively for probing
    RtMidiIn* midi_in = new RtMidiIn();
    RtMidiOut* midi_out = new RtMidiOut();

    //
    //  FUNCTION: MyRegisterClass()
    //
    //  PURPOSE: Registers the window class.
    //
    ATOM MyRegisterClass(HINSTANCE hInstance)
    {
        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MACROPAD));
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MACROPAD);
        wcex.lpszClassName = szWindowClass;
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        return RegisterClassExW(&wcex);
    }

    //
    //   FUNCTION: InitInstance(HINSTANCE, int)
    //
    //   PURPOSE: Saves instance handle and creates main window
    //
    //   COMMENTS:
    //
    //        In this function, we save the instance handle in a global variable and
    //        create and display the main program window.
    //
    BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
    {
        hInst = hInstance; // Store instance handle in our global variable

        HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 1010, 650, nullptr, nullptr, hInstance, nullptr);

        HWND hWindForm = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, FormDlgProc);

        if (!(hWnd || hWindForm))
        {
            return FALSE;
        }

        ShowWindow(hWnd, nCmdShow);
        ShowWindow(hWindForm, SW_SHOW);
        UpdateWindow(hWnd);

        macropad::hList_debug_help = GetDlgItem(hWindForm, IDC_LIST_DEBUG_HELP);

        macropad::hCombo_Midi_Ins = GetDlgItem(hWindForm, IDC_MIDI_DEVICE_IN);
        macropad::hCombo_Midi_Outs = GetDlgItem(hWindForm, IDC_MIDI_DEVICE_OUT);

        return TRUE;
    }


    LRESULT CALLBACK FormDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);

        switch (uMsg)
        {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_LAUNCHPAD_TEST_LOW:
                midi_device::launchpad::Launchpad::GetDevice()->low_brightness_test();
                break;
            case IDC_LAUNCHPAD_TEST_MED:
                midi_device::launchpad::Launchpad::GetDevice()->medium_brightness_test();
                break;
            case IDC_LAUNCHPAD_TEST_FULL:
                midi_device::launchpad::Launchpad::GetDevice()->full_brightness_test();
                break;
            case IDC_LAUNCHPAD_REFRESH: {
                midi_device::launchpad::Launchpad::GetDevice()->fullLedUpdate();
                RefreshButtonList();
                break;
            }
            case IDC_LAUNCHPAD_RESET:
                midi_device::launchpad::Launchpad::GetDevice()->reset();
                break;
            case IDC_BUTTON_TEST2:
                midi_device::launchpad::Launchpad::GetDevice()->setup_pages_test();
                break;
            case IDC_CONFIG_RELOAD: {
                if (config::file_handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(config::file_handle);
                }

                config::openFileHandle();
                config::loadFile();
                CloseHandle(config::file_handle);

                break;
            }
            case IDC_CONFIG_RELOAD2: {
                midi_device::launchpad::Launchpad::GetDevice()->load_config_buttons_test();
                midi_device::launchpad::Launchpad::GetDevice()->fullLedUpdate();
                break;
            }
            case IDC_MIDI_DEVICE_START: {
                new midi_device::launchpad::Launchpad
                break;
            }
            case IDC_MIDI_DEVICE_REFRESH: {
                macropad::RefreshDevicesList();
                break;
            }
            case IDCANCEL:
                EndDialog(hdlg, IDCANCEL);
                break;
            }

            break;
        default:
            return FALSE;
        }
        return FALSE;
    }

    //
    //  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
    //
    //  PURPOSE: Processes messages for the main window.
    //
    //  WM_COMMAND  - process the application menu
    //  WM_PAINT    - Paint the main window
    //  WM_DESTROY  - post a quit message and return
    //
    //
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    // Message handler for about box.
    LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);
        switch (message)
        {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
        return (INT_PTR)FALSE;
    }
}


void macropad::RefreshButtonList() {
    midi_device::launchpad::launchpad_grid* buttons = midi_device::launchpad::Launchpad::GetDevice()->getCurrentButtons();

    ClearButtonList();

    if (buttons != nullptr) {
        for (size_t x = 0; x < buttons->size(); x++) {
            for (size_t y = 0; y < buttons->at(x).size(); y++) {
                midi_device::launchpad::config::ButtonBase* button = buttons->at(x).at(y);
                std::wstring str = std::to_wstring(midi_device::launchpad::commands::calculate_grid(x, y)) + L" | x= " + std::to_wstring(x) + L" y= " + std::to_wstring(y) + L" | ";

                if (button == nullptr) {
                    str += L"null button";
                }
                else {
                    str += button->to_wstring();
                }

                ListBox_AddString(macropad::hList_debug_help, str.c_str());
            }
        }
    }
    else {
        ListBox_AddString(macropad::hList_debug_help, L"this page is empty!");
    }
}


void macropad::ClearButtonList() {
    for (int i = ListBox_GetCount(macropad::hList_debug_help); i >= 0; i--) {
        ListBox_DeleteString(macropad::hList_debug_help, i);
    }
}

void macropad::RefreshDevicesList()
{
    for (int i = ComboBox_GetCount(macropad::hCombo_Midi_Ins); i >= 0; i--) {
        ComboBox_DeleteString(macropad::hCombo_Midi_Ins, i);
    }

    for (int i = ComboBox_GetCount(macropad::hCombo_Midi_Outs); i >= 0; i--) {
        ComboBox_DeleteString(macropad::hCombo_Midi_Outs, i);
    }


    size_t nPorts = midi_in->getPortCount();
    std::string portName;
    for (size_t i = 0; i < nPorts; i++) {
        try {
            portName = midi_in->getPortName(i);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }

        std::wstring conv = string_to_wstring(portName);

        ComboBox_AddString(macropad::hCombo_Midi_Ins, conv.c_str());
    }

    nPorts = midi_out->getPortCount();
    for (size_t i = 0; i < nPorts; i++) {
        try {
            portName = midi_out->getPortName(i);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }

        std::wstring conv = string_to_wstring(portName);

        ComboBox_AddString(macropad::hCombo_Midi_Outs, conv.c_str());
    }

    
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, macropad::szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MACROPAD, macropad::szWindowClass, MAX_LOADSTRING);
    macropad::MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!macropad::InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MACROPAD));

    MSG msg;

    std::thread launchpad_thread(midi_device::launchpad::Launchpad::RunDevice);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    midi_device::launchpad::Launchpad::TerminateDevice();
    launchpad_thread.join();

    return (int)msg.wParam;
}



void _DebugString(std::wstring s) {
    OutputDebugStringW(s.c_str());
}

void _DebugString(std::string s) {
    OutputDebugStringA(s.c_str());
}