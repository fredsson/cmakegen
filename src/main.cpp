#include <iostream>
#include "cmdoptionparser.h"

void generateCmakeFiles(const std::string& cmakeVersion, const std::string& cppVersion) {
  std::cout << "generating with cmake: " << cmakeVersion << " and cpp: " << cppVersion << "\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "no arguments provided\n";
    return 0;
  }

  CmdOptionParser optionParser(argc, argv);

  if (optionParser.hasAnyOption({ "-g", "--gen" })) {
    const auto* cmdCmakeVersion = optionParser.getOption("--cmake");
    const auto* cmdCppVersion = optionParser.getOption("--cpp");

    generateCmakeFiles(
      cmdCmakeVersion != nullptr ? cmdCmakeVersion : "3.10.0",
      cmdCppVersion != nullptr ? cmdCppVersion : "cxx_std_11"
    );
  } else if (optionParser.hasAnyOption({ "-b", "--build" })) {
    std::cout << "building files!\n";
  } else {
  }
  return 0;
}
