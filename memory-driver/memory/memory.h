#pragma once

#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <TlHelp32.h>
#include <tchar.h>
#include <string>
#include <codecvt>
#include <cstdint>
#include <string_view>
#include <vector>

#include "driver/driver.h"
#include "driver/entry.h"

#pragma comment(lib, "ntdll.lib")

#define _is_invalid(v) if(v==NULL) return false
#define _is_invalid(v,n) if(v==NULL) return n
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
    ULONG ProcessId;
    BYTE ObjectTypeNumber;
    BYTE Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    ULONG HandleCount;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef NTSTATUS(NTAPI* FUNC_NtQuerySystemInformation)(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS(NTAPI* FUNC_RtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
typedef NTSTATUS(NTAPI* FUNC_NtDuplicateObject)(HANDLE SourceProcessHandle, HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ACCESS_MASK DesiredAccess, ULONG Attributes, ULONG Options);

enum StatusCode
{
    SUCCEED,
    FAILE_PROCESSID,
    FAILE_HPROCESS,
    FAILE_MODULE,
};

using namespace std;

class memory
{
private:

public:

    // process id y process handle 
    DWORD pid;
    HANDLE handle;

    memory(const char* processName) noexcept
    {
        // obtenemos estos pasandole el nombre del proceso
        pid = get_pid(processName);
        handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        driver.initdriver(pid);
        load_driver();

        set_privilege(handle, "SeDebugPrivilege");

        if (pid == NULL || handle == 0) {
            exit(-1);
        }
    }

    ~memory()
    {
        CloseHandle(handle);
    }

    DWORD get_pid(const char* processName);
    BOOL set_privilege(HANDLE processhandle, std::string perm);
    BOOL get_debug(void);
    vector<LPVOID> aob_scan(const vector<BYTE>& findPattern);
    BOOL replace_byte(const std::vector<BYTE>& findPattern, const std::vector<BYTE>& replacePattern);
    vector<LPVOID> find_string(const string& searchString);
    BOOL replace_string(const std::string& searchString, const std::string& replaceString);
    const std::uintptr_t get_module(const std::string_view moduleName) const noexcept;

    template <typename ReadType>
    ReadType read(DWORD64 Address)
    {
        ReadType buffer;
        driver.readsize(static_cast<uintptr_t>(Address), &buffer, sizeof(ReadType));
        return buffer;
    }

    template <typename ReadType>
    bool write(DWORD64 Address, ReadType& Value, int Size)
    {
        driver.Write((uintptr_t)Address, Value, Size);
        return true;
    }

    template <typename ReadType>
    bool write(DWORD64 Address, ReadType& Value)
    {
        driver.Write((uintptr_t)Address, Value);
        return true;
    }

};
