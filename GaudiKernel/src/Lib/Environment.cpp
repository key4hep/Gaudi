#include "GaudiKernel/Environment.h"
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
              const char* res = ::getenv(env.c_str());
              std::string rep = (res) ? std::string(res) : i_resolve(env,--recursions);
              if ( rep.length() )  {
                std::string e(beg,c-beg+1);
                size_t idx=std::string::npos;
                while((idx=source.find(e)) != std::string::npos)  {
                  source.replace(idx, e.length(), rep);
                }
                return i_resolve(source, --recursions);
              }
              else  {
                // error: environment cannot be reolved....
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
  const char *h = 0;
  if ( 0 == (h = ::getenv("home")))  {
    h = ::getenv("HOME");
  }
  if ( h ) {
    home_dir = h;
  }
  else {
    // for Windows NT HOME might be defined as either $(HOMESHARE)/$(HOMEPATH)
    //                                         or     $(HOMEDRIVE)/$(HOMEPATH)
    h = ::getenv("HOMESHARE");
    if ( 0 == h )  {
      h = ::getenv("HOMEDRIVE");
    }
    if (h) {
      home_dir = h;
      h = ::getenv("HOMEPATH");
      if( h )  {
        home_dir += h;
      }
    }
  }
  return home_dir;
}

std::string System::tempDirectory()   {
   // Return a user configured or systemwide directory to create
   // temporary files in.
   const char *dir = ::getenv("TEMP");
   if (!dir)   dir = ::getenv("TEMPDIR");
   if (!dir)   dir = ::getenv("TEMP_DIR");
   if (!dir)   dir = ::getenv("TMP");
   if (!dir)   dir = ::getenv("TMPDIR");
   if (!dir)   dir = ::getenv("TMP_DIR");
   if (!dir) return homeDirectory();
   return dir;
}
