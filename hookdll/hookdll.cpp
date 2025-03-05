#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdio>
#include "HookManager.h"
#include "UnifiedHookHandler.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

#define PIPE_NAME "\\\\.\\pipe\\HookPipe"

HANDLE g_hPipe = INVALID_HANDLE_VALUE;
std::string g_mode;
std::string g_param;

HookManager& g_hookManager = HookManager::getInstance();

std::string NormalizePath(const std::string& path) {
	char fullPath[MAX_PATH] = { 0 };
	if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr)) {
		std::string normalizedPath(fullPath);
		std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
		size_t pos = normalizedPath.find_last_of('/');
		return (pos != std::string::npos) ? normalizedPath.substr(pos + 1) : normalizedPath;
	}
	return path;
}

bool ShouldHideFile(const std::string& fileName) {
	std::string normalized = NormalizePath(fileName);
	if (_stricmp(normalized.c_str(), g_param.c_str()) == 0) {
			return true;
	}
	return false;
}

void HookLogging() {
	using namespace std::chrono;
	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
	std::time_t t = system_clock::to_time_t(now);
	std::tm local_tm;
	localtime_s(&local_tm, &t);

	std::ostringstream oss;
	oss << "[" << std::put_time(&local_tm, "%H:%M:%S")
		<< '.' << std::setfill('0') << std::setw(3)
		<< ms.count() << "] "
		<< g_hookManager.getLastHookedFuncName() << " executed";

	std::string message = oss.str();
	DWORD bytesWritten;
	if (!WriteFile(g_hPipe, message.c_str(), (DWORD)message.size(), &bytesWritten, NULL)) {
		CloseHandle(g_hPipe);
	}

	return;
}

HANDLE(*OriginalCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HANDLE(*OriginalCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HANDLE(*OriginalFindFirstFileA)(LPCSTR, LPWIN32_FIND_DATAA);
HANDLE(*OriginalFindFirstFileW)(LPCWSTR, LPWIN32_FIND_DATAW);
BOOL(*OriginalFindNextFileA)(HANDLE, LPWIN32_FIND_DATAA);
BOOL(*OriginalFindNextFileW)(HANDLE, LPWIN32_FIND_DATAW);

HANDLE __stdcall HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (ShouldHideFile(lpFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "CreateFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE __stdcall HookCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	std::wstring wstr(lpFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (ShouldHideFile(strFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "CreateFileW");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE __stdcall HookFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	if (ShouldHideFile(lpFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "FindFirsFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindFirstFileA(lpFileName, lpFindFileData);
}

HANDLE __stdcall HookFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
	std::wstring wstr(lpFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (ShouldHideFile(strFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "FindFirsFileW");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindFirstFileW(lpFileName, lpFindFileData);
}

BOOL __stdcall HookFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	if (ShouldHideFile(lpFindFileData->cFileName)) {
		SetLastError(ERROR_NO_MORE_FILES);
		return FALSE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "FindNextFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindNextFileA(hFindFile, lpFindFileData);
}

BOOL __stdcall HookFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
	std::wstring wstr(lpFindFileData->cFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (ShouldHideFile(strFileName)) {
		SetLastError(ERROR_NO_MORE_FILES);
		return FALSE;
	}

	void* trampolineAddress = g_hookManager.getTrampolineAddress("kernel32.dll", "FindNextFileW");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindNextFileW(hFindFile, lpFindFileData);
}

DWORD WINAPI PipeClientThread(LPVOID lpParam)
{
	while (true)
	{
		g_hPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (g_hPipe != INVALID_HANDLE_VALUE)
			break;
		Sleep(100);
	}

	char buffer[512] = { 0 };
	DWORD bytesRead;
	if (ReadFile(g_hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::string config(buffer);
		size_t modePos = config.find("MODE:");
		size_t paramPos = config.find(";PARAM:");
		if (modePos != std::string::npos && paramPos != std::string::npos)
		{
			g_mode = config.substr(modePos + 5, paramPos - (modePos + 5));
			size_t endPos = config.find("\n", paramPos);
			if (endPos == std::string::npos)
				endPos = config.length();
			g_param = config.substr(paramPos + 7, endPos - (paramPos + 7));
		}
	}

	if (g_mode == "hide")
	{
		g_hookManager.addHook("kernel32.dll", "CreateFileA", reinterpret_cast<void*>(&HookCreateFileA));
		g_hookManager.addHook("kernel32.dll", "CreateFileW", reinterpret_cast<void*>(&HookCreateFileW));
		g_hookManager.addHook("kernel32.dll", "FindFirstFileA", reinterpret_cast<void*>(&FindFirstFileA));
		g_hookManager.addHook("kernel32.dll", "FindFirstFileW", reinterpret_cast<void*>(&FindFirstFileW));
		g_hookManager.addHook("kernel32.dll", "FindNextFileA", reinterpret_cast<void*>(&FindNextFileA));
		g_hookManager.addHook("kernel32.dll", "FindNextFileW", reinterpret_cast<void*>(&FindNextFileW));
	}
	else if (g_mode == "func")
	{
		g_HookFunction = reinterpret_cast<uint64_t>(&HookLogging);
		g_hookManager.addHook("kernel32.dll", g_param, reinterpret_cast<void*>(&UnifiedHookHandler));
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, PipeClientThread, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		if (g_hPipe != INVALID_HANDLE_VALUE)
			CloseHandle(g_hPipe);
		break;
	}
	return TRUE;
}
