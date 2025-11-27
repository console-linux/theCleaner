#include "dirdata.h"
#include "sessionmanager.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <locale>
#include <codecvt>

using namespace std;

bool cmp(const pair<string, uint64_t>& a, const pair<string, uint64_t>& b) {
    return a.second < b.second;
}
wstring string_to_wstring(const string& str) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}
int main() {
    vector<string> whitelist = {"admin", "user"};
    uint64_t freeSpace = getFreeSpaceOnCDrive();
    const uint64_t TARGET_SPACE = 5'000'000'000;
    if (freeSpace > TARGET_SPACE) {
        wcout << L"Доступное пространство на диске C больше 5GB, программа завершает работу.";
        return 0;
    } else {
        vector<string> users = getUserList(whitelist);
        vector<pair<string, uint64_t>> usersAndSizes;
        for (const auto& user : users) {
            uint64_t size = getUserFolderSize(user);
            usersAndSizes.emplace_back(user, size);
        }
        sort(usersAndSizes.begin(), usersAndSizes.end(), cmp);
        while (freeSpace < TARGET_SPACE && !usersAndSizes.empty()) {
            auto& user = usersAndSizes.back();
            wstring wideUser = string_to_wstring(user.first);
            UserSessionManager::ForceDisconnectUser(wideUser);
            freeSpace += user.second;
            usersAndSizes.pop_back();
        }
    }
    return 0;
}
