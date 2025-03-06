#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <windows.h>
#include <string>

class ProcessManager {
public:
	static DWORD findProcessId(const std::string& processName);
	static bool injectDLL(DWORD pid, const std::string& dllPath);
};

#endif // PROCESSMANAGER_H
