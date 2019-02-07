#ifndef CMAKE_CMAKESCANNER_H
#define CMAKE_CMAKESCANNER_H
#include <fstream>
#include <memory>
#include <string>

namespace cmake {

enum TokenType {
  NONE,
  SPACE,
  NEWLINE,
  IDENTIFIER,
  PARENLEFT,
  PARENRIGHT,
  ARGUMENTUNQUOTED,
  ARGUMENTQUOTED,
  ARGUMENTBRACKET,
  COMMENTBRACKET,
  BADCHARACTER,
  BADBRACKET,
  BADSTRING,
  ENDOFFILE,
};

struct Token {
  TokenType type;
  std::string text;
  unsigned int length;
  unsigned int line;
  unsigned int column;
};

class CmakeScanner {
public:
  CmakeScanner(const std::string& filePath);
  ~CmakeScanner();
  Token getNextToken();
private:
  Token getComment();
  unsigned int currentLine_;
  unsigned int currentColumn_;
  char lastChar_;
  bool scanningArguments_;
  std::ifstream file_;
};

}

#endif