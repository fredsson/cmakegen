#include "../cmakefunctioncriteria.h"

#include "../cmakefile.h"
#include "constants.h"

namespace cmake {

namespace {
  const std::string SetSourceFilesArgumentName = "SRC_FILES";
  const std::string SetFilesFunctionName = "set";
  const std::string ProjectFunctionName = "project";
  const std::string OutputExecutableName = "add_executable";
  const std::string OutputLibraryName = "add_library";
}

ICmakeFunctionCriteria::~ICmakeFunctionCriteria() = default;

CmakeSetFileFunctionCriteria::CmakeSetFileFunctionCriteria(FileFunctionType type)
  : type_(type) {
}

bool CmakeSetFileFunctionCriteria::matches(const CmakeFunction& function) const {
  if (function.name() != SetFilesFunctionName) {
    return false;
  }

  if (function.arguments().empty()) {
    return false;
  }

  switch(type_) {
    case IncludeFiles:
      return function.arguments()[0].value_ == constants::SetIncludeFilesArgumentName;
    case SourceFiles:
      return function.arguments()[0].value_ == SetSourceFilesArgumentName;
    default:
      return false;
  }
}

bool CmakeProjectFunctionCriteria:: matches(const CmakeFunction& function) const {
  return function.name() == ProjectFunctionName;
}

CmakeOutputFunctionCriteria::CmakeOutputFunctionCriteria(const std::string& projectName)
  : projectName_(projectName) {
}

bool CmakeOutputFunctionCriteria::matches(const CmakeFunction& function) const {
  if (function.name() != OutputExecutableName && function.name() != OutputLibraryName) {
    return false;
  }

  const auto& arguments = function.arguments();
  if (arguments.empty()) {
    return false;
  }

  return arguments[0].value_ == projectName_;
}

}