#include "dirdata.h"
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

uint64_t getUserFolderSize(const std::string& username) {
    fs::path userPath = "C:/Users/" + username;
    uint64_t totalSize = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(userPath)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Произошла ошибка при подсчете размера папки пользователя: " + std::string(err.what()));
    }

    return totalSize;
}

std::chrono::system_clock::time_point getUserFolderCreationTime(const std::string& username) {
    fs::path userPath = "C:/Users/" + username;

    try {
        auto fileStatus = fs::status(userPath);
        auto birthTime = fs::last_write_time(userPath);

        return std::chrono::clock_cast<std::chrono::system_clock>(birthTime);
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Произошла ошибка при получении времени создания папки пользователя: " + std::string(err.what()));
    }
}

uint64_t getFreeSpaceOnCDrive() {
    try {
        fs::space_info spaceInfo = fs::space("C:/");
        return spaceInfo.available;
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Произошла ошибка при получении свободного места на диске C: " + std::string(err.what()));
    }
}
