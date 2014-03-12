#ifndef ROOTCOMPRESSIONSETTINGS_H 
#define ROOTCOMPRESSIONSETTINGS_H 1

// STD
#include <string>

namespace RootHistCnv {

  /** @class RootCompressionSettings RootCompressionSettings.h
   *
   *  Simple class to decode a ROOT compression settings string, 
   *  of the form '<Alg>:<level>' into the integer code to pass
   *  to ROOT.
   *
   *  @author Chris Jones
   *  @date   2013-10-24
   */

  class RootCompressionSettings 
  {
  
  public:
  
    /// Standard constructor
    RootCompressionSettings( const std::string & settings );

    /// Destructor
    ~RootCompressionSettings( ) { }

  public:

    /// Get the level
    int level() const { return m_level; }

  private:

    /// The compression settings string
    std::string m_compSettings;

    /// The cached ROOT compression level int
    int m_level;

  };

}

#endif // ROOTCOMPRESSIONSETTINGS_H
