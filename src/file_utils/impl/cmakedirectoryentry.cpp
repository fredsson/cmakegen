#include "cmakedirectoryentry.h"

namespace file_utils {

CmakeDirectoryEntry::CmakeDirectoryEntry(const std::string& path)
  : path_(path), cmakeFilePath_(nullptr), includeFiles_({}), sourceFiles_({}) {
}

const std::string& CmakeDirectoryEntry::path() const {
  return path_;
}

std::shared_ptr<std::string> CmakeDirectoryEntry::cmakeFilePath() const {
  return cmakeFilePath_;
}

void CmakeDirectoryEntry::addCmakeFilePath(const std::string& path) {
  cmakeFilePath_ = std::make_shared<std::string>(path);
}



}