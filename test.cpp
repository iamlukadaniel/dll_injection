#include <windows.h>
#include <iostream>
#include <string>

void testFindFiles() {
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA("*.*", &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::cerr << "FindFirstFileA failed! Error: " << GetLastError() << std::endl;
		return;
	}

	std::cout << "FindFirstFileA: Found file: " << findData.cFileName << std::endl;

	while (FindNextFileA(hFind, &findData)) {
		std::cout << "FindNextFileA: Found file: " << findData.cFileName << std::endl;
	}

	FindClose(hFind);
}

int main() {

	system("pause");

	/*CreateFileA("testfile.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);*/

	/*system("pause");*/

	testFindFiles();

	system("pause");

	return 0;
}
