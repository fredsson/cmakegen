#include <iostream>
#include "cmdoptionparser.h"
#include "cmakegenerator.h"
#include "file_utils/ignorefile.h"
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

void updateCmakeFiles(const file_utils::IgnoreFile& ignoreFile) {
  auto ioHandler = StdIoHandler();
  ProjectBuilder builder(ignoreFile, ioHandler);
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
    updateCmakeFiles(ignoreFile);
  } else {
  }
  return 0;
}
