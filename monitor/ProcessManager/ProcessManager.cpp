#include "ProcessManager.h"
#include <tlhelp32.h>
#include <iostream>

DWORD ProcessManager::findProcessId(const std::wstring& processName) {
	DWORD processId = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32W pe;
		pe.dwSize = sizeof(PROCESSENTRY32W);
		if (Process32FirstW(hSnapshot, &pe)) {
			do {
				if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
					processId = pe.th32ProcessID;
					break;
				}
			} while (Process32NextW(hSnapshot, &pe));
		}
		CloseHandle(hSnapshot);
	}
	return processId;
}

bool ProcessManager::injectDLL(DWORD pid, const std::wstring& dllPath) {
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
		PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

	if (!hProcess) {
		std::cerr << "Failed to open process " << pid << std::endl;
		return false;
	}

	size_t size = (dllPath.size() + 1) * sizeof(wchar_t);
	LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pRemoteMemory) {
		std::cerr << "Failed to allocate memory in target process." << std::endl;
		CloseHandle(hProcess);
		return false;
	}

	if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), size, NULL)) {
		std::cerr << "Failed to write data to target process memory." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	auto pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
	if (!pLoadLibrary) {
		std::cerr << "Failed to get address of LoadLibraryA." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pLoadLibrary, pRemoteMemory, 0, NULL);
	if (!hThread) {
		std::cerr << "Failed to create remote thread." << std::endl;
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	return true;
}
