#include "../projectbuilder.h"

#include "../file_utils/directory.h"
#include "../file_utils/fileutils.h"
#include "../file_utils/ignorefile.h"

#include "../cmake/cmakefile.h"
#include "../cmake/cmakefunctioncriteria.h"
#include "../cmake/impl/constants.h"

#include <stdlib.h>

namespace {
  bool isSetArgument(const std::string& argument) {
    return argument == cmake::constants::SetIncludeFilesArgumentName || argument == cmake::constants::SetSourceFilesArgumentName;
  }
  bool filesChanged(const std::vector<std::string>& newFiles, const std::vector<cmake::CmakeFunctionArgument>& currentArguments) {
    for (const auto& argument : currentArguments) {
      if (isSetArgument(argument.value_)) {
        continue;
      }
      for (const auto& file : newFiles) {
        if (file == argument.value_) {
          return true;
        }
      }
    }

    return false;
  }
}

ProjectBuilder::ProjectBuilder(const file_utils::IgnoreFile& ignoreFile)
  : ignoreFile_(ignoreFile) {
}

void ProjectBuilder::run() {
  update();

  build();
}

void ProjectBuilder::update() {
  auto directoryRoot = file_utils::getDirectories(ignoreFile_);

  const auto cmakeDirectories = directoryRoot->filter([](const file_utils::Directory& directory){
    return directory.hasCmakeFile();
  });

  for (const auto* cmakeDirectory : cmakeDirectories) {
    const auto cmakeFile = cmake::CmakeFile::parse(cmakeDirectory->path(), cmakeDirectory->path() + "/" + cmake::constants::FileName);
    auto files = file_utils::getFilesForProject(cmakeDirectory);

    if (files.empty()) {
      continue;
    }

    const auto* includeFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::IncludeFiles)
    );
    if (!files.includeFiles.empty()) {
      replaceSetFunction([&cmakeFile](const std::vector<std::string>& files){
        cmakeFile->replaceIncludeFiles(files);
      }, includeFileFunction, files.includeFiles);
    } else if (includeFileFunction) {
      cmakeFile->removeIncludeFiles();
    }

    const auto* sourceFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::SourceFiles)
    );
    if (!files.sourceFiles.empty()) {
      replaceSetFunction([&cmakeFile](const std::vector<std::string>& files){
        cmakeFile->replaceSourceFiles(files);
      }, sourceFileFunction, files.sourceFiles);
    }

    cmakeFile->write();
  }
}

void ProjectBuilder::replaceSetFunction(
  const std::function<void(const std::vector<std::string>&)> replaceFileFunction,
  const cmake::CmakeFunction* function,
  const std::vector<std::string>& files
) {
  if (!function) {
    replaceFileFunction(files);
    return;
  }

  const auto& arguments = function->arguments();
  const bool differentSize = files.size() != arguments.size() - 1;
  if (differentSize) {
    replaceFileFunction(files);
    return;
  }

  const auto differentFiles = filesChanged(files, arguments);
  if (differentFiles) {
    replaceFileFunction(files);
  }
}

void ProjectBuilder::build() {
  file_utils::createDir("_build");

  system("cd _build && cmake ../ && make");
}