#include "../projectbuilder.h"

#include "../file_utils/directory.h"
#include "../file_utils/fileutils.h"
#include "../file_utils/ignorefile.h"

#include "../cmake/cmakefile.h"
#include "../cmake/cmakefunctioncriteria.h"

namespace {
  bool filesChanged(const std::vector<std::string>& newFiles, const std::vector<cmake::CmakeFunctionArgument>& currentArguments) {
    for (const auto& argument : currentArguments) {
      if (argument.value_ == "INCLUDE_FILES" || argument.value_ == "SRC_FILES") {
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
    const auto cmakeFile = cmake::CmakeFile::parse(cmakeDirectory->path(), cmakeDirectory->path() + "/CMakeLists.txt");
    auto files = file_utils::getFilesForProject(cmakeDirectory);

    if (files.empty()) {
      continue;
    }

    const auto* includeFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::IncludeFiles)
    );
    if (!files.includeFiles.empty()) {
      if (!includeFileFunction) {
        cmakeFile->replaceIncludeFiles(files.includeFiles);
      } else {
        const auto includeFilesChanged =
          (files.includeFiles.size() != includeFileFunction->arguments().size() - 1) ||
          filesChanged(files.includeFiles, includeFileFunction->arguments());
        if (includeFilesChanged) {
          cmakeFile->replaceIncludeFiles(files.includeFiles);
        }
      }
    } else if(includeFileFunction) {
      cmakeFile->removeIncludeFiles();
    }

    const auto* sourceFileFunction = cmakeFile->getFunction(
      cmake::CmakeSetFileFunctionCriteria(cmake::CmakeSetFileFunctionCriteria::SourceFiles)
    );
    if (!files.sourceFiles.empty()) {
      if(!sourceFileFunction) {
        cmakeFile->replaceSourceFiles(files.sourceFiles);
      } else {
        const auto differentSize = files.sourceFiles.size() != sourceFileFunction->arguments().size() - 1;
        const auto sourceFilesChanged = differentSize || filesChanged(files.sourceFiles, sourceFileFunction->arguments());
        if (sourceFilesChanged) {
          cmakeFile->replaceSourceFiles(files.sourceFiles);
        }
      }
    }
    // TODO: what if no cpp files (is this even legal)

    cmakeFile->write();

    // auto* sourceFunction = cmakeFile->sourceFilesFunction();





    /*const auto sourceFilesChanged =
      (files.sourceFiles.size() != includeFunction->arguments().size() - 1) ||
      filesChanged(files.sourceFiles, sourceFunction->arguments());*/

    /*if (includeFilesChanged) {
      cmakeFile->replaceIncludeFiles(files.includeFiles);
    }*/

    /*if (sourceFilesChanged) {
      files.replaceSourceFunctionArguments(cmakeDirectory, sourceFunction);
    }*/
  }
}

void ProjectBuilder::build() {
}