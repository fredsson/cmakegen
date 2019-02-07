#ifndef PROJECT_BUILDER_H
#define PROJECT_BUILDER_H
#include <functional>
#include <vector>
#include <string>

namespace file_utils {
class IgnoreFile;
}

namespace cmake {
class CmakeFunction;
}

class ProjectBuilder {
public:
  ProjectBuilder(const file_utils::IgnoreFile& ignoreFile);
  void run();
private:
  void update();
  void replaceSetFunction(
    const std::function<void(const std::vector<std::string>&)> replaceFileFunction,
    const cmake::CmakeFunction* function,
    const std::vector<std::string>& files
  );
  void build();

  const file_utils::IgnoreFile& ignoreFile_;
};

#endif