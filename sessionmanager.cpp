#include "sessionmanager.h"
#include <windows.h>
#include <wtsapi32.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <sddl.h>
#include <fileapi.h>
#include <shellapi.h>
#include <iostream>

#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

bool UserSessionManager::HasAdminPrivileges() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

bool UserSessionManager::ForceTerminateUserProcesses(const std::wstring& username) {
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    if (sessionId == 0xFFFFFFFF) return false;

    PWTS_PROCESS_INFO processes = nullptr;
    DWORD count = 0;
    bool success = true;

    if (WTSEnumerateProcessesW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &processes, &count)) {
        for (DWORD i = 0; i < count; i++) {
            if (processes[i].SessionId == sessionId && processes[i].pUserName) {
                if (wcscmp(processes[i].pUserName, username.c_str()) == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processes[i].ProcessId);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            }
        }
        WTSFreeMemory(processes);
    }

    return success;
}

bool UserSessionManager::ForceDeleteUserFolder(const std::wstring& username) {
    std::wstring profilePath = L"C:\\Users\\" + username;

    // Принудительное удаление через файловую систему
    SHFILEOPSTRUCTW fileOp = {0};
    std::wstring from = profilePath + L'\0';

    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from.c_str();
    fileOp.fFlags = FOF_NO_UI | FOF_SILENT | FOF_NOCONFIRMATION;

    return SHFileOperationW(&fileOp) == 0;
}

bool UserSessionManager::ForceDisconnectUser(const std::wstring& username) {
    if (!HasAdminPrivileges()) {
        std::wcout << L"Требуются права администратора" << std::endl;
        return false;
    }

    if (username.empty()) {
        std::wcout << L"Неверное имя пользователя" << std::endl;
        return false;
    }

    // Принудительное завершение процессов
    ForceTerminateUserProcesses(username);

    // Даем время процессам завершиться
    Sleep(1000);

    // Принудительное удаление папки
    return ForceDeleteUserFolder(username);

    Sleep(30000);
}
