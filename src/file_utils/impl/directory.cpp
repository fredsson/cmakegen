#include "../directory.h"
#include <queue>

namespace file_utils {

Directory::Directory(const std::string& path, std::shared_ptr<Directory> parent)
  : path_(path), hasCmakeFile_(false), parent_(parent)  {
}

const std::string& Directory::path() const {
  return path_;
}

bool Directory::hasCmakeFile() const {
  return hasCmakeFile_;
}

void Directory::addChild(const std::shared_ptr<Directory>& child) {
  children_.push_back(child);
}

void Directory::addCmakeFile() {
  hasCmakeFile_ = true;
}

void Directory::addIncludeFile(const std::string& file) {
  includeFiles_.push_back(file);
}

void Directory::addSourceFile(const std::string& file) {
  sourceFiles_.push_back(file);
}

void Directory::forEach(std::function<void(Directory& directory)> callback) {
  std::queue<Directory*> childrenToCheck = {};
  childrenToCheck.push(this);

  while(!childrenToCheck.empty()) {
    auto& directory = childrenToCheck.front();
    childrenToCheck.pop();

    callback(*directory);

    for(auto& child : directory->children_) {
      childrenToCheck.push(child.get());
    }
  }
}

std::vector<Directory*> Directory::filter(std::function<bool(const Directory& directory)> predicate) {
  std::vector<Directory*> result = {};
  std::queue<Directory*> childrenToCheck = {};
  childrenToCheck.push(this);

  while (!childrenToCheck.empty()) {
    auto& directory = childrenToCheck.front();
    childrenToCheck.pop();
    if(predicate(*directory)) {
      result.push_back(directory);
    }

    for(auto& child : directory->children_) {
      childrenToCheck.push(child.get());
    }
  }

  return result;
}

}