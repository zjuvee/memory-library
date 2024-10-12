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

#pragma comment(lib, "ntdll.lib")

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

        set_privilege(handle, "SeDebugPrivilege");
        OpenDevice();

        if (pid == NULL || handle == 0) {
            exit(-1);
        }
    }

    ~memory()
    {
        CloseDevice();
    }

    DWORD get_pid(const char* processName);
    BOOL set_privilege(HANDLE processhandle, std::string perm);
    BOOL get_debug(void);
    vector<LPVOID> aob_scan(const vector<BYTE>& findPattern);
    BOOL replace_byte(const std::vector<BYTE>& findPattern, const std::vector<BYTE>& replacePattern);
    vector<LPVOID> find_string(const string& searchString);
    BOOL replace_string(const std::string& searchString, const std::string& replaceString);
    const std::uintptr_t get_module(const std::string_view moduleName) const noexcept;
    void ReadMemory(int pid, PVOID address, SIZE_T size, BYTE* buffer) const;
    void WriteMemory(int pid, PVOID address, PVOID data, SIZE_T size) const;
    void OpenDevice();
    void CloseDevice();

    // leer memoria fisica desde el kernel
    template <typename T>
    constexpr const T Read(const std::uintptr_t& address) const noexcept
    {
        T value = { };
        T buffer{};
        ReadMemory(pid, reinterpret_cast<PVOID>(address), sizeof(T), reinterpret_cast<BYTE*>(&buffer));
        return buffer;
    }

    // escribir memoria fisica desde el kernel
    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
    {
        WriteMemory(pid, reinterpret_cast<PVOID>(address), (PVOID)&value, sizeof(T));  // Conversión explícita de address a PVOID
    }

};
