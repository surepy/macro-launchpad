#pragma once
#include <filesystem>
#include "json.hpp"

namespace config {
    extern HANDLE file_handle;
    extern std::filesystem::path file_path;
    extern nlohmann::json config_file;

    int openFileHandle();
    int loadFile();

};