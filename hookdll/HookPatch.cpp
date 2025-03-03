#include "HookPatch.h"

HookPatch::HookPatch(void* target, void* hook, size_t size)
	: targetAddress(target), hookAddress(hook), patchSize(size) {}

bool HookPatch::patch() {
	originalBytes.resize(patchSize);
	memcpy(originalBytes.data(), targetAddress, patchSize);

	DWORD oldProtect;
	if (!VirtualProtect(targetAddress, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		std::cerr << "VirtualProtect failed during patching." << std::endl;
		return false;
	}

	uint8_t hookInstruction[12];
	hookInstruction[0] = 0x48;
	hookInstruction[1] = 0xB8;
	uintptr_t addr = reinterpret_cast<uintptr_t>(hookAddress);
	memcpy(hookInstruction + 2, &addr, sizeof(addr));
	hookInstruction[10] = 0xFF;
	hookInstruction[11] = 0xE0;

	memcpy(targetAddress, hookInstruction, patchSize);

	VirtualProtect(targetAddress, patchSize, oldProtect, &oldProtect);

	return true;
}

bool HookPatch::unpatch() {
	DWORD oldProtect;
	if (!VirtualProtect(targetAddress, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		std::cerr << "VirtualProtect failed during unpatching." << std::endl;
		return false;
	}

	memcpy(targetAddress, originalBytes.data(), patchSize);

	VirtualProtect(targetAddress, patchSize, oldProtect, &oldProtect);

	return true;
}