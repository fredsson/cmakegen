#include "../cmdoptionparser.h"

#include <algorithm>

CmdOptionParser::CmdOptionParser(int argc, char* argv[])
  : begin_(argv), end_(argv + argc) {
}

bool CmdOptionParser::hasOption(const std::string& option) {
  return std::find(begin_, end_, option) != end_;
}

bool CmdOptionParser::hasAnyOption(const std::vector<std::string>& options) {
  return std::any_of(options.begin(), options.end(), [&](const auto& option) {
    return hasOption(option);
  });
}

const char* CmdOptionParser::getOption(const std::string& name) {
  auto** itr = std::find(begin_, end_, name);
  if (itr != end_ && itr++ != end_) {
    return *itr;
  }

  return nullptr;
}