// $Id: FileReadTool.h,v 1.2 2008/06/12 15:23:03 marcocle Exp $
#ifndef _FILEREADTOOL_H
#define _FILEREADTOOL_H

#include "GaudiKernel/IFileAccess.h"
#include "GaudiKernel/AlgTool.h"

/** @class FileReadTool FileReadTool.h
 *
 *  Basic implementation of the IFileAccess interface.
 *  This tool simply takes a path to a file as url and return the std::istream interface
 *  of std::ifstream.
 *
 *  @author Marco Clemencic
 *  @date 2008-01-18
 */
class FileReadTool : public extends1<AlgTool, IFileAccess> {

public:

  /// Standard constructor
  FileReadTool( const std::string& type,
                const std::string& name,
                const IInterface* parent);

  virtual ~FileReadTool();

  //virtual StatusCode initialize();
  //virtual StatusCode finalize();

  virtual std::auto_ptr<std::istream> open(const std::string &url);

  /// Protocols supported by the instance.
  virtual const std::vector<std::string> &protocols() const;

private:

  /// Vector of supported protocols.
  std::vector<std::string> m_protocols;

};

#endif  // _FILEREADTOOL_H
