#include "../cmakefile.h"
#include "cmakescanner.h"
#include "cmakeformatter.h"
#include "../../file_utils/fileutils.h"
#include "../cmakefunctioncriteria.h"
#include "constants.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace cmake {

namespace {
  const unsigned int IncludeFunctionArgumentPosition = 1;
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

CmakeFunction* CmakeFile::getFunction(const ICmakeFunctionCriteria& criteria) const {
  const auto itr = std::find_if(functions_.begin(), functions_.end(), [&criteria](const auto& function) {
    return criteria.matches(*function);
  });

  if (itr != functions_.end()) {
    return (*itr).get();
  }

  return nullptr;
}

void CmakeFile::addFunction(const std::shared_ptr<CmakeFunction>& func) {
  if (!hasPositions_ && func->hasPosition()) {
    hasPositions_ = true;
  }
  functions_.push_back(func);
}

void CmakeFile::replaceIncludeFiles(const std::vector<std::string>& includeFiles) {
  const auto includeFileCriteria = CmakeSetFileFunctionCriteria(CmakeSetFileFunctionCriteria::IncludeFiles);
  const auto* includeFileFunction = getFunction(includeFileCriteria);
  if (!includeFileFunction) {
    addIncludeFunction(includeFiles);
    return;
  }

  const int lineOffset = (includeFiles.size() - (includeFileFunction->arguments().size() - 1));

  const auto newFunction = createReplacementFunction(
    includeFileFunction->name(),
    constants::SetIncludeFilesArgumentName,
    *includeFileFunction->startPosition(),
    includeFiles
  );

  std::replace_if(functions_.begin(), functions_.end(), [&includeFileCriteria](const auto& function) {
    return includeFileCriteria.matches(*function);
  }, newFunction);

  const auto itr = std::find_if(functions_.begin(), functions_.end(), [&includeFileCriteria](const auto& function) {
    return includeFileCriteria.matches(*function);
  });
  moveFunctions(itr + 1, lineOffset);
}

void CmakeFile::replaceSourceFiles(const std::vector<std::string>& sourceFiles) {
  const auto sourceFileCriteria = CmakeSetFileFunctionCriteria(CmakeSetFileFunctionCriteria::SourceFiles);
  const auto* sourceFileFunction = getFunction(sourceFileCriteria);
  if (!sourceFileFunction) {
    return;
  }

  const int lineOffset = (sourceFiles.size() - (sourceFileFunction->arguments().size() - 1));

  const auto newFunction = createReplacementFunction(
    sourceFileFunction->name(),
    constants::SetSourceFilesArgumentName,
    *sourceFileFunction->startPosition(),
    sourceFiles
  );

  std::replace_if(functions_.begin(), functions_.end(), [&sourceFileCriteria](const auto& function) {
    return sourceFileCriteria.matches(*function);
  }, newFunction);

  const auto itr = std::find_if(functions_.begin(), functions_.end(), [&sourceFileCriteria](const auto& function) {
    return sourceFileCriteria.matches(*function);
  });
  moveFunctions(itr + 1, lineOffset);
}

void CmakeFile::removeIncludeFiles() {
  const auto* includeFileFunction = getFunction(CmakeSetFileFunctionCriteria(CmakeSetFileFunctionCriteria::IncludeFiles));
  if (includeFileFunction) {
    const int noOfArguments = includeFileFunction->arguments().size() - 1;
    const int functionLength = (includeFileFunction->startPosition()->line_ - includeFileFunction->endPosition()->line_);

    const auto criteria = CmakeSetFileFunctionCriteria(CmakeSetFileFunctionCriteria::IncludeFiles);
    auto itr = std::find_if(functions_.begin(), functions_.end(), [&criteria](const std::shared_ptr<CmakeFunction>& function) {
      return criteria.matches(*function);
    });

    itr = functions_.erase(itr);
    moveFunctions(itr, functionLength - noOfArguments);
  }

  const auto* projectFunction = getFunction(CmakeProjectFunctionCriteria());
  auto* outputFunc = getFunction(CmakeOutputFunctionCriteria(projectFunction->arguments()[0].value_));
  outputFunc->removeArgument(constants::SetIncludeFilesArgumentName);
}

void CmakeFile::write() {
  std::ofstream stream(path_ + "/" + constants::FileName);
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

void CmakeFile::addIncludeFunction(const std::vector<std::string>& includeFiles) {
  const auto srcFileCriteria = CmakeSetFileFunctionCriteria(CmakeSetFileFunctionCriteria::SourceFiles);
  const auto* srcFileFunction = getFunction(srcFileCriteria);

  const auto includeFunction = createReplacementFunction(
    srcFileFunction->name(),
    constants::SetIncludeFilesArgumentName,
    *srcFileFunction->startPosition(),
    includeFiles
  );

  auto itr = std::find_if(functions_.begin(), functions_.end(), [&srcFileCriteria](const auto& func) {
    return srcFileCriteria.matches(*func);
  });
  itr = functions_.insert(itr, includeFunction);
  moveFunctions(itr + 1, includeFiles.size() + 3);

  const auto* projectFunction = getFunction(CmakeProjectFunctionCriteria());
  auto* outputFunc = getFunction(CmakeOutputFunctionCriteria(projectFunction->arguments()[0].value_));

  outputFunc->insertArgument(
    IncludeFunctionArgumentPosition,
    {constants::SetIncludeFilesArgumentName, {outputFunc->arguments()[0].position_->line_, outputFunc->arguments()[1].position_->column_}}
  );
}

void CmakeFile::moveFunctions(std::vector<std::shared_ptr<CmakeFunction>>::iterator startItr, const int lineOffset) {
  for (auto it = startItr; it != functions_.end(); it++) {
    (*it)->move(lineOffset);
  }
}

std::shared_ptr<CmakeFunction> CmakeFile::createReplacementFunction(
  const std::string& functionName,
  const std::string& setArgumentName,
  const FilePosition& startPosition,
  const std::vector<std::string>& files
) {
  unsigned int line = startPosition.line_ + 1;
  std::vector<CmakeFunctionArgument> arguments = {{setArgumentName, startPosition, false}};
  std::transform(files.begin(), files.end(), std::back_inserter(arguments), [this, &line](const auto& file)->CmakeFunctionArgument {
    return {"\"" + file_utils::makeRelative(file, path_) + "\"", {line++, 3}, true};
  });

  return CmakeFunction::create(
    functionName,
    arguments,
    startPosition,
    {line, 0}
  );
}

}