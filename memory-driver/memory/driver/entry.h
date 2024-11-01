#pragma once

void createDriver();
LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo);
bool callbackEx(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize);
int kdmap(const int argc, wchar_t** argv);
void load_driver();