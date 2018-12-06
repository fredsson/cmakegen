#include "../fileutils.h"
#include "../ignorefile.h"
#include "../directory.h"

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

std::string makeRelative(const std::string& target) {
  const auto currentPath = filesystem::current_path().generic_string();
  return "." + target.substr(currentPath.size());
}

std::string makeRelative(const std::string& target, const std::string& rootPath) {
  return "." + target.substr(rootPath.size());
}

std::string directoryName(const std::string& path) {
  auto filePath = filesystem::path(path);
  return filePath.filename();
}

std::shared_ptr<Directory> getDirectories(const IgnoreFile& ignoreFile) {
  return walkDirectory(filesystem::current_path(), ignoreFile, nullptr);
}

}