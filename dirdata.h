#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <vector>

// Декларации функций
uint64_t get_user_folder_size(const std::string& username);
std::chrono::system_clock::time_point get_user_folder_creation_time(const std::string& username);
uint64_t get_free_space_on_c_drive();
std::vector<std::string> get_user_list(const std::vector<std::string>& whitelist = {});
