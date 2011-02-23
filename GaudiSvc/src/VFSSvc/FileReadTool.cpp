#include "FileReadTool.h"
#include <fstream>

#include "GaudiKernel/ToolFactory.h"
DECLARE_TOOL_FACTORY( FileReadTool )

FileReadTool::FileReadTool( const std::string& type,
                            const std::string& name,
                            const IInterface* parent):
  base_class(type, name, parent)
{
  //declareInterface<IFileAccess>(this);
  m_protocols.push_back("file");
}

FileReadTool::~FileReadTool(){}

std::auto_ptr<std::istream> FileReadTool::open(const std::string &url) {
  // remove the optional "file://" from the beginning of the url
  std::string path;
  if ( url.substr(0,7) == "file://" ) {
    path = url.substr(7);
  } else {
    path = url;
  }
  return std::auto_ptr<std::istream>(new std::ifstream(path.c_str()));
}

const std::vector<std::string> &FileReadTool::protocols() const
{
  return m_protocols;
}
