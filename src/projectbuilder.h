#ifndef PROJECT_BUILDER_H
#define PROJECT_BUILDER_H

namespace file_utils {
class IgnoreFile;
}

class ProjectBuilder {
public:
  ProjectBuilder(const file_utils::IgnoreFile& ignoreFile);
  void run();
private:
  void update();
  void build();

  const file_utils::IgnoreFile& ignoreFile_;
};

#endif