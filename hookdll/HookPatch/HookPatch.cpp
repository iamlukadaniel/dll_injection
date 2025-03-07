#include "HookPatch.h"

HookPatch::HookPatch(void* target, void* hook)
	: targetAddress(getRealFunctionAddress(target)),
	hookAddress(hook),
	trampolineAddress(nullptr),
	patchSize(0) {}

bool HookPatch::patch() {
	const size_t jumpSize = 13; // 12 bytes for: MOV RAX, imm64; JMP RAX
	patchSize = calculatePatchSize(jumpSize);
	if (patchSize < jumpSize) {
		std::cerr << "Not enough bytes to install hook." << std::endl;
		return false;
	}

	originalBytes.resize(patchSize);
	memcpy(originalBytes.data(), targetAddress, patchSize);

	trampolineAddress = VirtualAlloc(nullptr, patchSize + jumpSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!trampolineAddress) {
		std::cerr << "Failed to allocate memory for trampoline." << std::endl;
		return false;
	}

	memcpy(trampolineAddress, originalBytes.data(), patchSize);

	writeAbsoluteJump64(reinterpret_cast<uint8_t*>(trampolineAddress) + patchSize,
		reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(targetAddress) + patchSize));

	g_TrampolineAddress = reinterpret_cast<uint64_t>(trampolineAddress);

	DWORD oldProtect;
	if (!VirtualProtect(targetAddress, patchSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		std::cerr << "VirtualProtect failed during patching." << std::endl;
		return false;
	}

	writeAbsoluteJump64(targetAddress, hookAddress);

	if (patchSize > jumpSize) {
		size_t nopCount = patchSize - jumpSize;
		memset(reinterpret_cast<uint8_t*>(targetAddress) + jumpSize, 0x90, nopCount);
	}

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

	if (trampolineAddress) {
		VirtualFree(trampolineAddress, 0, MEM_RELEASE);
		trampolineAddress = nullptr;
	}

	return true;
}

void* HookPatch::getTrampolineAddress()
{
	return trampolineAddress;
}

void* HookPatch::getRealFunctionAddress(void* jmpStubAddr)
{
	uint8_t* jmpStubBytes = reinterpret_cast<uint8_t*>(jmpStubAddr);

	if (jmpStubBytes[0] == 0xE9) {
		int32_t offset = *reinterpret_cast<int32_t*>(jmpStubBytes + 1);
		return jmpStubBytes + 5 + offset;
	}
	else if (jmpStubBytes[0] == 0xFF && jmpStubBytes[1] == 0x25) {
		int32_t disp = *reinterpret_cast<int32_t*>(jmpStubBytes + 2);
		unsigned char* addrToRead = jmpStubBytes + 6 + disp;
		return *reinterpret_cast<void**>(addrToRead);
	}
	return jmpStubAddr;
}

size_t HookPatch::calculatePatchSize(size_t minSize) {
	csh handle;
	cs_insn* insn = nullptr;
	size_t totalLength = 0;

	if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
		std::cerr << "Capstone initialization failed." << std::endl;
		return 0;
	}
	cs_option(handle, CS_OPT_DETAIL, CS_OPT_OFF);

	size_t count = cs_disasm(handle, reinterpret_cast<uint8_t*>(targetAddress), 32, reinterpret_cast<uint64_t>(targetAddress), 0, &insn);
	if (count > 0) {
		for (size_t i = 0; i < count; i++) {
			totalLength += insn[i].size;
			if (totalLength >= minSize)
				break;
		}
		cs_free(insn, count);
	}
	else {
		std::cerr << "Failed to disassemble target function." << std::endl;
	}
	cs_close(&handle);
	return totalLength;
}

size_t HookPatch::writeAbsoluteJump64(void* absJumpMemory, void* addrToJumpTo)
{
	uint8_t absJumpInstructions[] = { 0x49, 0xBA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //mov 64 bit value into r10
										0x41, 0xFF, 0xE2 };

	uint64_t addrToJumpTo64 = (uint64_t)addrToJumpTo;
	memcpy(&absJumpInstructions[2], &addrToJumpTo64, sizeof(addrToJumpTo64));
	memcpy(absJumpMemory, absJumpInstructions, sizeof(absJumpInstructions));
	return sizeof(absJumpInstructions);
}