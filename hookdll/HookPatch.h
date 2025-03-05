#ifndef HOOKPATCH_H
#define HOOKPATCH_H

#include <windows.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <capstone/capstone.h>
#include "UnifiedHookHandler.h"

class HookPatch {
public:
    HookPatch(void* target, void* hook);

    bool patch();

    bool unpatch();

    void* getTrampolineAddress();

private:
    void* getRealFunctionAddress(void* jmpStubAddr);

	size_t calculatePatchSize(size_t minSize);

    size_t writeAbsoluteJump64(void* absJumpMemory, void* addrToJumpTo);
     
    void* targetAddress;
    void* hookAddress;
    void* trampolineAddress;
    size_t patchSize;
    std::vector<uint8_t> originalBytes;
};

#endif // HOOKPATCH_H