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

std::vector<std::string> getUserList(const std::vector<std::string>& whitelist) {
    std::vector<std::string> users;
    fs::path usersPath = "C:/Users";

    std::vector<std::string> defaultExclusions = {"Public", "Default", "Default User", "All Users", "desktop.ini"};

    auto now = std::chrono::system_clock::now();
    auto nowTimeT = std::chrono::system_clock::to_time_t(now);
    std::tm nowTm = *std::localtime(&nowTimeT);

    std::tm septemberTm = {};
    septemberTm.tm_year = nowTm.tm_year;
    septemberTm.tm_mon = 8;
    septemberTm.tm_mday = 1;
    septemberTm.tm_hour = 0;
    septemberTm.tm_min = 0;
    septemberTm.tm_sec = 0;
    septemberTm.tm_isdst = -1;

    auto septemberTime = std::mktime(&septemberTm);
    auto septemberPoint = std::chrono::system_clock::from_time_t(septemberTime);

    try {
        for (const auto& entry : fs::directory_iterator(usersPath)) {
            if (entry.is_directory()) {
                std::string username = entry.path().filename().string();

                bool isExcluded = std::find(defaultExclusions.begin(), defaultExclusions.end(), username) != defaultExclusions.end();
                bool isWhitelisted = std::find(whitelist.begin(), whitelist.end(), username) != whitelist.end();

                if (!isExcluded && !isWhitelisted) {
                    try {
                        auto creationTime = getUserFolderCreationTime(username);
                        if (creationTime > septemberPoint) {
                            users.push_back(username);
                        }
                    }
                    catch (const std::runtime_error&) {
                        continue;
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Произошла ошибка при получении списка пользователей: " + std::string(err.what()));
    }

    return users;
}
