#include "../fileutils.h"
#include "cmakedirectoryentry.h"
#include "../ignorefile.h"

#include <algorithm>
#include <experimental/filesystem>
#include <functional>
#include <queue>

namespace filesystem = std::experimental::filesystem;

namespace file_utils {
namespace {
const std::vector<std::string> HEADER_EXTENSIONS = {".h", ".hpp", ".hh"};
const std::vector<std::string> SOURCE_EXTENSIONS = {".c", ".cpp", ".c++"};

struct Directory {
  std::string path_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
  bool hasCmakeFile_;
  std::shared_ptr<Directory> parent_;
  std::vector<std::shared_ptr<Directory>> children_;
};


std::shared_ptr<Directory> walkDirectory(const filesystem::path& path, const IgnoreFile& ignoreFile, std::shared_ptr<Directory> parent) {

  std::shared_ptr<Directory> currentDirectory = std::make_shared<Directory>();
  currentDirectory->parent_ = parent;
  currentDirectory->path_ = path.generic_string();

  for (const auto& entry : filesystem::directory_iterator(path)) {
    if (entry.status().type() == filesystem::file_type::directory) {
      currentDirectory->children_.push_back(walkDirectory(entry.path(), ignoreFile, currentDirectory));
      continue;
    }

    const auto path = entry.path().generic_string();
    if (hasExtension(path, HEADER_EXTENSIONS)) {
      currentDirectory->includeFiles_.push_back(path);
    } else if (hasExtension(path, SOURCE_EXTENSIONS)) {
      currentDirectory->sourceFiles_.push_back(path);
    } else if (path.find("CMakeLists.txt") != std::string::npos) {
      currentDirectory->hasCmakeFile_ = true;
    }
  }

  return currentDirectory;
}

void walkDirectory(
  const filesystem::path& path,
  const IgnoreFile& ignoreFile,
  std::function<void(const filesystem::directory_entry&, const std::shared_ptr<CmakeDirectoryEntry>&)> entryCallback,
  std::function<void(const std::shared_ptr<CmakeDirectoryEntry>&)> directoryCallback
) {
  std::queue<std::shared_ptr<CmakeDirectoryEntry>> directoriesToSearch;
  directoriesToSearch.push(std::make_shared<CmakeDirectoryEntry>(path));
  while(!directoriesToSearch.empty()) {
    auto dir = directoriesToSearch.front();
    directoriesToSearch.pop();
    for (const auto& entry : filesystem::directory_iterator(filesystem::path(dir->path()))) {
      if (ignoreFile.contains(entry.path().generic_string())) {
        continue;
      }

      if (entry.status().type() == filesystem::file_type::directory) {
        directoriesToSearch.push(std::make_shared<CmakeDirectoryEntry>(entry.path().generic_string(), dir));
      }

      entryCallback(entry, dir);
    }
    directoryCallback(dir);
  }
}

bool hasExtension(const std::string& path, const std::vector<std::string>& extensions) {
  return std::any_of(extensions.begin(), extensions.end(), [&path](const std::string& extension){
    const auto position = path.find(extension);
    return position == path.size() - extension.size();
  });
}

CmakeDirectoryEntry* getParentWithCmakeFile(CmakeDirectoryEntry& dir) {
  auto* parent = dir.parent();
  while (parent && !parent->hasCmakeFilePath()) {
    parent = parent->parent();
  }
  return parent;
}

}

std::string makeRelative(const std::string& target) {
  const auto currentPath = filesystem::current_path().generic_string();
  return "." + target.substr(currentPath.size());
}

std::vector<CmakeDirectory> getCmakeDirectories(const IgnoreFile& ignoreFile) {
  std::vector<std::string> includeFiles = {};
  std::vector<std::string> sourceFiles = {};
  std::vector<std::shared_ptr<CmakeDirectoryEntry>> relevantEntries = {};
  walkDirectory(
    filesystem::current_path(),
    ignoreFile,
    [&includeFiles, &sourceFiles](const filesystem::directory_entry& entry, const std::shared_ptr<CmakeDirectoryEntry>& dir) {
      const auto path = entry.path().generic_string();
      if (hasExtension(path, HEADER_EXTENSIONS)) {
        includeFiles.push_back(path);
      } else if (hasExtension(path, SOURCE_EXTENSIONS)) {
        sourceFiles.push_back(path);
      } else if (path.find("CMakeLists.txt") != std::string::npos) {
        dir->addCmakeFilePath(path);
      }
    },
    [&includeFiles, &sourceFiles, &relevantEntries](const std::shared_ptr<CmakeDirectoryEntry>& dir) {
      auto* cmakeTarget = dir->hasCmakeFilePath() ? dir.get() : getParentWithCmakeFile(*dir);
      cmakeTarget->addIncludeFiles(includeFiles);
      cmakeTarget->addSourceFiles(sourceFiles);
      includeFiles = {};
      sourceFiles = {};
      if (dir->hasCmakeFilePath()) {
        relevantEntries.push_back(dir);
      }
    }
  );
  std::vector<CmakeDirectory> result = {};
  std::transform(relevantEntries.begin(), relevantEntries.end(), std::back_inserter(result), [](const auto& entry){
    return CmakeDirectory{
      entry->path(),
      entry->cmakeFilePath(),
      entry->includeFiles(),
      entry->sourceFiles()
    };
  });
  return result;
}

std::vector<CmakeDirectory> getPossibleCmakeDirectories(const IgnoreFile& ignoreFile) {
  std::vector<CmakeDirectory> result = {};

  walkDirectory(
    filesystem::current_path(),
    ignoreFile,
    [](const filesystem::directory_entry& entry, const std::shared_ptr<CmakeDirectoryEntry>& dir) {
        const auto path = entry.path().generic_string();
        if (path.find("CMakeLists.txt") != std::string::npos) {
          dir->addCmakeFilePath(path);
        }
      },
      [&result](const std::shared_ptr<CmakeDirectoryEntry>& dir) {
        result.push_back({
          dir->path(),
          dir->cmakeFilePath(),
          {},
          {}
        });
      }
  );

  return result;
}

}