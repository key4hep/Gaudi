#ifndef GAUDIKERNEL_FILEINCIDENT_H
#define GAUDIKERNEL_FILEINCIDENT_H

/** @file FileIncident.h
 *  @brief This file contains the class definition for the FileIncident class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  @author Charles Leggett
 **/

#include "GaudiKernel/Incident.h"

#include <string>

/** @class FileIncident
 *  @brief This class is the FileIncident.
 **/
class GAUDI_API FileIncident : public Incident {
public:
  /// standard constructor
  FileIncident( std::string source, std::string type, std::string fileName );
  FileIncident( std::string source, std::string type, std::string fileName, std::string fileGuid );

  const std::string& fileName() const { return m_fileName; }
  const std::string& fileGuid() const { return m_fileGuid; }

private:
  std::string m_fileName;
  std::string m_fileGuid;
};

inline FileIncident::FileIncident( std::string source, std::string type, std::string fileName )
    : Incident( std::move( source ), std::move( type ) ), m_fileName( std::move( fileName ) ) {}

inline FileIncident::FileIncident( std::string source, std::string type, std::string fileName, std::string fileGuid )
    : Incident( std::move( source ), std::move( type ) )
    , m_fileName( std::move( fileName ) )
    , m_fileGuid( std::move( fileGuid ) ) {}

#endif
