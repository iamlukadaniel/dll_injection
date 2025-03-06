#include "HookHandlers.h"

HANDLE(*OriginalCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HANDLE(*OriginalCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
HANDLE(*OriginalFindFirstFileA)(LPCSTR, LPWIN32_FIND_DATAA);
HANDLE(*OriginalFindFirstFileW)(LPCWSTR, LPWIN32_FIND_DATAW);
BOOL(*OriginalFindNextFileA)(HANDLE, LPWIN32_FIND_DATAA);
BOOL(*OriginalFindNextFileW)(HANDLE, LPWIN32_FIND_DATAW);

Pipe& pipe = Pipe::getInstance();

void HookHandlers::HookLogging() {
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
		<< HookManager::getInstance().getLastHookedFuncName() << " executed";

	std::string message = oss.str();
	pipe.sendMessage(message);

	return;
}

HANDLE __stdcall HookHandlers::HookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (FileHider::getInstance().shouldHideFile(lpFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
		std::cout << HookManager::getInstance().getLastHookedFuncName() << std::endl;
	}

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "CreateFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE __stdcall HookHandlers::HookCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	std::wstring wstr(lpFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (FileHider::getInstance().shouldHideFile(strFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "CreateFileW");

	memcpy((void*)(&OriginalCreateFileW), &trampolineAddress, sizeof(uint64_t));
	return OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE __stdcall HookHandlers::HookFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	HANDLE hFind = nullptr;
	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindFirstFileA");
	memcpy((void*)(&OriginalFindFirstFileA), &trampolineAddress, sizeof(uint64_t));

	do {
		hFind = OriginalFindFirstFileA(lpFileName, lpFindFileData);
		if (hFind == INVALID_HANDLE_VALUE) return hFind;
	} while (FileHider::getInstance().shouldHideFile(lpFindFileData->cFileName));

	return hFind;
}

HANDLE __stdcall HookHandlers::HookFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
	HANDLE hFind = nullptr;
	std::wstring wstr(lpFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindFirstFileW");
	memcpy((void*)(&OriginalFindFirstFileW), &trampolineAddress, sizeof(uint64_t));

	do {
		hFind = OriginalFindFirstFileW(lpFileName, lpFindFileData);
		if (hFind == INVALID_HANDLE_VALUE) return hFind;
	} while (FileHider::getInstance().shouldHideFile(strFileName));

	return hFind;
}

// ====================== FindNextFileA / FindNextFileW ======================

BOOL __stdcall HookHandlers::HookFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	BOOL result = FALSE;
	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindNextFileA");
	memcpy((void*)(&OriginalFindNextFileA), &trampolineAddress, sizeof(uint64_t));

	do {
		result = OriginalFindNextFileA(hFindFile, lpFindFileData);
		if (!result) return FALSE;
	} while (FileHider::getInstance().shouldHideFile(lpFindFileData->cFileName));

	return result;
}

BOOL __stdcall HookHandlers::HookFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
	BOOL result = FALSE;
	std::wstring wstr(lpFindFileData->cFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindNextFileW");
	memcpy((void*)(&OriginalFindNextFileW), &trampolineAddress, sizeof(uint64_t));

	do {
		result = OriginalFindNextFileW(hFindFile, lpFindFileData);
		if (!result) return FALSE;
	} while (FileHider::getInstance().shouldHideFile(strFileName));

	return result;
}
