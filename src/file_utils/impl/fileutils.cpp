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

void walkDirectory(
  const filesystem::path& path,
  const IgnoreFile& ignoreFile,
  std::function<void(const filesystem::directory_entry&, CmakeDirectoryEntry&)> entryCallback,
  std::function<void(CmakeDirectoryEntry&)> directoryCallback
) {
  std::queue<CmakeDirectoryEntry> directoriesToSearch;
  directoriesToSearch.push({path});
  while(!directoriesToSearch.empty()) {
    auto dir = directoriesToSearch.front();
    directoriesToSearch.pop();
    for (const auto& entry : filesystem::directory_iterator(filesystem::path(dir.path()))) {
      if (ignoreFile.contains(entry.path().generic_string())) {
        continue;
      }

      if (entry.status().type() == filesystem::file_type::directory) {
        directoriesToSearch.push({entry.path()});
      }

      entryCallback(entry, dir);
    }
    directoryCallback(dir);
  }
}

}

std::string makeRelative(const std::string& target) {
  const auto currentPath = filesystem::current_path().generic_string();
  return "." + target.substr(currentPath.size());
}

std::vector<CmakeDirectory> getPossibleCmakeDirectories(const IgnoreFile& ignoreFile) {
  std::vector<CmakeDirectory> result = {};

  walkDirectory(
    filesystem::current_path(),
    ignoreFile,
    [](const filesystem::directory_entry& entry, CmakeDirectoryEntry& dir) {
        const auto path = entry.path().generic_string();
        if (path.find("CMakeLists.txt") != std::string::npos) {
          dir.addCmakeFilePath(path);
        }
      },
      [&result](CmakeDirectoryEntry& dir) {
        result.push_back({
          dir.path(),
          dir.cmakeFilePath(),
          {},
          {}
        });
      }
  );

  return result;
}

}