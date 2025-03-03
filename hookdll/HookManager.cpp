#include "HookManager.h"
#include <iostream>

bool HookManager::addHook(void* target, void* hook) {
	if (hooks.find(target) != hooks.end()) {
		std::cerr << "Hook already installed for this target." << std::endl;
		return false;
	}
	std::unique_ptr<HookPatch> patch = std::make_unique<HookPatch>(target, hook);
	if (!patch->patch()) {
		std::cerr << "Failed to patch target function." << std::endl;
		return false;
	}
	hooks[target] = std::move(patch);
	return true;
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
	return addHook(target, hook);
}

bool HookManager::removeHook(void* target) {
	auto it = hooks.find(target);
	if (it == hooks.end()) {
		std::cerr << "No hook found for the target function." << std::endl;
		return false;
	}
	if (!it->second->unpatch()) {
		std::cerr << "Failed to unpatch target function." << std::endl;
		return false;
	}
	hooks.erase(it);
	return true;
}

bool HookManager::removeHook(const std::string& libraryName, const std::string& functionName) {
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
	return removeHook(target);
}
