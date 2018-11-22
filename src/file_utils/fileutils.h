#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <memory>
#include <string>

namespace file_utils {

class IgnoreFile;
class Directory;

std::string makeRelative(const std::string& target);
std::string directoryName(const std::string& path);
std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile);

}

#endif
