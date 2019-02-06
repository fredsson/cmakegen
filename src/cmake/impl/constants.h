#ifndef CMAKE_CONSTANTS_H
#define CMAKE_CONSTANTS_H
#include <string>

namespace cmake {
namespace constants {

const std::string SetIncludeFilesArgumentName = "INCLUDE_FILES";
const std::string SetIncludeFilesOutputArgument = "${INCLUDE_FILES}";
const std::string SetSourceFilesArgumentName = "SRC_FILES";
const std::string FileName = "CMakeLists.txt";

}
}

#endif