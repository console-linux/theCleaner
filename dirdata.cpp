#include "dirdata.h"
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

uint64_t get_user_folder_size(const std::string& username) {
    fs::path user_path = "C:/Users/" + username;
    uint64_t total_size = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(user_path)) {
            if (entry.is_regular_file()) {
                total_size += entry.file_size();
            }
        }
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Failed to calculate folder size: " + std::string(err.what()));
    }

    return total_size;
}

std::chrono::system_clock::time_point get_user_folder_creation_time(const std::string& username) {
    fs::path user_path = "C:/Users/" + username;

    try {
        // Use status to get file attributes including birth time
        auto file_status = fs::status(user_path);

        // Get the birth time (creation time) - this is available in C++20
        auto birth_time = fs::last_write_time(user_path); // Fallback to last write time

        // Try to get birth time if supported by the filesystem
        #ifdef _WIN32
        // On Windows, we can use the status to try to get creation time
        // Note: std::filesystem doesn't directly expose birth_time in all implementations
        // We'll use last_write_time as fallback, but in practice you might need platform-specific code
        #endif

        return std::chrono::clock_cast<std::chrono::system_clock>(birth_time);
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Failed to get folder creation time: " + std::string(err.what()));
    }
}

uint64_t get_free_space_on_c_drive() {
    try {
        fs::space_info si = fs::space("C:/");
        return si.available;
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Failed to get free space: " + std::string(err.what()));
    }
}

std::vector<std::string> get_user_list(const std::vector<std::string>& whitelist) {
    std::vector<std::string> users;
    fs::path users_path = "C:/Users";

    // Default system folders to always exclude
    std::vector<std::string> default_exclusions = {"Public", "Default", "Default User", "All Users", "desktop.ini"};

    // Get current date and calculate September 1st of current year
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);

    // Create September 1st of current year
    std::tm september_tm = {};
    september_tm.tm_year = now_tm.tm_year; // Current year
    september_tm.tm_mon = 8; // September (0-based: 0=Jan, 8=Sep)
    september_tm.tm_mday = 1; // 1st day
    september_tm.tm_hour = 0;
    september_tm.tm_min = 0;
    september_tm.tm_sec = 0;
    september_tm.tm_isdst = -1; // Let system determine DST

    auto september_time = std::mktime(&september_tm);
    auto september_point = std::chrono::system_clock::from_time_t(september_time);

    try {
        for (const auto& entry : fs::directory_iterator(users_path)) {
            if (entry.is_directory()) {
                std::string username = entry.path().filename().string();

                // Check if this user should be excluded
                bool is_excluded = std::find(default_exclusions.begin(), default_exclusions.end(), username) != default_exclusions.end();
                bool is_whitelisted = std::find(whitelist.begin(), whitelist.end(), username) != whitelist.end();

                if (!is_excluded && !is_whitelisted) {
                    try {
                        // Check creation time - only include if created after September 1st
                        auto creation_time = get_user_folder_creation_time(username);
                        if (creation_time > september_point) {
                            users.push_back(username);
                        }
                    }
                    catch (const std::runtime_error&) {
                        // If we can't get creation time, skip this user
                        continue;
                    }
                }
            }
        }
    }
    catch (const fs::filesystem_error& err) {
        throw std::runtime_error("Failed to get user list: " + std::string(err.what()));
    }

    return users;
}
