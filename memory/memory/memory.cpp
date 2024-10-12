#include "memory.h"

extern "C" NTSTATUS ZwReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PULONG NumberOfBytesReaded);
extern "C" NTSTATUS ZwWriteVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, LPCVOID Buffer, SIZE_T BufferSize, PSIZE_T NumberOfBytesWritten);

// obtener el pid de un proceso por el nombre
DWORD memory::get_pid(const char* processName)
{
    if (processName == NULL)
        return 0;
    DWORD pid = 0;
    DWORD threadCount = 0;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;

    pe.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnap, &pe);
    while (Process32Next(hSnap, &pe)) {
        if (_tcsicmp(pe.szExeFile, processName) == 0) {
            if ((int)pe.cntThreads > threadCount) {
                threadCount = pe.cntThreads;

                pid = pe.th32ProcessID;

            }
        }
    }
    return pid;
}

// cambiar los tokens del proceso
BOOL memory::set_privilege(HANDLE processhandle, std::string perm)
{
    const char* permchar = perm.c_str();
    HANDLE tokenhandle;
    LUID permissionidentifier;
    TOKEN_PRIVILEGES tokenpriv;
    if (OpenProcessToken(processhandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenhandle))
    {
        if (LookupPrivilegeValue(NULL, permchar, &permissionidentifier))
        {
            tokenpriv.PrivilegeCount = 1;
            tokenpriv.Privileges[0].Luid = permissionidentifier;
            tokenpriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (AdjustTokenPrivileges(tokenhandle, false, &tokenpriv, sizeof(tokenpriv), NULL, NULL)) { return true; }
            else { return false; }
        }
        else { return false; }
    }
    else { return false; }
    CloseHandle(tokenhandle);
}

// obtener privilegios de debug
BOOL memory::get_debug(void) {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        return FALSE;

    if (!set_privilege(handle, "SeDebugPrivilege"))
        return FALSE; 

    return TRUE;
}

// pattern scan / aob scan
vector<LPVOID> memory::aob_scan(const vector<BYTE>& findPattern) {
    vector<LPVOID> addresses;

    if (pid == 0 || handle == NULL) return addresses;

    LPVOID baseAddress = nullptr;

    while (true) {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (VirtualQueryEx(handle, baseAddress, &memoryInfo, sizeof(memoryInfo)) == 0) {
            break;
        }

        if (memoryInfo.State == MEM_COMMIT &&
            (memoryInfo.Protect & PAGE_READONLY ||
                memoryInfo.Protect & PAGE_READWRITE ||
                memoryInfo.Protect & PAGE_WRITECOPY ||
                memoryInfo.Protect & PAGE_EXECUTE_READ ||
                memoryInfo.Protect & PAGE_EXECUTE_READWRITE ||
                memoryInfo.Protect & PAGE_EXECUTE_WRITECOPY)) {
            const size_t bufferSize = memoryInfo.RegionSize;
            vector<BYTE> buffer(bufferSize);
            ULONG bytesRead;
            if (ZwReadVirtualMemory(handle, memoryInfo.BaseAddress, &buffer[0], bufferSize, &bytesRead) == 0) {
                auto foundIter = search(buffer.begin(), buffer.end(), findPattern.begin(), findPattern.end());
                while (foundIter != buffer.end()) {
                    LPVOID byteAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(memoryInfo.BaseAddress) + distance(buffer.begin(), foundIter));
                    addresses.push_back(byteAddress);
                    foundIter = search(foundIter + 1, buffer.end(), findPattern.begin(), findPattern.end());
                }
            }
        }

        baseAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(baseAddress) + memoryInfo.RegionSize);
    }
    return addresses;
}

// reemplazar byte pattern
BOOL memory::replace_byte(const std::vector<BYTE>& findPattern, const std::vector<BYTE>& replacePattern) {

    size_t RepByteSize = replacePattern.size();
    if (RepByteSize <= 0) return false;

    std::vector<LPVOID> addresses = memory::aob_scan(findPattern);
    if (addresses.empty()) return false;

    for (const auto& address : addresses)
    {
        SIZE_T bytesWritten;
        memory::set_privilege(handle, "SeDebugPrivilege");
        ZwWriteVirtualMemory(handle, address, replacePattern.data(), RepByteSize, &bytesWritten);
    }
    return true;
}


// pasar string a hexadecimal
vector<BYTE> string_to_bytes(const string& str) {
    vector<BYTE> bytes;
    for (char c : str) {
        bytes.push_back(static_cast<BYTE>(c));
    }
    return bytes;
}

// buscar en la memoria un string usando aob scan
vector<LPVOID> memory::find_string(const string& searchString) {
    vector<BYTE> pattern = string_to_bytes(searchString);
    return memory::aob_scan(pattern);
}

// buscar y reemplazar string
BOOL memory::replace_string(const std::string& searchString, const std::string& replaceString) {
    std::vector<BYTE> searchPattern = string_to_bytes(searchString);
    std::vector<BYTE> replacePattern = string_to_bytes(replaceString);

    return replace_byte(searchPattern, replacePattern);
}

// obtener el modulo base de un proceso
const std::uintptr_t memory::get_module(const std::string_view moduleName) const noexcept
{
    ::MODULEENTRY32 entry = { };
    entry.dwSize = sizeof(::MODULEENTRY32);

    const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

    std::uintptr_t result = 0;

    while (::Module32Next(snapShot, &entry))
    {
        if (!moduleName.compare(entry.szModule))
        {
            result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
            break;
        }
    }

    if (snapShot)
        ::CloseHandle(snapShot);

    return result;
}

