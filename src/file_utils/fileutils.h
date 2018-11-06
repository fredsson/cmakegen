#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <memory>
#include <string>

namespace file_utils {

class IgnoreFile;
class Directory;

std::string makeRelative(const std::string& target);
std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile);

}

#endif
