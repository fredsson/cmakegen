#include "cmakedirectoryentry.h"
#include <algorithm>
namespace file_utils {

CmakeDirectoryEntry::CmakeDirectoryEntry(const std::string& path)
  : path_(path), cmakeFilePath_(nullptr), parent_(nullptr), includeFiles_({}), sourceFiles_({}) {
}

CmakeDirectoryEntry::CmakeDirectoryEntry(const std::string& path, std::shared_ptr<CmakeDirectoryEntry> parent)
  : path_(path), cmakeFilePath_(nullptr), parent_(parent), includeFiles_({}), sourceFiles_({}) {
}

const std::string& CmakeDirectoryEntry::path() const {
  return path_;
}

std::shared_ptr<std::string> CmakeDirectoryEntry::cmakeFilePath() const {
  return cmakeFilePath_;
}

CmakeDirectoryEntry* CmakeDirectoryEntry::parent() const {
  return parent_.get();
}

bool CmakeDirectoryEntry::hasParent() const {
  return !!parent_;
}

bool CmakeDirectoryEntry::hasCmakeFilePath() const {
  return !!cmakeFilePath_;
}

const std::vector<std::string>& CmakeDirectoryEntry::includeFiles() const {
  return includeFiles_;
}

const std::vector<std::string>& CmakeDirectoryEntry::sourceFiles() const {
  return sourceFiles_;
}

void CmakeDirectoryEntry::addCmakeFilePath(const std::string& path) {
  cmakeFilePath_ = std::make_shared<std::string>(path);
}

void CmakeDirectoryEntry::addIncludeFiles(const std::vector<std::string>& files) {
  std::transform(files.begin(), files.end(), std::back_inserter(includeFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });
}

void CmakeDirectoryEntry::addSourceFiles(const std::vector<std::string>& files) {
  std::transform(files.begin(), files.end(), std::back_inserter(sourceFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });
}

}
