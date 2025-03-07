#include <windows.h>
#include <iostream>
#include <string>
#include "common/StringConverter.h"
#include <fcntl.h>
#include <io.h>

void testFindFiles() {
	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(L"testfile.txt", &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::cerr << "FindFirstFileW failed! Error: " << GetLastError() << std::endl;
		return;
	}

	std::wcout << L"FindFirstFileW: Found file: " << findData.cFileName << std::endl;

	while (FindNextFileW(hFind, &findData)) {
		std::wcout << L"FindNextFileW: Found file: " << findData.cFileName << std::endl;
	}

	FindClose(hFind);
}

int main() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	_setmode(_fileno(stdout), _O_U16TEXT);

	system("pause");

	CreateFileW(L"тест.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	
	testFindFiles();

	system("pause");

	return 0;
}
