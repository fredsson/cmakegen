#include "../cmakefile.h"
#include "cmakescanner.h"
#include "cmakeformatter.h"

#include <algorithm>
#include <fstream>
#include <sstream>


namespace cmake {

CmakeFunctionArgument::CmakeFunctionArgument(std::string value)
  : value_(value), position_(nullptr), quoted_(false) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, bool quoted)
  : value_(value), position_(nullptr), quoted_(quoted) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, const FilePosition position)
  : value_(value), position_(std::make_shared<FilePosition>(position)), quoted_(false) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, const FilePosition position, bool quoted)
  : value_(value), position_(std::make_shared<FilePosition>(position)), quoted_(quoted) {
}

std::shared_ptr<CmakeFunction> CmakeFunction::create(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments
) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(name, arguments, nullptr, nullptr));
}

std::shared_ptr<CmakeFunction> CmakeFunction::create(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments,
  const FilePosition startPosition,
  const FilePosition endPosition
) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(
    name,
    arguments,
    std::make_unique<FilePosition>(startPosition),
    std::make_unique<FilePosition>(endPosition)
  ));
}

CmakeFunction::CmakeFunction(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments,
  std::unique_ptr<FilePosition> startPosition,
  std::unique_ptr<FilePosition> endPosition
) : name_(name), arguments_(arguments), startPosition_(std::move(startPosition)), endPosition_(std::move(endPosition)) {
}

bool CmakeFunction::hasPosition() const {
  return startPosition_ && endPosition_;
}

const std::string& CmakeFunction::name() const {
  return name_;
}

const FilePosition* CmakeFunction::startPosition() const {
  return startPosition_.get();
}

const FilePosition* CmakeFunction::endPosition() const {
  return endPosition_.get();
}

const std::vector<CmakeFunctionArgument>& CmakeFunction::arguments() const {
  return arguments_;
}

std::shared_ptr<CmakeFile> CmakeFile::parse(const std::string& directoryPath, const std::string& filePath) {
  CmakeScanner scanner(filePath);

  Token token = { TokenType::NONE, "", 0, 0, 0 };
  bool hasEncounteredNewline = true;
  auto cmakeFile = std::make_shared<CmakeFile>(directoryPath);
  while (token.type != TokenType::ENDOFFILE) {
    token = scanner.getNextToken();

    switch (token.type) {
      case TokenType::NEWLINE:
        hasEncounteredNewline = true;
      break;
      case TokenType::IDENTIFIER:
        if (hasEncounteredNewline) {
          hasEncounteredNewline = false;
          const auto function = CmakeFile::parseFunction(token, scanner);
          if (function) {
            cmakeFile->addFunction(function);
          }
        }
      break;
      default:
      break;
    }
  }

  return cmakeFile;
}

CmakeFile::CmakeFile(const std::string& path)
  : path_(path), includeFiles_({}), sourceFiles_({}), hasPositions_(false) {
}

const std::string& CmakeFile::path() const {
  return path_;
}

const std::vector<CmakeFunction*> CmakeFile::functions() const {
  std::vector<CmakeFunction*> functions = {};
  std::transform(functions_.begin(), functions_.end(), std::back_inserter(functions), [](const auto& f){
    return f.get();
  });

  return functions;
}

bool CmakeFile::hasPositions() const {
  return hasPositions_;
}

void CmakeFile::addFiles(const std::vector<std::string>& includeFiles, const std::vector<std::string>& sourceFiles) {
  std::transform(includeFiles.begin(), includeFiles.end(), std::back_inserter(includeFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });

  std::transform(sourceFiles.begin(), sourceFiles.end(), std::back_inserter(sourceFiles_), [this](const auto& file) {
    const auto relativePath = file.substr(path_.size() + 1);
    return relativePath;
  });
}

void CmakeFile::addFunction(const std::shared_ptr<CmakeFunction>& func) {
  if (!hasPositions_ && func->hasPosition()) {
    hasPositions_ = true;
  }
  functions_.push_back(func);
}

void CmakeFile::write() {
  std::ofstream stream(path_ + "/CMakeLists.txt");
  if (!stream.is_open()) {
    return;
  }

  CmakeFormatter formatter;
  formatter.format(stream, *this);

  stream.close();
}

std::shared_ptr<CmakeFunction> CmakeFile::parseFunction(const Token& parentToken, CmakeScanner& scanner) {
  Token token = { TokenType::NONE, "", 0, 0, 0 };
  std::vector<CmakeFunctionArgument> arguments = {};
  while (token.type != TokenType::PARENRIGHT) {
    token = scanner.getNextToken();
    switch(token.type) {
      case TokenType::IDENTIFIER:
      case TokenType::ARGUMENTUNQUOTED:
        arguments.push_back({ token.text, {token.line, token.column}});
      break;
      case TokenType::ARGUMENTQUOTED:
        arguments.push_back({ token.text, {token.line, token.column}, true});
      break;
      default:
      break;
    }
  }

  return CmakeFunction::create(parentToken.text, arguments, {parentToken.line, parentToken.column}, {token.line, token.column});
}

}