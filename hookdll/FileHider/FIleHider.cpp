#include "FileHider.h"
#include <algorithm>

FileHider& FileHider::getInstance() {
	static FileHider instance;
	return instance;
}

std::string FileHider::normalizePath(const std::string& path) {
	char fullPath[MAX_PATH] = { 0 };
	if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr)) {
		std::string normalizedPath(fullPath);
		std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
		size_t pos = normalizedPath.find_last_of('/');
		return (pos != std::string::npos) ? normalizedPath.substr(pos + 1) : normalizedPath;
	}
	return path;
}

std::wstring FileHider::normalizePath(const std::wstring& path) {
	wchar_t fullPath[MAX_PATH] = { 0 };
	if (GetFullPathNameW(path.c_str(), MAX_PATH, fullPath, nullptr)) {
		std::wstring normalizedPath(fullPath);
		std::replace(normalizedPath.begin(), normalizedPath.end(), L'\\', L'/');
		size_t pos = normalizedPath.find_last_of(L'/');
		return (pos != std::wstring::npos) ? normalizedPath.substr(pos + 1) : normalizedPath;
	}
	return path;
}

bool FileHider::shouldHideFile(const std::string& fileName) const {
	std::string normalized = normalizePath(fileName);
	return hiddenFiles.find(normalized) != hiddenFiles.end();
}

bool FileHider::shouldHideFile(const std::wstring& fileName) const {
	std::wstring normalized = normalizePath(fileName);
	return hiddenFilesW.find(normalized) != hiddenFilesW.end();
}

void FileHider::addHiddenFile(const std::string& hiddenFile) {
	hiddenFiles.insert(normalizePath(hiddenFile));
}

void FileHider::addHiddenFile(const std::wstring& hiddenFile) {
	hiddenFilesW.insert(normalizePath(hiddenFile));
}

void FileHider::removeHiddenFile(const std::string& hiddenFile) {
	hiddenFiles.erase(normalizePath(hiddenFile));
}

void FileHider::removeHiddenFile(const std::wstring& hiddenFile) {
	hiddenFilesW.erase(normalizePath(hiddenFile));
}

void FileHider::clearHiddenFiles() {
	hiddenFiles.clear();
	hiddenFilesW.clear();
}
