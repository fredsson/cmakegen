#include "../projectbuilder.h"

#include "../file_utils/directory.h"
#include "../file_utils/fileutils.h"
#include "../file_utils/ignorefile.h"

#include "../cmake/cmakefile.h"
#include "../cmake/cmakefunctioncriteria.h"
#include "../cmake/impl/constants.h"

#include <algorithm>
#include <stdlib.h>

namespace {
  bool isSetArgument(const cmake::CmakeFunctionArgument& argument) {
    return argument.value_ == cmake::constants::SetIncludeFilesArgumentName || argument.value_ == cmake::constants::SetSourceFilesArgumentName;
  }

  bool fileMatchesArgument(const cmake::CmakeFunctionArgument& argument, const std::string& filePath) {
    const auto relativePath = file_utils::makeRelative(filePath);
    return argument.value_ == relativePath;
  }

  bool filesChanged(const std::vector<std::string>& newFiles, const std::vector<cmake::CmakeFunctionArgument>& currentArguments) {
    return std::any_of(currentArguments.begin(), currentArguments.end(), [&newFiles](const cmake::CmakeFunctionArgument& argument) {
      if (isSetArgument(argument.value_)) {
        return true;
      }

      return std::any_of(newFiles.begin(), newFiles.end(), [&argument](const std::string& newFile) {
        return !fileMatchesArgument(argument, newFile);
      });
    });
  }
}

ProjectBuilder::ProjectBuilder(const std::string& buildSystem, const file_utils::IgnoreFile& ignoreFile, IoHandler& ioHandler)
  : buildSystem_(buildSystem), ignoreFile_(ignoreFile), ioHandler_(ioHandler) {
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
    const auto cmakeFile = cmake::CmakeFile::parse(
      cmakeDirectory->path(),
      cmakeDirectory->path() + "/" + cmake::constants::FileName,
      ioHandler_
    );
    auto projectFiles = file_utils::getFilesForProject(cmakeDirectory);

    if (projectFiles.empty()) {
      continue;
    }

    const auto* includeFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::IncludeFiles)
    );
    if (!projectFiles.includeFiles.empty()) {
      replaceSetFunction([&cmakeFile](const std::vector<std::string>& files) {
        cmakeFile->replaceIncludeFiles(files);
      }, includeFileFunction, projectFiles.includeFiles);
    } else if (includeFileFunction) {
      cmakeFile->removeIncludeFiles();
    }

    const auto* sourceFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::SourceFiles)
    );
    if (!projectFiles.sourceFiles.empty()) {
      replaceSetFunction([&cmakeFile](const std::vector<std::string>& files) {
        cmakeFile->replaceSourceFiles(files);
      }, sourceFileFunction, projectFiles.sourceFiles);
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

  // TODO: Handle more build systems here
  int result;
  if (buildSystem_ == "ninja") {
    result = system("cd _build && cmake -GNinja ../ && ninja");
  } else {
    result = system("cd _build && cmake ../ && make");
  }

  if (result != 0) {
    // TODO: log something here
  }
}
