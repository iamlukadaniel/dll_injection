#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <windows.h>
#include <string>

class ProcessManager {
public:
	static DWORD findProcessId(const std::wstring& processName);
	static bool injectDLL(DWORD pid, const std::wstring& dllPath);
};

#endif // PROCESSMANAGER_H
