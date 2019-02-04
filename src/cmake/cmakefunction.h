#ifndef CMAKE_CMAKEFUNCTION_H
#define CMAKE_CMAKEFUNCTION_H
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

  void move(const int lines);
  void insertArgument(const unsigned int position, const CmakeFunctionArgument& argument);
  void removeArgument(const std::string& name);

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

}

#endif
