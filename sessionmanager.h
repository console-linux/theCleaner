#pragma once
#include <string>

class UserSessionManager {
public:
    static bool ForceDisconnectUser(const std::wstring& username);

private:
    static bool HasAdminPrivileges();
    static bool ForceTerminateUserProcesses(const std::wstring& username);
    static bool ForceDeleteUserFolder(const std::wstring& username);
};
