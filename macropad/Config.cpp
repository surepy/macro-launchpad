#include "json.hpp"
#include "framework.h"
#include <filesystem>

namespace config {
    HANDLE file_handle;
    std::filesystem::path file_path = std::filesystem::current_path() / L"config.json";
    nlohmann::json config_file;
};



void test() {
    nlohmann::json j;
}

int config::openFileHandle() {
    file_handle = CreateFileW(file_path.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == NULL)
        return GetLastError();
    return 0;
};

int config::loadFile() {
    std::wstring str = L"";
    const int buffer_size = 1024;
    LARGE_INTEGER file_size;

    GetFileSizeEx(file_handle, &file_size);

    // bytes left over
    long long bytes_left = file_size.QuadPart;
    
    char buffer[buffer_size];
    DWORD dwBtytesRead;
    OVERLAPPED ol = { 0 };

    for (; bytes_left >= 0; bytes_left -= buffer_size) {
        if (FALSE == ReadFile(file_handle, buffer, buffer_size - 1, &dwBtytesRead, &ol))
        {
            return GetLastError();
        }

        // used to find the buffer size you need for the wide string.
        int buffer_2_size = MultiByteToWideChar(CP_UTF8, 0, buffer, dwBtytesRead, nullptr, 0);
        // + 1 for null termination.
        wchar_t *buffer_2 = new wchar_t[buffer_2_size + (int)1];
        // convert.
        MultiByteToWideChar(CP_UTF8, 0, buffer, dwBtytesRead, buffer_2, buffer_2_size);
        // add null termination
        buffer_2[buffer_2_size] = 0x0;
        // apprend.
        str += std::wstring(buffer_2);
    }

    config_file = nlohmann::json::parse(str);

    return 0;
};