#ifndef FILEUTILS_IGNOREFILE_H
#define FILEUTILS_IGNOREFILE_H
#include <string>
#include <vector>

namespace file_utils {

class IgnoreFile {
public:
  static IgnoreFile load(const std::string& fileName);
  IgnoreFile(std::vector<std::string> patterns);
  bool contains(const std::string& path) const;
private:
  std::vector<std::string> patterns_;
};

}

#endif