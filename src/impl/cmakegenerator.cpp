#include "../cmakegenerator.h"
#include "../file_utils/fileutils.h"
#include "../file_utils/directory.h"
#include "../cmake/cmakefile.h"
#include "../iohandler.h"

#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
#include <queue>

namespace {

std::string lowercase(std::string subject) {
  std::transform(subject.begin(), subject.end(), subject.begin(), ::tolower);
  return subject;
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

template<class Container>
bool parseIntervalInput(const std::string& input, const int maxAllowedIndex, std::back_insert_iterator<Container> iter) {
  try {
    const auto delimIndex = input.find("-");
    const int firstIndex = std::stoi(input.substr(0, delimIndex)) - 1;
    const int lastIndex = std::stoi(input.substr(delimIndex + 1));
    const int start = std::min(firstIndex, lastIndex);
    const int end = std::max(firstIndex, lastIndex);

    if (start >= 0 && end <= maxAllowedIndex) {
      for (int i = start; i < end; i++) {
        *(iter++) = i;
      }
      return true;
    }
  } catch (const std::invalid_argument&) {
  }
  return false;
}

template<class Container>
bool parseCmakeFileList(const std::string& input, const int maxAllowedIndex, std::back_insert_iterator<Container> iter) {
  std::vector<std::string> parts = {};
  split(input, ' ', std::back_inserter(parts));

  std::set<int> results = {};
  for (const auto& part : parts) {
    try {
      const auto index = std::stoi(part) - 1;
      if (index < 0 || index >= maxAllowedIndex) {
        return false;
      }
      results.insert(index);
    } catch(const std::invalid_argument&) {
      return false;
    }
  }
  for (const auto& result : results) {
    *(iter++) = result;
  }
  return true;
}

std::string getOptionalInput(const std::string& input, const std::string& defaultValue) {
  return input.empty() ? defaultValue : input;
}

std::string getProjectType(IoHandler& ioHandler) {
  auto input = lowercase(ioHandler.input());
  while (input != "lib" && input != "exe") {
    input = input = lowercase(ioHandler.input());
  }

  return input;
}

std::vector<const file_utils::Directory*> getSubDirectoriesForProject(const file_utils::Directory* directory) {
  std::vector<const file_utils::Directory*> subDirectories = {};
  directory->forEach([&subDirectories, directory](const file_utils::Directory& dir) {
    if (dir.path() != directory->path() && dir.hasCmakeFile()) {
      subDirectories.push_back(&dir);
    }
  });

  return subDirectories;
}

}

CmakeGenerator::CmakeGenerator(
  IoHandler& iohandler,
  const file_utils::IgnoreFile& ignoreFile,
  const std::string& cmakeVersion,
  const std::string& cppVersion
): defaultCmakeVersion_(cmakeVersion), defaultCppVersion_(cppVersion), ioHandler_(iohandler),  ignoreFile_(ignoreFile) {
}

void CmakeGenerator::run() {
  ioHandler_.write("Welcome to cmakgen\n");
  ioHandler_.write("This tool will guide you through the process of configuring all the CMakeLists.txt files needed for your project\n");

  auto directoryRoot = file_utils::getDirectories(ignoreFile_);

  const auto cmakeDirectories = directoryRoot->filter([](const file_utils::Directory& directory){
    return directory.hasCmakeFile();
  });
  if (!cmakeDirectories.empty()) {
    ioHandler_.write("Found CMakeLists.txt files in the project, please use -b instead to update them.");
    return;
  }

  placeInitialCmakeFiles(directoryRoot);

  populateCmakeFiles(directoryRoot);

}

void CmakeGenerator::placeInitialCmakeFiles(const std::shared_ptr<file_utils::Directory>& directoryRoot) {
  std::stringstream ss;
  ss << "Found the following folders, please specify which should be considered projects (contain CMakeLists.txt):\n";

  std::vector<file_utils::Directory*> allowedDirectories = {};
  directoryRoot->forEach([&allowedDirectories, &ss](file_utils::Directory& directory){
    allowedDirectories.push_back(&directory);
    ss << allowedDirectories.size() << " " << file_utils::makeRelative(directory.path()) << "\n";
  });

  ss << "==> Folders to create CMakeLists.txt in (ex: (N)one, 1 2 3 or 1-3)";
  ioHandler_.write(ss.str());

  bool hasSelectedFiles = false;
  std::vector<int> indices = {};
  while(!hasSelectedFiles) {
    const auto input = lowercase(ioHandler_.input());
    if (input.find("n") != std::string::npos) {
      hasSelectedFiles = true;
      continue;
    }

    const auto delimIndex = input.find("-");
    if (delimIndex != std::string::npos) {
      hasSelectedFiles = parseIntervalInput(input, allowedDirectories.size(), std::back_inserter(indices));
      continue;
    }

    hasSelectedFiles = parseCmakeFileList(input, allowedDirectories.size(), std::back_inserter(indices));
  }

  for (const auto& index : indices) {
    allowedDirectories[index]->addCmakeFile();
    std::ofstream file(allowedDirectories[index]->path() + "/CMakeLists.txt");
    file.close();
  }
}

void CmakeGenerator::populateCmakeFiles(const std::shared_ptr<file_utils::Directory>& directoryRoot) {
  directoryRoot->hasCmakeFile();
  ioHandler_.write("CMake version? (" + defaultCmakeVersion_ + ")");
  const auto cmakeVersion = getOptionalInput(ioHandler_.input(), defaultCmakeVersion_);

  ioHandler_.write("C++ version? (" + defaultCppVersion_ + ")");
  const auto cppVersion = getOptionalInput(ioHandler_.input(), defaultCppVersion_);

  const auto cmakeDirectories = directoryRoot->filter([](const file_utils::Directory& directory){
    return directory.hasCmakeFile();
  });

  for (const auto* directory : cmakeDirectories) {
    populateCmakeFile(directory, cmakeVersion, cppVersion);
  }
}

void CmakeGenerator::populateCmakeFile(
  const file_utils::Directory* directory,
  const std::string& cmakeVersion,
  const std::string& cppVersion
) {
  auto cmakeFile = std::make_shared<cmake::CmakeFile>(directory->path());

  const auto projectName = file_utils::directoryName(directory->path());

  cmakeFile->addFunction(cmake::CmakeFunction::create("cmake_minimum_required", {
    {"VERSION"},
    {cmakeVersion}
  }));

  cmakeFile->addFunction(cmake::CmakeFunction::create("project", {
    {projectName},
    {"VERSION"},
    {"1.0.0"},
    {"LANGUAGES"},
    {"CXX"}
  }));

  const auto subDirectories = getSubDirectoriesForProject(directory);
  for (const auto* subDirectory : subDirectories) {
    cmakeFile->addFunction(cmake::CmakeFunction::create("add_subdirectory", {
      {file_utils::directoryName(subDirectory->path())}
    }));
  }

  auto files = file_utils::getFilesForProject(directory);
  const auto hasIncludeFiles = !files.includeFiles.empty();
  const auto hasSourceFiles = !files.sourceFiles.empty();
  if (hasIncludeFiles || hasSourceFiles) {
    if (hasIncludeFiles) {
      cmakeFile->addFunction(files.createIncludeFilesFunction(directory));
    }

    if (hasSourceFiles) {
      cmakeFile->addFunction(files.createSourceFilesFunction(directory));
    }

    ioHandler_.write("Found source files for " + projectName + " what should the project type be? (lib/exe)");
    const auto projectType = getProjectType(ioHandler_).find("lib") != std::string::npos ? "add_library" : "add_executable";
    cmakeFile->addFunction(cmake::CmakeFunction::create(projectType,
      files.availableFileTypeArguments(projectName)
    ));

    cmakeFile->addFunction(cmake::CmakeFunction::create("target_compile_features", {
      {projectName},
      {"PRIVATE"},
      {cppVersion}
    }));

    cmakeFile->addFunction(cmake::CmakeFunction::create("target_compile_options", {
      {projectName},
      {"PRIVATE"},
      {"-Wall"},
      {"-Wextra"},
      {"-Wshadow"},
      {"-Wnon-virtual-dtor"},
      {"-pedantic"},
      {"-Werror"},
    }));
  }

  cmakeFile->write();
}