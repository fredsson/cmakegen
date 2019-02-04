#ifndef CMAKE_CMAKEFUNCTIONCRITERIA_H
#define CMAKE_CMAKEFUNCTIONCRITERIA_H
#include <string>

namespace cmake {

class CmakeFunction;
class ICmakeFunctionCriteria {
public:
  virtual ~ICmakeFunctionCriteria();
  virtual bool matches(const CmakeFunction& function) const = 0;
};

class CmakeSetFileFunctionCriteria : public ICmakeFunctionCriteria {
public:
  enum FileFunctionType { IncludeFiles, SourceFiles };

  CmakeSetFileFunctionCriteria(FileFunctionType type);
  bool matches(const CmakeFunction& function) const override;
private:
  FileFunctionType type_;
};

class CmakeProjectFunctionCriteria : public ICmakeFunctionCriteria {
public:
  bool matches(const CmakeFunction& function) const override;
};

class CmakeOutputFunctionCriteria : public ICmakeFunctionCriteria {
public:
  CmakeOutputFunctionCriteria(const std::string& projectName);
  bool matches(const CmakeFunction& function) const override;
private:
  std::string projectName_;
};

}

#endif