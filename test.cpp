#include <windows.h>
#include <iostream>
#include <string>

int main() {
	system("pause");

	CreateFileA("testfile.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	CreateFileA("testfile.txt", GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	return 0;
}
