#ifndef _FILEREADTOOL_H
#define _FILEREADTOOL_H

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IFileAccess.h"

/** @class FileReadTool FileReadTool.h
 *
 *  Basic implementation of the IFileAccess interface.
 *  This tool simply takes a path to a file as url and return the std::istream interface
 *  of std::ifstream.
 *
 *  @author Marco Clemencic
 *  @date 2008-01-18
 */
class FileReadTool : public extends<AlgTool, IFileAccess>
{

public:
  /// Standard constructor
  FileReadTool( const std::string& type, const std::string& name, const IInterface* parent );

  std::unique_ptr<std::istream> open( const std::string& url ) override;

  /// Protocols supported by the instance.
  const std::vector<std::string>& protocols() const override;
};

#endif // _FILEREADTOOL_H
