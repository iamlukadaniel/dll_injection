#include "HookManager.h"
#include <iostream>

HookManager& HookManager::getInstance() {
	static HookManager instance;
	return instance;
}

bool HookManager::addHook(const std::string& libraryName, const std::string& functionName, void* hook) {
	HMODULE hModule = LoadLibraryA(libraryName.c_str());
	if (!hModule) {
		std::cerr << "Failed to load " << libraryName << std::endl;
		return false;
	}

	void* target = reinterpret_cast<void*>(GetProcAddress(hModule, functionName.c_str()));
	if (!target) {
		std::cerr << "Failed to get address of " << functionName << std::endl;
		return false;
	}

	std::string hookKey = libraryName + "!" + functionName;

	if (hooks.find(hookKey) != hooks.end()) {
		std::cerr << "Hook for " << functionName << " already installed." << std::endl;
		return false;
	}

	auto hookPatch = std::make_unique<HookPatch>(target, hook);
	if (!hookPatch->patch()) {
		std::cerr << "Failed to patch target function: " << functionName << std::endl;
		return false;
	}

	hooks[hookKey] = std::move(hookPatch);
	lastHookedFuncName = functionName;
	return true;
}

bool HookManager::removeHook(const std::string& libraryName, const std::string& functionName) {
	std::string hookKey = libraryName + "!" + functionName;

	auto it = hooks.find(hookKey);
	if (it == hooks.end()) {
		std::cerr << "Hook for " << functionName << " not found." << std::endl;
		return false;
	}

	if (!it->second->unpatch()) {
		std::cerr << "Failed to unpatch target function: " << functionName << std::endl;
		return false;
	}

	hooks.erase(it);

	if (hooks.empty()) {
		lastHookedFuncName.clear();
	}

	return true;
}

void HookManager::removeAllHooks() {
	for (auto& [key, hook] : hooks) {
		if (!hook->unpatch()) {
			std::cerr << "Failed to unpatch hook: " << key << std::endl;
		}
	}
	hooks.clear();
	lastHookedFuncName.clear();
}

std::string HookManager::getLastHookedFuncName() {
	return lastHookedFuncName;
}

void* HookManager::getTrampolineAddress(const std::string& libraryName, const std::string& functionName) {
	std::string hookKey = libraryName + "!" + functionName;

	auto it = hooks.find(hookKey);
	if (it == hooks.end()) {
		std::cerr << "Hook for " << functionName << " not found." << std::endl;
		return false;
	}

	return it->second->getTrampolineAddress();
}
