#include "../fileutils.h"
#include "../ignorefile.h"
#include "../directory.h"

#include "../../cmake/cmakefile.h"

#include <algorithm>
#include <experimental/filesystem>
#include <functional>
#include <queue>

namespace filesystem = std::experimental::filesystem;

namespace file_utils {
namespace {
const std::vector<std::string> HEADER_EXTENSIONS = {".h", ".hpp", ".hh"};
const std::vector<std::string> SOURCE_EXTENSIONS = {".c", ".cpp", ".c++"};

bool hasExtension(const std::string& path, const std::vector<std::string>& extensions) {
  return std::any_of(extensions.begin(), extensions.end(), [&path](const std::string& extension){
    const auto position = path.find(extension);
    return position == path.size() - extension.size();
  });
}

std::shared_ptr<Directory> walkDirectory(const filesystem::path& rootPath, const IgnoreFile& ignoreFile, std::shared_ptr<Directory> parent) {

  std::shared_ptr<Directory> currentDirectory = std::make_shared<Directory>(rootPath.generic_string(), parent);

  for (const auto& entry : filesystem::directory_iterator(rootPath)) {
    const auto& path = entry.path().generic_string();
    if (ignoreFile.contains(path)) {
      continue;
    }

    if (entry.status().type() == filesystem::file_type::directory) {
      currentDirectory->addChild(walkDirectory(entry.path(), ignoreFile, currentDirectory));
      continue;
    }

    if (hasExtension(path, HEADER_EXTENSIONS)) {
      currentDirectory->addIncludeFile(path);
    } else if (hasExtension(path, SOURCE_EXTENSIONS)) {
      currentDirectory->addSourceFile(path);
    } else if (path.find("CMakeLists.txt") != std::string::npos) {
      currentDirectory->addCmakeFile();
    }
  }

  return currentDirectory;
}

}

bool DirectoryFiles::empty() const {
  return includeFiles.empty() && sourceFiles.empty();
}

std::vector<cmake::CmakeFunctionArgument> DirectoryFiles::availableFileTypeArguments(const std::string& projectName) const {
  std::vector<cmake::CmakeFunctionArgument> arguments = {projectName};
  if (!includeFiles.empty()) {
    arguments.push_back({"${INCLUDE_FILES}"});
  }

  if (!sourceFiles.empty()) {
    arguments.push_back({"${SRC_FILES}"});
  }

  return arguments;
}

std::shared_ptr<cmake::CmakeFunction> DirectoryFiles::createIncludeFilesFunction(const file_utils::Directory* directory) {
  std::vector<cmake::CmakeFunctionArgument> arguments = {{"INCLUDE_FILES"}};
  std::transform(includeFiles.begin(), includeFiles.end(), std::back_inserter(arguments), [&directory](const std::string& file) {
    return cmake::CmakeFunctionArgument{file_utils::makeRelative(file, directory->path()), true};
  });

  return cmake::CmakeFunction::create("set", arguments);
}

std::shared_ptr<cmake::CmakeFunction> DirectoryFiles::createSourceFilesFunction(const file_utils::Directory* directory) {
  std::vector<cmake::CmakeFunctionArgument> arguments = {{"SRC_FILES"}};
  std::transform(sourceFiles.begin(), sourceFiles.end(), std::back_inserter(arguments), [&directory](const std::string& file) {
    return cmake::CmakeFunctionArgument{file_utils::makeRelative(file, directory->path()), true};
  });

  return cmake::CmakeFunction::create("set", arguments);
}

std::string makeRelative(const std::string& target) {
  return makeRelative(target, filesystem::current_path().generic_string());
}

std::string makeRelative(const std::string& target, const std::string& rootPath) {
  return "." + target.substr(rootPath.size());
}

std::string directoryName(const std::string& path) {
  auto filePath = filesystem::path(path);
  return filePath.filename();
}

void createDir(const std::string& name) {
  filesystem::create_directory(filesystem::current_path().append(name));
}

std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile) {
  return walkDirectory(filesystem::current_path(), ignoreFile, nullptr);
}

 DirectoryFiles getFilesForProject(const Directory* directory) {

  DirectoryFiles files;

  std::copy(directory->includeFiles().begin(), directory->includeFiles().end(), std::back_inserter(files.includeFiles));
  std::copy(directory->sourceFiles().begin(), directory->sourceFiles().end(), std::back_inserter(files.sourceFiles));

  directory->forEachIf(
    [&files](const file_utils::Directory& dir) {
      std::copy(dir.includeFiles().begin(), dir.includeFiles().end(), std::back_inserter(files.includeFiles));
      std::copy(dir.sourceFiles().begin(), dir.sourceFiles().end(), std::back_inserter(files.sourceFiles));
    },
    [&directory](const file_utils::Directory& dir) {
      return dir.path() == directory->path() || !dir.hasCmakeFile();
    }
  );

  return files;
}

}