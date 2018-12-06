#include "../cmakefile.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cmake {

CmakeFunctionArgument::CmakeFunctionArgument(std::string value)
  : value_(value), quoted_(false) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, bool quoted)
  : value_(value), quoted_(quoted) {
}

std::shared_ptr<CmakeFunction> CmakeFunction::create(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(name, arguments));
}

CmakeFunction::CmakeFunction(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments)
  : name_(name), arguments_(arguments) {

}

const std::string& CmakeFunction::name() const {
  return name_;
}

const std::string CmakeFunction::argumentString() const {
  std::ostringstream ss;

  const bool includeOrSourceList = arguments_.size() > 1 &&
    (arguments_[0].value_ == "INCLUDE_FILES" || arguments_[0].value_ == "SRC_FILES");

  ss << "(";

  for (size_t i = 0; i < arguments_.size(); ++i) {
    const auto& arg = arguments_[i];
    const auto pre = (i > 0 ? (includeOrSourceList ? "\n  " : " ") : "");
    ss << pre << (arg.quoted_ ? "\"" : "") << arg.value_ << (arg.quoted_ ? "\"" : "");
  }

  ss << (includeOrSourceList ? "\n" : "") << ")";

  return ss.str();
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

void CmakeFile::addFunction(const std::shared_ptr<CmakeFunction>& func) {
  functions_.push_back(func);
}

void CmakeFile::write() {
  std::ofstream stream(path_ + "/CMakeLists.txt");
  if (stream.is_open()) {
    for (const auto& f : functions_) {
      const auto arguments = f->argumentString();
      stream << f->name() << arguments << "\n\n";
    }

    stream.close();
  }
}

}