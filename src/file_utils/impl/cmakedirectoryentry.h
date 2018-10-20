#ifndef FILEUTILS_CMAKEDIRECTORYENTRY_H
#define FILEUTILS_CMAKEDIRECTORYENTRY_H
#include <memory>
#include <string>
#include <vector>

namespace file_utils {

class CmakeDirectoryEntry {
public:
  CmakeDirectoryEntry(const std::string& path);
  CmakeDirectoryEntry(const std::string& path, std::shared_ptr<CmakeDirectoryEntry> parent);

  const std::string& path() const;
  std::shared_ptr<std::string> cmakeFilePath() const;
  CmakeDirectoryEntry* parent() const;
  bool hasParent() const;
  bool hasCmakeFilePath() const;
  const std::vector<std::string>& includeFiles() const;
  const std::vector<std::string>& sourceFiles() const;

  void addCmakeFilePath(const std::string& path);
  void addIncludeFiles(const std::vector<std::string>& files);
  void addSourceFiles(const std::vector<std::string>& files);


private:
  std::string path_;
  std::shared_ptr<std::string> cmakeFilePath_;
  std::shared_ptr<CmakeDirectoryEntry> parent_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
};

}

#endif
