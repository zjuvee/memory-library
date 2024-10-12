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

        if (pid == NULL || handle == 0) {
            exit(-1);
        }
    }

    DWORD get_pid(const char* processName);
    BOOL set_privilege(HANDLE processhandle, std::string perm);
    BOOL get_debug(void);
    vector<LPVOID> aob_scan(const vector<BYTE>& findPattern);
    BOOL replace_byte(const std::vector<BYTE>& findPattern, const std::vector<BYTE>& replacePattern);
    vector<LPVOID> find_string(const string& searchString);
    BOOL replace_string(const std::string& searchString, const std::string& replaceString);
    const std::uintptr_t get_module(const std::string_view moduleName) const noexcept;

    // you can replace this with physical driver memory read/writing, example: https://github.com/zjuvee/kernel-driver
    template <typename T>
    constexpr const T Read(const std::uintptr_t& address) const noexcept
    {
        T value = { };
        ::ReadProcessMemory(handle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
        return value;
    }

    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
    {
        ::WriteProcessMemory(handle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
    }

    std::string ReadString(const std::uintptr_t& address) const noexcept
    {
        std::vector<char> buffer(256, '\0');
        ::ReadProcessMemory(handle, reinterpret_cast<const void*>(address), buffer.data(), 256, NULL);
        return std::string(buffer.data());
    }

    void WriteString(const std::uintptr_t& address, const std::string& value) const noexcept
    {
        ::WriteProcessMemory(handle, reinterpret_cast<void*>(address), value.c_str(), value.size() + 1, NULL); // +1 para incluir el carácter nulo
    }

};
