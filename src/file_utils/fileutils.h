#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <memory>
#include <string>
#include <vector>

namespace cmake {
struct CmakeFunctionArgument;
class CmakeFunction;
}

namespace file_utils {

class Directory;

// TODO: this does not really belong here, maybe in the cmakefile
class DirectoryFiles {
public:
  bool empty() const;

  std::vector<cmake::CmakeFunctionArgument> availableFileTypeArguments(const std::string& projectName) const;

  std::shared_ptr<cmake::CmakeFunction> createIncludeFilesFunction(const file_utils::Directory* directory);

  std::shared_ptr<cmake::CmakeFunction> createSourceFilesFunction(const file_utils::Directory* directory);

  std::vector<std::string> includeFiles;
  std::vector<std::string> sourceFiles;
};

class IgnoreFile;

std::string makeRelative(const std::string& target);
std::string makeRelative(const std::string& target, const std::string& rootPath);
std::string directoryName(const std::string& path);
std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile);
DirectoryFiles getFilesForProject(const Directory* directory);

}

#endif
