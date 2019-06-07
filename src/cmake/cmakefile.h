#ifndef CMAKE_CMAKEFILE_H
#define CMAKE_CMAKEFILE_H
#include "cmakefunction.h"

#include <memory>
#include <string>
#include <vector>

class IoHandler;

namespace cmake {

struct Token;
class CmakeScanner;
class ICmakeFunctionCriteria;
class CmakeFile {
public:
  static std::shared_ptr<CmakeFile> parse(const std::string& directoryPath, const std::string& filePath, IoHandler& ioHandler);

  CmakeFile(const std::string& path);

  const std::string& path() const;
  const std::vector<CmakeFunction*> functions() const;
  bool hasPositions() const;
  int includeFilesOffsetAdded() const;
  CmakeFunction* getFunction(const ICmakeFunctionCriteria& criteria) const;

  void addFunction(const std::shared_ptr<CmakeFunction>& func);
  void replaceIncludeFiles(const std::vector<std::string>& includeFiles);
  void replaceSourceFiles(const std::vector<std::string>& sourceFiles);
  void removeIncludeFiles();
  void write();

private:
  static std::shared_ptr<CmakeFunction> parseFunction(const Token& parentToken, CmakeScanner& scanner);
  void addIncludeFunction(const std::vector<std::string>& includeFiles);
  void moveFunctions(std::vector<std::shared_ptr<CmakeFunction>>::iterator startItr, const int lineOffset);
  std::shared_ptr<CmakeFunction> createReplacementFunction(
    const std::string& functionName,
    const std::string& setArgumentName,
    const FilePosition& startPosition,
    const std::vector<std::string>& files
  );

  std::string path_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
  std::vector<std::shared_ptr<CmakeFunction>> functions_;
  bool hasPositions_;
};

}

#endif