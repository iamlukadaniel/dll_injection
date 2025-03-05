#ifndef HOOKMANAGER_H
#define HOOKMANAGER_H

#include <windows.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "HookPatch.h"

class HookManager {
public:
	static HookManager& getInstance();

	bool addHook(const std::string& libraryName, const std::string& functionName, void* hook);

	bool removeHook(const std::string& libraryName, const std::string& functionName);

	void removeAllHooks();

	std::string getLastHookedFuncName();

	void* getTrampolineAddress(const std::string& libraryName, const std::string& functionName);

private:
	HookManager() = default;
	HookManager(const HookManager&) = delete;
	HookManager& operator=(const HookManager&) = delete;

	std::unordered_map<std::string, std::unique_ptr<HookPatch>> hooks;
	std::string lastHookedFuncName;
};

#endif // HOOKMANAGER_H
