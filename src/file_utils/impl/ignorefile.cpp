#include "../ignorefile.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace file_utils {
namespace {
  template<typename Out>
  void split(const std::string &s, char delim, Out result) {
      std::stringstream ss(s);
      std::string item;
      while (std::getline(ss, item, delim)) {
          *(result++) = item;
      }
  }

  std::vector<std::string> parsePatterns(std::ifstream& stream) {
    std::vector<std::string> patterns = {"_build"};
    for (std::string line; std::getline(stream, line);) {
      if (!line.empty()) {
        patterns.push_back(line);
      }
    }
    return patterns;
  }
}

IgnoreFile IgnoreFile::load(const std::string& fileName) {
  std::ifstream stream(fileName);
  if (!stream.is_open()) {
    return {
      std::vector<std::string>{"_build"}
    };
  }

  return {
    parsePatterns(stream)
  };
}

IgnoreFile::IgnoreFile(std::vector<std::string> patterns)
 : patterns_(patterns) {
}

bool IgnoreFile::contains(const std::string& path) const {
  return std::any_of(patterns_.begin(), patterns_.end(), [&path](const std::string& pattern) {
    std::vector<std::string> parts = {};
    split(path, '/', std::back_inserter(parts));

    return std::any_of(parts.begin(), parts.end(), [&pattern](const std::string& pathPart) {
      if (pattern[0] == '*') {
        const auto patternIndex = pathPart.find(pattern.substr(1));
        if (patternIndex == std::string::npos) {
          return false;
        }

        return (patternIndex + pattern.size() - 1) == pathPart.size();
      }

      return pathPart == pattern;
    });
  });
}

}
