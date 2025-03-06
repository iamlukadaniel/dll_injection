#ifndef HOOKHANDLERS_H
#define HOOKHANDLERS_H

#include <windows.h>
#include <string>
#include "../FileHider/FileHider.h"
#include "../HookManager/HookManager.h"
#include "../../common/Pipe.h"
#include <iostream>
#include <ctime>
#include <cstdio>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

class HookHandlers {
public:
	static void HookLogging();

	static HANDLE __stdcall HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

	static HANDLE __stdcall HookCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

	static HANDLE __stdcall HookFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
	static HANDLE __stdcall HookFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);

	static BOOL __stdcall HookFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
	static BOOL __stdcall HookFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
};

#endif // HOOKHANDLERS_H
