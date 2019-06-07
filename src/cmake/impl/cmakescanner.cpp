#include "cmakescanner.h"

#include <algorithm>
#include <functional>
#include <vector>
#include <sstream>
#include <stdio.h>

namespace cmake {

namespace {
  char nextCharacter(std::ifstream& file) {
    char c;
    file.get(c);
    return c;
  }

  const std::vector<char> argumentCharacters = {'.', '_', '\\', '/', '$', '{', '}', ':', '-', '"', '+'};
  bool allowedInArgument(char c) {
    bool specialCharacter = std::any_of(argumentCharacters.begin(), argumentCharacters.end(), [c](char argumentCharacter){
      return c == argumentCharacter;
    });

    return specialCharacter || isalnum(c);
  }

  bool allowedInIdentifier(char c) {
    return c == '_' || isalnum(c);
  }

  struct AllowedCharactersResult {
    char nextCharacter;
    std::string processedCharacters;
  };
  AllowedCharactersResult getAllowedCharacters(std::ifstream& file, char firstCharacter, std::function<bool(char)> predicate) {
    char c = firstCharacter;
    std::stringstream s;
    while(predicate(c)) {
      s << c;
      c = nextCharacter(file);
    }

    return { c, s.str() };
  }
}

CmakeScanner::CmakeScanner(const std::string& filePath)
  : currentLine_(1), currentColumn_(1), lastChar_(0), scanningArguments_(false), file_(filePath) {
}

CmakeScanner::~CmakeScanner() {
  file_.close();
}

Token CmakeScanner::getNextToken() {
  if (!file_.is_open()) {
    return { TokenType::ENDOFFILE, "", 0, currentLine_, currentColumn_ };
  }

  char c = lastChar_ ? lastChar_ : nextCharacter(file_);
  if (lastChar_) {
    lastChar_ = 0;
  }

  if (file_.eof()) {
    return {TokenType::ENDOFFILE, "", 0, currentLine_, currentColumn_};
  }

  if (c == ' ') {
    return {TokenType::SPACE, " ", 1, currentLine_, currentColumn_++};
  }

  if (c == '\n') {
    currentColumn_ = 1;
    currentLine_++;
    return {TokenType::NEWLINE, "\n", 1, currentLine_, currentColumn_};
  }

  if (c == '(' || c == ')') {
    scanningArguments_ = (c == '(');
    return {
      (c == '(') ? TokenType::PARENLEFT : TokenType::PARENRIGHT,
      {1, c},
      1,
      currentLine_,
      currentColumn_++
    };
  }

  if (scanningArguments_) {
    return getArgument(c);
  }

  if (allowedInIdentifier(c)) {
    return getIdentifier(c);
  }

  if(c == '#') {
    return getComment();
  }

  return { TokenType::BADCHARACTER, "", 0, currentLine_, currentColumn_++ };
}

Token CmakeScanner::getComment() {
  bool multiLine = false;
  char c = '#';
  std::stringstream s;
  s << c;

  char previous = c;
  c = nextCharacter(file_);

  int commentLines = 0;
  int commentColumns = 1;
  while ((multiLine || c != '\n') && (!multiLine || (c != ']' || previous != ']'))) {
    if (c == '[' && previous == '[') {
      multiLine = true;
    }
    if (multiLine && c == '\n') {
      commentLines++;
      commentColumns = 0;
    }
    s << c;
    previous = c;
    c = nextCharacter(file_);
    commentColumns++;
  }

  if (multiLine) {
    s << c;
    commentColumns++;
  }

  lastChar_ = c;
  const auto comment = s.str();
  const auto column = currentColumn_;
  const auto line = currentLine_;

  currentColumn_ += commentColumns;
  currentLine_ += commentLines;

  return {TokenType::COMMENTBRACKET, comment, (unsigned int)comment.size(), line, column};
}

Token CmakeScanner::getArgument(char c) {
  bool quoted = (c == '"');
  const auto result = getAllowedCharacters(file_, c, allowedInArgument);
  const unsigned int noOfCharactersProcessed = result.processedCharacters.size();
  const auto column = currentColumn_;

  lastChar_ = result.nextCharacter;
  currentColumn_ += noOfCharactersProcessed;
  return {
    quoted ? TokenType::ARGUMENTQUOTED : TokenType::ARGUMENTUNQUOTED,
    result.processedCharacters,
    noOfCharactersProcessed,
    currentLine_,
    column
  };
}

Token CmakeScanner::getIdentifier(char c) {
  const auto result = getAllowedCharacters(file_, c, allowedInIdentifier);
  const unsigned int noOfCharactersProcessed = result.processedCharacters.size();
  const auto column = currentColumn_;

  lastChar_ = result.nextCharacter;
  currentColumn_ += noOfCharactersProcessed;

  return {
    TokenType::IDENTIFIER,
    result.processedCharacters,
    noOfCharactersProcessed,
    currentLine_,
    column
  };
}

}