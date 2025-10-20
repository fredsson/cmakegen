#include <iostream>
#include "cmdoptionparser.h"
#include "cmakegenerator.h"
#include "file_utils/ignorefile.h"
#include "file_utils/fileutils.h"
#include "file_utils/directory.h"
#include "iohandler.h"
#include "cmake/cmakefile.h"
#include "projectbuilder.h"

class StdIoHandler : public IoHandler {
public:
  void write(const std::string& text) override {
    std::cout << text << "\n";
  }

  std::string input() override {
    std::string input;
    std::getline(std::cin, input);
    return input;
  }
};

void generateCmakeFiles(const std::string& cmakeVersion, const std::string& cppVersion, const file_utils::IgnoreFile& ignoreFile) {
  auto ioHandler = StdIoHandler();
  CmakeGenerator generator(ioHandler, ignoreFile, cmakeVersion, cppVersion);
  generator.run();
}

void updateCmakeFiles(const std::string& buildSystem, const bool debug, const file_utils::IgnoreFile& ignoreFile) {
  auto ioHandler = StdIoHandler();
  ProjectBuilder builder(buildSystem, debug, ignoreFile, ioHandler);
  builder.run();
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "no arguments provided\n";
    return 0;
  }

  const auto ignoreFile = file_utils::IgnoreFile::load(".cmakeignore");
  CmdOptionParser optionParser(argc, argv);

  if (optionParser.hasAnyOption({ "-g", "--gen" })) {
    const auto* cmdCmakeVersion = optionParser.getOption("--cmake");
    const auto* cmdCppVersion = optionParser.getOption("--cpp");

    generateCmakeFiles(
      cmdCmakeVersion != nullptr ? cmdCmakeVersion : "3.10.0",
      cmdCppVersion != nullptr ? cmdCppVersion : "cxx_std_11",
      ignoreFile
    );
  } else if (optionParser.hasAnyOption({ "-b", "--build" })) {
    const auto* cmdBuildSystem = optionParser.getOption("--system");
    const auto debug = optionParser.hasOption("--debug");
    updateCmakeFiles(
      cmdBuildSystem != nullptr ? cmdBuildSystem : "make",
      debug,
      ignoreFile
    );
  } else if (optionParser.hasOption("--list-files")) {
    auto directoryRoot = file_utils::getDirectories(ignoreFile);
    directoryRoot->forEach([](file_utils::Directory& dir){
      std::cout << dir.path() << (dir.hasCmakeFile() ? "(cmake)" : "")  << ":\n";
      for (const auto& include: dir.includeFiles()) {
        std::cout << "I: " << include << "\n";
      }
      for(const auto& src: dir.sourceFiles()) {
        std::cout << "S: " << src << "\n";
      }
    });
  } else {
  }
  return 0;
}
