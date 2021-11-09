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

class IoHandler;
class ProjectBuilder {
public:
  ProjectBuilder(const std::string& buildSystem, const file_utils::IgnoreFile& ignoreFile, IoHandler& ioHandler);
  void run();
private:
  void update();
  void replaceSetFunction(
    const std::function<void(const std::vector<std::string>&)> replaceFileFunction,
    const cmake::CmakeFunction* function,
    const std::vector<std::string>& files
  );
  void build();

  std::string buildSystem_;
  const file_utils::IgnoreFile& ignoreFile_;
  IoHandler& ioHandler_;
};

#endif