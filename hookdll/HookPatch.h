#ifndef HOOKPATCH_H
#define HOOKPATCH_H

#include <windows.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

class HookPatch {
public:
    // Конструктор принимает адрес целевой функции, адрес функции-хука и размер патча (по умолчанию 12 байт)
    HookPatch(void* target, void* hook, size_t size = 12);

    // Применяет патч: сохраняет оригинальные байты и записывает инструкцию перехода
    bool patch();

    // Восстанавливает оригинальные байты (отменяет патч)
    bool unpatch();

private:
    void* targetAddress;                // Адрес функции, которую патчат
    void* hookAddress;                  // Адрес функции-хука
    size_t patchSize;                   // Размер патча (в байтах)
    std::vector<uint8_t> originalBytes; // Сохранённые оригинальные байты
};

#endif // HOOKPATCH_H