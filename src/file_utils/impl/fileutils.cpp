#include "../fileutils.h"
#include "../ignorefile.h"
#include "../directory.h"

#include "../../cmake/cmakefunction.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <unordered_map>

#include <iostream>

namespace filesystem = std::filesystem;

namespace file_utils {
namespace {
const std::vector<std::string> HEADER_EXTENSIONS = {".h", ".hpp", ".hh"};
const std::vector<std::string> SOURCE_EXTENSIONS = {".c", ".cpp", ".c++"};

bool hasExtension(const std::filesystem::path& path, const std::vector<std::string>& extensions) {
  return std::any_of(extensions.begin(), extensions.end(), [&path](const std::string& extension){
    return path.extension().compare(extension) == 0;
  });
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
    const auto relativePath = file_utils::makeRelativeWithCmakeSeparator(file, directory->path());
    return cmake::CmakeFunctionArgument{relativePath, true};
  });

  return cmake::CmakeFunction::create("set", arguments);
}

std::shared_ptr<cmake::CmakeFunction> DirectoryFiles::createSourceFilesFunction(const file_utils::Directory* directory) {
  std::vector<cmake::CmakeFunctionArgument> arguments = {{"SRC_FILES"}};
  std::transform(sourceFiles.begin(), sourceFiles.end(), std::back_inserter(arguments), [&directory](const std::string& file) {
    const auto relativePath = file_utils::makeRelativeWithCmakeSeparator(file, directory->path());
    return cmake::CmakeFunctionArgument{relativePath, true};
  });

  return cmake::CmakeFunction::create("set", arguments);
}

std::string makeRelative(const std::string& target) {
  return makeRelative(target, filesystem::current_path().generic_string());
}

std::string makeRelative(const std::string& target, const std::string& rootPath) {
  if (target.size() < rootPath.size()) {
    return "." + target;
  }
  return "." + target.substr(rootPath.size());
}

std::string makeRelativeWithCmakeSeparator(std::string_view target, std::string_view root) {
  std::string result = makeRelative(target.data(), root.data());
  std::replace(result.begin(), result.end(), '\\', '/');
  return result;
}

std::string directoryName(const std::string& path) {
  auto filePath = filesystem::path(path);
  return filePath.filename().generic_string();
}

void createDir(const std::string& name) {
  filesystem::create_directory(filesystem::current_path().append(name));
}

std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile) {
  filesystem::recursive_directory_iterator directoryItr{filesystem::current_path()};

  auto root = std::make_shared<Directory>(
    filesystem::current_path().string(),
    nullptr
  );

  std::unordered_map<std::string, std::shared_ptr<Directory>> directoryByPath;
  directoryByPath.insert({
    filesystem::current_path().string(),
    root
  });
  for (const auto& entry : directoryItr) {
    const auto p = entry.path().string();
    if (ignoreFile.contains(p)) {
      continue;
    }

    const auto parentPath = entry.path().parent_path().string();
    const auto parentItr = directoryByPath.find(parentPath);
    if (parentItr == directoryByPath.end()) {
      std::cout << "could not find directory " << parentPath  << "!\n";
      continue;
    }

    if (entry.is_directory()) {
      auto dir = std::make_shared<Directory>(p, parentItr->second);
      directoryByPath.insert({
        p,
        dir
      });
      parentItr->second->addChild(dir);
    }
    if (entry.is_regular_file()) {
      // TODO: this needs to be relative to closest cmake file
      const auto relativePath = std::filesystem::relative(entry.path(), root->path());

      if (hasExtension(entry.path(), HEADER_EXTENSIONS)) {
        parentItr->second->addIncludeFile(p);
      } else if (hasExtension(entry.path(), SOURCE_EXTENSIONS)) {
        parentItr->second->addSourceFile(p);
      } else if (p.find("CMakeLists.txt") != std::string::npos) {
        parentItr->second->addCmakeFile();
      }
    }
  }

  return root;
}

 DirectoryFiles getFilesForProject(const Directory* directory) {

  DirectoryFiles files;

  directory->forEachIf(
    [&files](const file_utils::Directory& dir) {
      std::copy(dir.includeFiles().begin(), dir.includeFiles().end(), std::back_inserter(files.includeFiles));
      std::copy(dir.sourceFiles().begin(), dir.sourceFiles().end(), std::back_inserter(files.sourceFiles));
    },
    [&directory](const file_utils::Directory& dir) {
      return dir.path() == directory->path() || !dir.hasCmakeFile();
    }
  );

  DirectoryFiles relativeFiles;
  std::transform(
    files.includeFiles.begin(),
    files.includeFiles.end(),
    std::back_inserter(relativeFiles.includeFiles),
    [&directory](const auto& file){
      return makeRelativeWithCmakeSeparator(file, directory->path());
    }
  );
  std::transform(
    files.sourceFiles.begin(),
    files.sourceFiles.end(),
    std::back_inserter(relativeFiles.sourceFiles),
    [&directory](const auto& file){
      return makeRelativeWithCmakeSeparator(file, directory->path());
    }
  );

  std::sort(relativeFiles.includeFiles.begin(), relativeFiles.includeFiles.end());
  std::sort(relativeFiles.sourceFiles.begin(), relativeFiles.sourceFiles.end());

  return relativeFiles;
}

}