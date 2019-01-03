#ifndef CMAKE_CMAKEFORMATTER_H
#define CMAKE_CMAKEFORMATTER_H
#include <ostream>

namespace cmake {

class CmakeFile;
struct FilePosition;
class CmakeFormatter {
public:
  CmakeFormatter();
  void format(std::ostream& stream, CmakeFile& file);
private:
  void formatGenerated(std::ostream& stream, CmakeFile& file);
  void formatFileWithPositions(std::ostream& stream, CmakeFile& file);

  void write(std::ostream& stream, std::string text);
  void moveStreamToPosition(std::ostream& stream, const FilePosition& position);
  void addNewLine(std::ostream& stream);
  void addColumn(std::ostream& stream);

  unsigned int currentLine_;
  unsigned int currentColumn_;
};

}

#endif