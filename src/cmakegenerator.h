#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H
#include <memory>
#include <string>

namespace file_utils {
class IgnoreFile;
class Directory;
}

class IoHandler {
public:
  virtual ~IoHandler() {};
  virtual void write(const std::string& text) = 0;
  virtual std::string input() = 0;
};

class CmakeGenerator {
public:
  CmakeGenerator(
    IoHandler& iohandler,
    const file_utils::IgnoreFile& ignoreFile,
    const std::string& cmakeVersion,
    const std::string& cppVersion
  );
  void run();
private:
  void placeInitialCmakeFiles(const std::shared_ptr<file_utils::Directory>& directoryRoot);
  void populateCmakeFiles(const std::shared_ptr<file_utils::Directory>& directoryRoot);
  void populateCmakeFile(const file_utils::Directory* directory, const std::string& cmakeVersion, const std::string& cppVersion);

  std::string defaultCmakeVersion_;
  std::string defaultCppVersion_;
  IoHandler& ioHandler_;
  const file_utils::IgnoreFile& ignoreFile_;
};

#endif