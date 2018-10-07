#ifndef CMDOPTIONPARSER_H
#define CMDOPTIONPARSER_H
#include <vector>
#include <string>

class CmdOptionParser {
public:
  CmdOptionParser(int argc, char* argv[]);

  bool hasOption(const std::string& option);
  bool hasAnyOption(const std::vector<std::string>& options);

  const char* getOption(const std::string& name);

private:
  char** begin_;
  char** end_;
};

#endif