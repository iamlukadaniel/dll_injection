#ifndef HOOKMANAGER_H
#define HOOKMANAGER_H

#include <windows.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "HookPatch.h"

class HookManager {
public:
	bool addHook(void* target, void* hook);

	bool addHook(const std::string& libraryName, const std::string& functionName, void* hook);

	bool removeHook(void* target);

	bool removeHook(const std::string& libraryName, const std::string& functionName);

private:
	std::unordered_map<void*, std::unique_ptr<HookPatch>> hooks;
};

#endif // HOOKMANAGER_H
