#ifndef FILEHIDER_H
#define FILEHIDER_H

#include <string>
#include <unordered_set>
#include <windows.h>

class FileHider {
public:
	static FileHider& getInstance();

	bool shouldHideFile(const std::string& fileName) const;
	void addHiddenFile(const std::string& hiddenFile);
	void removeHiddenFile(const std::string& hiddenFile);
	void clearHiddenFiles();

private:
	FileHider() = default;
	FileHider(const FileHider&) = delete;
	FileHider& operator=(const FileHider&) = delete;

	static std::string normalizePath(const std::string& path);
	std::unordered_set<std::string> hiddenFiles;
};

#endif // FILEHIDER_H
