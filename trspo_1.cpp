#include <windows.h>
#include <iostream>
#include <string>
#include "HookManager.h"

HANDLE __stdcall HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	std::cout << "HookCreateFileA executed for file: " << lpFileName << std::endl;
	return nullptr;
}

BOOL __stdcall HookCloseHandle(HANDLE hObject) {
	std::cout << "HookCloseHandle executed" << std::endl;
	return 0;
}

int main() {
	HMODULE hModule = LoadLibraryA("kernel32.dll");
	FARPROC originalCreateFileA = GetProcAddress(hModule, R"(CreateFileA)");

	if (originalCreateFileA == nullptr) {
		std::cerr << "Failed to get address of CreateFileA!" << std::endl;
		return 1;
	}

	//HookPatch hookPatch = HookPatch::HookPatch(originalCreateFileA, &HookCreateFileA);
	//hookPatch.patch();

	HookManager hookManager;

	hookManager.addHook(originalCreateFileA, reinterpret_cast<void*>(&HookCreateFileA));
	hookManager.addHook("kernel32.dll", "CloseHandle", reinterpret_cast<void*>(&HookCloseHandle));

	CreateFileA("testfile.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	CloseHandle(nullptr);

	//hookPatch.unpatch();
	hookManager.removeHook("kernel32.dll", "CreateFileA");
	hookManager.removeHook("kernel32.dll", "CloseHandle");

	CreateFileA("testfile.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	return 0;
}
