#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H
#include <string>

namespace file_utils {
class IgnoreFile;
}

class IoHandler {
public:
  virtual ~IoHandler() {};
  virtual void write(const std::string& text) = 0;
  virtual std::string input() = 0;
};

class CmakeGenerator {
public:
  CmakeGenerator(IoHandler& iohandler, const file_utils::IgnoreFile& ignoreFile);
  void run();
private:
  void placeInitialCmakeFiles();
  void populateCmakeFiles();

  IoHandler& ioHandler_;
  const file_utils::IgnoreFile& ignoreFile_;
};

#endif