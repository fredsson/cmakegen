#ifndef IOHANDLER_HPP
#define IOHANDLER_HPP
#include <string>

class IoHandler {
public:
  virtual ~IoHandler() {};
  virtual void write(const std::string& text) = 0;
  virtual std::string input() = 0;
};

#endif
