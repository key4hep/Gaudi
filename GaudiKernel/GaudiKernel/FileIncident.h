#ifndef GAUDIKERNEL_FILEINCIDENT_H
#define GAUDIKERNEL_FILEINCIDENT_H

/** @file FileIncident.h
 *  @brief This file contains the class definition for the FileIncident class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  @author Charles Leggett
 *  $Id: $
 **/

#include "GaudiKernel/Incident.h"

#include <string>

/** @class FileIncident
 *  @brief This class is the FileIncident.
 **/
class GAUDI_API FileIncident : public Incident {
public:
   /// standard constructor
   FileIncident(const std::string& source,
           const std::string& type,
           const std::string& fileName,
           bool isPayload = false);
   FileIncident(const std::string& source,
           const std::string& type,
           const std::string& fileName,
           const std::string& fileGuid,
           bool isPayload = false);
   FileIncident(const FileIncident& rhs);
   virtual ~FileIncident();

   /// Overloaded Assignment Operator
   const FileIncident& operator=(const FileIncident& rhs);
   const std::string& fileName() const;
   const std::string& fileGuid() const;
   bool isPayload() const;

private:
   std::string m_fileName;
   std::string m_fileGuid;
   bool m_isPayload;
};

inline FileIncident::FileIncident(const std::string& source,
        const std::string& type,
        const std::string& fileName,
        bool isPayload) : Incident(source, type), m_fileName(fileName), m_fileGuid(""), m_isPayload(isPayload) {}
inline FileIncident::FileIncident(const std::string& source,
        const std::string& type,
        const std::string& fileName,
        const std::string& fileGuid,
        bool isPayload) : Incident(source, type), m_fileName(fileName), m_fileGuid(fileGuid), m_isPayload(isPayload) {}
inline FileIncident::FileIncident(const FileIncident& rhs) : Incident(rhs),
        m_fileName(rhs.m_fileName),
        m_fileGuid(rhs.m_fileGuid),
        m_isPayload(rhs.m_isPayload) {}
inline FileIncident::~FileIncident() {}

inline const FileIncident& FileIncident::operator=(const FileIncident& rhs) {
   if (this != &rhs) {
      Incident::operator=(rhs);
      m_fileName = rhs.m_fileName;
      m_fileGuid = rhs.m_fileGuid;
      m_isPayload = rhs.m_isPayload;
   }
   return(rhs);
}

inline const std::string& FileIncident::fileName() const { return(m_fileName); }
inline const std::string& FileIncident::fileGuid() const { return(m_fileGuid); }
inline bool FileIncident::isPayload() const { return(m_isPayload); }

#endif
