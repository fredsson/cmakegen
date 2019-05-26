#include "cmakeformatter.h"
#include "../cmakefile.h"
#include <algorithm>

namespace cmake {

static const unsigned int START_LINE = 1;
static const unsigned int START_COLUMN = 1;

CmakeFormatter::CmakeFormatter()
  : currentLine_(START_LINE), currentColumn_(START_COLUMN) {
}

void CmakeFormatter::format(std::ostream& stream, CmakeFile& file) {
  if (file.hasPositions()) {
    formatFileWithPositions(stream, file);
    return;
  }

  formatGenerated(stream, file);
}

void CmakeFormatter::formatGenerated(std::ostream& stream, CmakeFile& file) {
  for (const auto& function : file.functions()) {
    const auto& arguments = function->arguments();

    const auto includeOrSourceList = arguments.size() > 1 &&
    (arguments[0].value_ == "INCLUDE_FILES" || arguments[0].value_ == "SRC_FILES");

    stream << function->name() << "(" << arguments[0].value_;

    for (size_t i = 1; i < arguments.size(); i++) {
      stream << (includeOrSourceList ? "\n  " : " ") << arguments[i].value_;
    }

    stream << (includeOrSourceList ? "\n" : "") << ")";
    stream << "\n\n";
  }

  stream << "\n";
}

void CmakeFormatter::formatFileWithPositions(std::ostream& stream, CmakeFile& file) {
  for (const auto& f : file.functions()) {
    const auto* startPosition = f->startPosition();

    moveStreamToPosition(stream, *startPosition);

    if (f->name()[0] == '#') {
      write(stream, f->name());
    } else {
      write(stream, f->name() + "(");

      for (const auto& argument : f->arguments()) {
        moveStreamToPosition(stream, *argument.position_);

        write(stream, argument.value_);

      }

      moveStreamToPosition(stream, *f->endPosition());

      stream << ")";
    }
  }

  stream << "\n";
}

void CmakeFormatter::write(std::ostream& stream, std::string text) {
  stream << text;
  currentColumn_ += text.size();
  const auto n = std::count(text.begin(), text.end(), '\n');
  currentLine_ += n;
}

void CmakeFormatter::moveStreamToPosition(std::ostream& stream, const FilePosition& position) {
  while (currentLine_ < position.line_) {
    addNewLine(stream);
  }

  while (currentColumn_ < position.column_) {
    addColumn(stream);
  }
}

void CmakeFormatter::addNewLine(std::ostream& stream) {
  stream << "\n";
  currentLine_++;
  currentColumn_ = START_COLUMN;
}

void CmakeFormatter::addColumn(std::ostream& stream) {
  stream << " ";
  currentColumn_++;
}

}