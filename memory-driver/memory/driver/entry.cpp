#include "../kdm/kdmapper.hpp"
#include "entry.h"
#include "byte.h"
using namespace std;

#if defined(DISABLE_OUTPUT)
#define Log(content) 
#else
#define Log(content) std::wcout << content
#endif

#ifndef USERMODE

size_t arraySize = sizeof(Driver) / sizeof(Driver[0]);

void createDriver() {

	ofstream outFile("kernel.sys", ios::binary);

	if (!outFile) {
		cerr << "Create failed" << endl;
		return;
	}

	outFile.write(reinterpret_cast<const char*>(Driver), arraySize);

	if (!outFile) {
		cerr << "Write failed" << endl;
		return;
	}

	outFile.close();
	cout << "Write success" << endl;
	return;
}

HANDLE iqvw64e_device_handle;


LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		Log(L"[!!] Crash at addr 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << L" by 0x" << hex << ExceptionInfo->ExceptionRecord->ExceptionCode << endl);
	else
		Log(L"[!!] Crash" << endl);

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;
}

bool callbackEx(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize) {
	// bool callbackEx(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	// UNREFERENCED_PARAMETER(mdlptr);
	Log("[+] Callbacked" << endl);

	/*
	This callback occurs before call driver entry and
	can be usefull to pass more customized params in
	the last step of the mapping procedure since you
	know now the mapping address and other things
	*/
	return true;
}

int kdmap(const int argc, wchar_t** argv) {
	SetUnhandledExceptionFilter(SimplestCrashHandler);

	bool free = false;
	bool mdlMode = false;
	bool indPagesMode = false;
	bool passAllocationPtr = false;

	if (free) {
		Log(L"[+] Free pool memory after usage enabled" << endl);
	}

	if (mdlMode) {
		Log(L"[+] Mdl memory usage enabled" << endl);
	}

	if (indPagesMode) {
		Log(L"[+] Allocate Independent Pages mode enabled" << endl);
	}

	if (passAllocationPtr) {
		Log(L"[+] Pass Allocation Ptr as first param enabled" << endl);
	}


	const wstring driver_path = L"kernel.sys";//argv[drvIndex];


	iqvw64e_device_handle = intel_driver::Load();

	if (iqvw64e_device_handle == INVALID_HANDLE_VALUE) {
		return -1;
	}

	vector<uint8_t> raw_image = { 0 };
	if (!utils::ReadFileToMemory(driver_path, &raw_image)) {
		Log(L"[-] Failed to read image to memory" << endl);
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}

	kdmapper::AllocationMode mode = kdmapper::AllocationMode::AllocatePool;

	if (mdlMode && indPagesMode) {
		Log(L"[-] Too many allocation modes" << endl);
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}
	else if (mdlMode) {
		mode = kdmapper::AllocationMode::AllocateMdl;
	}
	else if (indPagesMode) {
		mode = kdmapper::AllocationMode::AllocateIndependentPages;
	}

	NTSTATUS exitCode = 0;
	if (!kdmapper::MapDriver(iqvw64e_device_handle, raw_image.data(), 0, 0, free, true, mode, passAllocationPtr, callbackEx, &exitCode)) {
		Log(L"[-] Failed to map " << driver_path << endl);
		intel_driver::Unload(iqvw64e_device_handle);
		return -1;
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		Log(L"[-] Warning failed to fully unload vulnerable driver " << endl);
	}
	Log(L"[+] success" << endl);
}

#endif

void load_driver()
{
	createDriver();
	kdmap(1, nullptr);
	remove("kernel.sys");
	system("cls");
}