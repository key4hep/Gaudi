#ifndef ROOTHISTCNV_ROOTDIRFCN_HPP
#define ROOTHISTCNV_ROOTDIRFCN_HPP

#include <string>

namespace RootHistCnv
{

  bool RootCd( const std::string& full );

  bool RootMkdir( const std::string& full );

  std::string RootPwd();

  bool RootTrimLeadingDir( std::string& full, std::string dir );
}

#endif
