#ifndef FILEUTILS_H
#define FILEUTILS_H
#include <memory>
#include <string>
#include <vector>

namespace file_utils {

class IgnoreFile;

struct CmakeDirectory {
  CmakeDirectory(
    const std::string& path,
    std::shared_ptr<std::string> cmakeFile,
    const std::vector<std::string>& includeFiles,
    const std::vector<std::string>& sourceFiles
  ): path_(path), cmakeFile_(cmakeFile), includeFiles_(includeFiles), sourceFiles_(sourceFiles) {
  }

  std::string path_;
  std::shared_ptr<std::string> cmakeFile_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
};

std::string makeRelative(const std::string& target);

std::vector<CmakeDirectory> getCmakeDirectories(const IgnoreFile& ignoreFile);
std::vector<CmakeDirectory> getPossibleCmakeDirectories(const IgnoreFile& ignoreFile);

}

#endif