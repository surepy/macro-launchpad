#include "json.hpp"
#include "framework.h"

namespace config {
    HANDLE file_handle;
    std::wstring file_path;
}

void test() {
    nlohmann::json j;
}

int config::loadFile() {
    file_handle = CreateFileW(file_path.c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == NULL)
        return GetLastError();
    return 0;
};