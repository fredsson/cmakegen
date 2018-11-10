#include "../cmakefile.h"
#include <algorithm>

namespace cmake {

std::shared_ptr<CmakeFunction> CmakeFunction::create(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(name, arguments));
}

CmakeFunction::CmakeFunction(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments)
  : name_(name), arguments_(arguments) {

}

const std::string& CmakeFunction::name() const {
  return name_;
}

const CmakeFunctionArgument* CmakeFunction::argumentByPosition(unsigned int position) const {
  if (position >= arguments_.size()) {
    return nullptr;
  }

  return &arguments_[position];
}

CmakeFile::CmakeFile(const std::string& path)
  : path_(path), children_({}), includeFiles_({}), sourceFiles_({}) {
}

const std::string& CmakeFile::path() const {
  return path_;
}

void CmakeFile::addChild(const std::shared_ptr<CmakeFile>& child) {
  children_.push_back(child);
  /*const auto* childProjectNameFunction = child->functionByName("project");
  if (childProjectNameFunction) {
    const auto* childName = childProjectNameFunction->argumentByPosition(0);
    if (childName) {
      functions_.push_back(std::make_shared<CmakeFunction>(
        "add_subdirectory",
        CmakeFunctionArgument{childName->value, childName->quoted}
      ));
    }
  }*/
}

void CmakeFile::addFiles(const std::vector<std::string>& includeFiles, const std::vector<std::string>& sourceFiles) {
  std::transform(includeFiles.begin(), includeFiles.end(), std::back_inserter(includeFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });

  std::transform(sourceFiles.begin(), sourceFiles.end(), std::back_inserter(sourceFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });
}

}