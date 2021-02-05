// header.h : include file for standard system include files,
// or project specific include files
//
#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <thread>
#include <tchar.h>
#include <wchar.h>
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"
#include "MidiDevice.h"
#include "Launchpad.h"
#include "Config.h"
#include "macropad.h"


inline std::wstring string_to_wstring(std::string convert) {
    // used to find the buffer size you need for the wide string.
    int buffer_size = MultiByteToWideChar(CP_UTF8, 0, convert.c_str(), -1, nullptr, 0);
    // new obj
    wchar_t* buffer = new wchar_t[buffer_size];
    // convert.
    MultiByteToWideChar(CP_UTF8, 0, convert.c_str(), -1, buffer, buffer_size);
    // get the result
    std::wstring result = std::wstring(buffer);
    delete[] buffer;
    return result;
}