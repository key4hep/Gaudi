/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/** test program for file parsing function
 * ---------------------------------------------------------------------
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

//<<<<<< INCLUDES                                                       >>>>>>

#include "GaudiKernel/DirSearchPath.h"
#include <iostream>

using namespace std;
using namespace boost::filesystem;

int main( int argc, char* argv[] ) {
  // start with empty path == current dir
  DirSearchPath searchPath;
  // process cmd line
  try {
    if ( argc < 2 || 3 < argc ) {
      cerr << "usage: " << argv[0] << " FILENAME [dir1[,dir2[,...]]]" << endl;
      return 1;
    } else if ( 3 == argc )
      searchPath = DirSearchPath( argv[2] );
  } catch ( const filesystem_error& err ) { cerr << err.what() << endl; }
  // now look for something
  string fullFileName;
  if ( !searchPath.find( argv[1], fullFileName ) )
    return 1;
  else
    cout << fullFileName << endl;

  return 0;
}
