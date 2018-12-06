#ifndef CMAKE_CMAKEFILE_H
#define CMAKE_CMAKEFILE_H
#include <memory>
#include <string>
#include <vector>

namespace cmake {

struct CmakeFunctionArgument {
  CmakeFunctionArgument(std::string value);
  CmakeFunctionArgument(std::string value, bool quoted);
  std::string value_;
  bool quoted_;
};

class CmakeFunction {
public:
  static std::shared_ptr<CmakeFunction> create(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments);

  const std::string& name() const;
  const std::string argumentString() const;
  const CmakeFunctionArgument* argumentByPosition(unsigned int position) const;

private:
  CmakeFunction(const std::string& name, const std::vector<CmakeFunctionArgument>& arguments);

  std::string name_;
  std::vector<CmakeFunctionArgument> arguments_;
};

class CmakeFile {
public:
  CmakeFile(const std::string& path);

  const std::string& path() const;

  void addChild(const std::shared_ptr<CmakeFile>& child);
  void addFiles(const std::vector<std::string>& includeFiles, const std::vector<std::string>& sourceFiles);
  void addFunction(const std::shared_ptr<CmakeFunction>& func);
  void write();

private:
  std::string path_;
  std::vector<std::shared_ptr<CmakeFile>> children_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
  std::vector<std::shared_ptr<CmakeFunction>> functions_;
};

}

#endif