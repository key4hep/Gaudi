/** test program for file parsing function
  * ---------------------------------------------------------------------
  * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
  */

// $Id: fileparser_test.cpp,v 1.2 2007/10/09 10:28:48 marcocle Exp $

//<<<<<< INCLUDES                                                       >>>>>>

#include <iostream>
#include "GaudiKernel/DirSearchPath.h"

using namespace std;
using namespace boost::filesystem;

int main(int argc, char* argv[]) {
  //start with empty path == current dir
  DirSearchPath searchPath;
  //process cmd line	
  try {
    if ( argc < 2 || 3 < argc ) {
      cerr << "usage: " << argv[0] << " FILENAME [dir1[,dir2[,...]]]" << endl;
      return 1;
    } else if (3 == argc) searchPath=DirSearchPath(argv[2]);    
  } catch (const filesystem_error& err) {
    cerr << err.what() << endl;
  }  
  //now look for something
  string fullFileName;
  if (!searchPath.find(argv[1], fullFileName))     return 1;
  else cout << fullFileName << endl;
      
  return 0;
  
}
