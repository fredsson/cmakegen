#ifndef CMAKE_CMAKEFILE_H
#define CMAKE_CMAKEFILE_H
#include <memory>
#include <string>
#include <vector>

namespace cmake {

struct FilePosition {
  unsigned int line_;
  unsigned int column_;
};

struct CmakeFunctionArgument {
  CmakeFunctionArgument(std::string value);
  CmakeFunctionArgument(std::string value, bool quoted);
  CmakeFunctionArgument(std::string value, const FilePosition position);
  CmakeFunctionArgument(std::string value, const FilePosition position, bool quoted);
  std::string value_;
  std::shared_ptr<FilePosition> position_;
  bool quoted_;
};

struct Token;
class CmakeScanner;

class CmakeFunction {
public:
  static std::shared_ptr<CmakeFunction> create(
    const std::string& name,
    const std::vector<CmakeFunctionArgument>& arguments
  );

  static std::shared_ptr<CmakeFunction> create(
    const std::string& name,
    const std::vector<CmakeFunctionArgument>& arguments,
    const FilePosition startPosition,
    const FilePosition endPosition
  );

  bool hasPosition() const;

  const std::string& name() const;
  const FilePosition* startPosition() const;
  const FilePosition* endPosition() const;
  const std::vector<CmakeFunctionArgument>& arguments() const;

private:
  CmakeFunction(
    const std::string& name,
    const std::vector<CmakeFunctionArgument>& arguments,
    std::unique_ptr<FilePosition> startPosition,
    std::unique_ptr<FilePosition> endPosition
  );

  std::string name_;
  std::vector<CmakeFunctionArgument> arguments_;
  std::unique_ptr<FilePosition> startPosition_;
  std::unique_ptr<FilePosition> endPosition_;
};

class CmakeFile {
public:
  static std::shared_ptr<CmakeFile> parse(const std::string& directoryPath, const std::string& filePath);

  CmakeFile(const std::string& path);

  const std::string& path() const;
  const std::vector<CmakeFunction*> functions() const;
  bool hasPositions() const;

  void addFiles(const std::vector<std::string>& includeFiles, const std::vector<std::string>& sourceFiles);
  void addFunction(const std::shared_ptr<CmakeFunction>& func);
  void write();

private:
  static std::shared_ptr<CmakeFunction> parseFunction(const Token& parentToken, CmakeScanner& scanner);

  std::string path_;
  std::vector<std::string> includeFiles_;
  std::vector<std::string> sourceFiles_;
  std::vector<std::shared_ptr<CmakeFunction>> functions_;
  bool hasPositions_;
};

}

#endif