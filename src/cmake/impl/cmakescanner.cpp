#include "cmakescanner.h"
#include <sstream>
#include <stdio.h>
#include <iostream>

namespace cmake {

namespace {
  bool isAlphaNum(const char& c) {
    if(c == '.' || c == '_' || c == '\\' || c == '/') {
      return true;
    }

    return isalnum(c);
  }
}

CmakeScanner::CmakeScanner(const std::string& filePath)
  : currentLine_(1), currentColumn_(1), lastChar_(0), scanningArguments_(false), file_(filePath) {
}

CmakeScanner::~CmakeScanner() {
  file_.close();
}

// https://github.com/Kitware/CMake/blob/e80e8eb609cc7dd8c4dca46b9c2819afd2293229/Source/LexerParser/cmListFileLexer.in.l
// http://llvm.org/docs/tutorial/LangImpl01.html
// https://stackoverflow.com/questions/6216449/where-can-i-learn-the-basics-of-writing-a-lexer
Token CmakeScanner::getNextToken() {
  if (!file_.is_open()) {
    return { TokenType::ENDOFFILE, "", 0, currentLine_, currentColumn_ };
  }

  char c;
  bool tokenFound = false;
  while(!tokenFound) {
    if (lastChar_) {
      c = lastChar_;
      lastChar_ = 0;
    } else {
      file_.get(c);
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

    if (isAlphaNum(c) || c == '"') {
      bool quoted = (c == '"');
      std::stringstream s;
      while(isAlphaNum(c) || c == '"') {
        s << c;
        file_.get(c);
      }

      const auto str = s.str();
      lastChar_ = c;
      const auto column = currentColumn_;
      currentColumn_ += str.size();
      return {
        scanningArguments_ ? (quoted ? TokenType::ARGUMENTQUOTED : TokenType::ARGUMENTUNQUOTED) : TokenType::IDENTIFIER,
        str,
        (unsigned int)str.size(),
        currentLine_,
        column
      };
    }

    return { TokenType::BADCHARACTER, "", 0, currentLine_, currentColumn_ };
  }

  return { TokenType::NONE, "", 0, currentLine_, currentColumn_ };
}

}