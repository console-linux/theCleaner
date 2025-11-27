#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <vector>

// Function declarations
uint64_t getUserFolderSize(const std::string& username);
std::chrono::system_clock::time_point getUserFolderCreationTime(const std::string& username);
uint64_t getFreeSpaceOnCDrive();
std::vector<std::string> getUserList(const std::vector<std::string>& whitelist = {});
