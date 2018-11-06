#ifndef FILE_UTILS_DIRECTORY_H
#define FILE_UTILS_DIRECTORY_H
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace file_utils {

class Directory {
public:
  Directory(const std::string& path, std::shared_ptr<Directory> parent);

  const std::string& path() const;

  bool hasCmakeFile() const;

  void addChild(const std::shared_ptr<Directory>& child);
  void addCmakeFile();
  void addIncludeFile(const std::string& file);
  void addSourceFile(const std::string& file);
  void forEach(std::function<void(Directory& directory)> callback);
  std::vector<Directory*> filter(std::function<bool(const Directory& directory)> predicate);
private:
  std::string path_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
  bool hasCmakeFile_;
  std::shared_ptr<Directory> parent_;
  std::vector<std::shared_ptr<Directory>> children_;
};

}

#endif
