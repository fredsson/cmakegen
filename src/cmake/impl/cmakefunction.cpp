#include "../cmakefunction.h"
#include <algorithm>
#include <iterator>

namespace cmake {

namespace {
  const unsigned int ArgumentSpace = 1;
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value)
  : value_(value), position_(nullptr), quoted_(false) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, bool quoted)
  : value_(value), position_(nullptr), quoted_(quoted) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, const FilePosition position)
  : value_(value), position_(std::make_shared<FilePosition>(position)), quoted_(false) {
}

CmakeFunctionArgument::CmakeFunctionArgument(std::string value, const FilePosition position, bool quoted)
  : value_(value), position_(std::make_shared<FilePosition>(position)), quoted_(quoted) {
}

std::shared_ptr<CmakeFunction> CmakeFunction::create(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments
) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(name, arguments, nullptr, nullptr));
}

std::shared_ptr<CmakeFunction> CmakeFunction::create(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments,
  const FilePosition startPosition,
  const FilePosition endPosition
) {
  return std::shared_ptr<CmakeFunction>(new CmakeFunction(
    name,
    arguments,
    std::make_unique<FilePosition>(startPosition),
    std::make_unique<FilePosition>(endPosition)
  ));
}

CmakeFunction::CmakeFunction(
  const std::string& name,
  const std::vector<CmakeFunctionArgument>& arguments,
  std::unique_ptr<FilePosition> startPosition,
  std::unique_ptr<FilePosition> endPosition
) : name_(name), arguments_(arguments), startPosition_(std::move(startPosition)), endPosition_(std::move(endPosition)) {
}

bool CmakeFunction::hasPosition() const {
  return startPosition_ && endPosition_;
}

const std::string& CmakeFunction::name() const {
  return name_;
}

const FilePosition* CmakeFunction::startPosition() const {
  return startPosition_.get();
}

const FilePosition* CmakeFunction::endPosition() const {
  return endPosition_.get();
}

const std::vector<CmakeFunctionArgument>& CmakeFunction::arguments() const {
  return arguments_;
}

void CmakeFunction::move(const int lines) {
  if(!hasPosition()) {
    return;
  }

  startPosition_->line_ = startPosition_->line_ + lines;
  endPosition_->line_ = endPosition_->line_ + lines;

  if (arguments_.empty()) {
    return;
  }

  std::vector<CmakeFunctionArgument> arguments = {};
  arguments.reserve(arguments_.size());
  std::transform(arguments_.begin(), arguments_.end(), std::back_inserter(arguments), [&lines](const CmakeFunctionArgument& arg) {
    const FilePosition position = {arg.position_->line_ + lines, arg.position_->column_};
    return CmakeFunctionArgument(arg.value_, position, arg.quoted_);
  });
  arguments_ = arguments;
}

void CmakeFunction::insertArgument(const unsigned int position, const CmakeFunctionArgument& argument) {
  // TODO: handle adding argument on different line

  const auto itr = arguments_.insert(arguments_.begin() + position, argument);

  for (auto it = itr + 1; it != arguments_.end(); it++) {
    it->position_->column_ =  it->position_->column_ + argument.value_.size() + ArgumentSpace;
  }
}

void CmakeFunction::removeArgument(const std::string& name) {
  // TODO: handle removing argument on different line

  endPosition_->column_ = endPosition_->column_ - ((unsigned int)name.size() + ArgumentSpace);

  std::vector<CmakeFunctionArgument> arguments = {};
  bool shouldMoveArguments = false;
  for (const CmakeFunctionArgument& argument : arguments_) {
    if (argument.value_ != name) {
      const FilePosition position = shouldMoveArguments ?
        FilePosition{argument.position_->line_, argument.position_->column_ - ((unsigned)name.size() + ArgumentSpace)} :
        *argument.position_;

      arguments.push_back({ argument.value_, position, argument.quoted_});
    } else {
      shouldMoveArguments = true;
    }
  }

  arguments_ = arguments;
}

}