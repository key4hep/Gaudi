#include "GaudiKernel/Environment.h"
#include "GaudiKernel/System.h"
#include <cstdlib>

namespace {
  std::string i_resolve(std::string& source, int recursions)  {
    if ( recursions > 0 )  {
      int lvl = 0, mx_lvl = 0;
      for(const char* c=source.c_str(), *beg=0; *c != 0; ++c)  {
        switch ( *c )  {
          case '$':
            if ( *(c+1) == '{' )  {
              lvl++;
              if ( lvl > mx_lvl )  {
                mx_lvl = lvl;
                beg = c;
              }
              c += 2;
            }
            break;
          case '}':
            if ( lvl == mx_lvl )  {
              std::string env(beg+2,c-beg-2);
              std::string rep;
              if ( ! System::getEnv(env.c_str(), rep) )
            	rep = i_resolve(env, --recursions);
              if ( rep.length() )  {
                std::string e(beg,c-beg+1);
                size_t idx=std::string::npos;
                while((idx=source.find(e)) != std::string::npos)  {
                  source.replace(idx, e.length(), rep);
                }
                return i_resolve(source, --recursions);
              }
              else  {
                // error: environment cannot be resolved....
                // Try to continue, but there is not too much hope.
              }
            }
            lvl--;
            break;
          default:
            break;
        }
      }
    }
    return source;
  }
}

StatusCode System::resolveEnv(const std::string& var, std::string& res, int recursions)  {
  std::string source = var;
  res = i_resolve(source, recursions);
  if ( res.find("${") == std::string::npos ) {
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

std::string System::homeDirectory()  {
  // Return the user's home directory.
  std::string home_dir = "./";
  // Try to replace the current value with the content of several
  // environment variables
  if ( ! (System::getEnv("home", home_dir) ||
          System::getEnv("HOME", home_dir)) ) {
    // for Windows NT HOME might be defined as either $(HOMESHARE)/$(HOMEPATH)
    //                                         or     $(HOMEDRIVE)/$(HOMEPATH)
    if ( System::getEnv("HOMESHARE", home_dir) ||
         System::getEnv("HOMEDRIVE", home_dir) ) {
      std::string path;
      if (System::getEnv("HOMEPATH", path))
    	home_dir += path;
    }
  }
  return home_dir;
}

std::string System::tempDirectory()   {
  // Return a user configured or systemwide directory to create
  // temporary files in.
  std::string dir;
  if ( System::getEnv("TEMP", dir) ||
       System::getEnv("TEMPDIR", dir) ||
       System::getEnv("TEMP_DIR", dir) ||
       System::getEnv("TMP", dir) ||
       System::getEnv("TMPDIR", dir) ||
       System::getEnv("TMP_DIR", dir)
     )
    return dir;
  else
    return homeDirectory();
}
