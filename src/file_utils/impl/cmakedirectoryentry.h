#ifndef FILEUTILS_CMAKEDIRECTORYENTRY_H
#define FILEUTILS_CMAKEDIRECTORYENTRY_H
#include <memory>
#include <string>
#include <vector>

namespace file_utils {

class CmakeDirectoryEntry {
public:
  CmakeDirectoryEntry(const std::string& path);

  const std::string& path() const;
  std::shared_ptr<std::string> cmakeFilePath() const;

  void addCmakeFilePath(const std::string& path);

private:
  std::string path_;
  std::shared_ptr<std::string> cmakeFilePath_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
};

}

#endif