// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/PathResolver.cpp,v 1.7 2008/10/27 16:41:33 marcocle Exp $
#include "GaudiKernel/PathResolver.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <vector>

#include <sys/stat.h>

#ifdef _WIN32
#define stat _stat
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

namespace System {

typedef enum
  {
    PR_regular_file,
    PR_directory
  } PR_file_type;

typedef enum {
  PR_local,
  PR_recursive } PR_search_type;

static void PR_compress_path (std::string& dir)
{
#ifdef _WIN32
  static const char pattern[] = "\\..\\";
#else
  static const char pattern[] = "/../";
#endif

#ifdef _WIN32
  static const char file_separator = '\\';
  static const char double_file_separator[] = "\\\\";
#else
  static const char file_separator = '/';
  static const char double_file_separator[] = "//";
#endif

  if (dir.size () == 0) return;

    //
    // We first synchronize to using file_separator() in any case.
    //

  for (;;)
    {
      std::string::size_type pos;
      pos = dir.find (double_file_separator);
      if (pos == std::string::npos) break;
      dir.erase (pos, 1);
    }

  for (;;)
    {
      std::string::size_type pos1;
      std::string::size_type pos2;

      pos1 = dir.find (pattern);
      if (pos1 == std::string::npos) break;

        //
        // extract "aaaa/xxxx" from "aaaa/xxxx/../bbbb"
        //
      std::string p = dir.substr (0, pos1);
      
        //
        // Is "aaaa/xxxx" only made of "xxxx" ?
        // 
      pos2 = p.find_last_of (file_separator);
      
      if (pos2 == std::string::npos) break;
      
        //    01234567890123456
        //    aaaa/xxxx/../bbbb
        //        2    1   3
        //
        // erase the "/xxxx/../" pattern
        // result will be "aaaa/bbbb"
        //
      dir.erase (pos2, pos1 + 4 - pos2 - 1);
    }

    //if (dir[dir.size () - 1] == file_separator ()) dir.erase (dir.size () - 1);
}

static void PR_dirname (const std::string& file_name, std::string& result)
{
  std::string::size_type pos = file_name.find_last_of ('/');
  if (pos == std::string::npos)
    {
      pos = file_name.find_last_of ('\\');
    }

  if (pos == std::string::npos)
    {
      result = "";
    }
  else
    {
      result = file_name;
      result.erase (pos);
    }
}

static bool PR_absolute_path (const std::string& name)
{
  if (name.size () == 0) return (false);

  if ((name[0] == '/') ||
      (name[0] == '\\')) return (true);

  if (name.size () >= 2)
    {
      if (name[1] == ':')
        {
          return (true);
        }
    }
  return (false);
}

static void PR_basename (const std::string& file_name, std::string& result)
{
  std::string::size_type pos = file_name.find_last_of ('/');

  if (pos == std::string::npos)
    {
      pos = file_name.find_last_of ('\\');
    }

  if (pos == std::string::npos)
    {
      result = file_name;
    }
  else
    {
      result = file_name.substr (pos + 1);
    }
}

static bool PR_test_exist (const std::string& name, std::string& real_name, PR_file_type file_type)
{
  struct stat file_stat;
  int status;

  char buf[1024];
  strcpy (buf, name.c_str ());

#ifdef _WIN32
  static const char file_separator = '\\';
#else
  static const char file_separator = '/';
#endif

  real_name = name;
#ifndef _WIN32
  for (;;) {
    status = lstat (buf, &file_stat);
    if (status == 0) {
          if (S_ISLNK (file_stat.st_mode) != 0) {
            //std::cout << "#1 " << buf << " stat=" << std::oct << file_stat.st_mode << std::dec << std::endl;
            int n = readlink (buf, buf, sizeof (buf));
            if (n >= 0) buf[n] = 0;
            //std::cout << "test_exist> buf=" << buf << std::endl;
            if (PR_absolute_path (buf)) { 
          real_name = buf;
        }
            else {
          PR_dirname (real_name, real_name);
          real_name += file_separator;
          real_name += buf;
          PR_compress_path (real_name);
          strcpy (buf, real_name.c_str ());
        }
            //std::cout << "#2 " << real_name << std::endl;
            //break;
      }
      else { 
        break;
      }
    }
    else {
      break;
    }
  }
#endif
  status = stat (name.c_str (), &file_stat);

  if (status == 0) {
    if ((file_stat.st_mode & S_IFDIR) == 0) {
      return (file_type == PR_regular_file);
    }
    else {
      return (file_type == PR_directory);
    }
  }
  else {
    return (false);
  }
}

static void PR_scan_dir (const std::string& dir_name,
                         std::vector<std::string>& list)
{
#ifdef _WIN32
  static const char file_separator = '\\';
#else
  static const char file_separator = '/';
#endif

  static std::string dir_prefix;
  static std::string name_prefix;
  static std::string real_name;

  dir_prefix = dir_name;
  if (dir_name == "") dir_prefix = ".";

  //std::cout << "PR_scan_dir1> dir_name=" << dir_name << " dir_prefix=" << dir_prefix << std::endl;

  if (!PR_test_exist (dir_prefix, real_name, PR_directory)) {
    PR_dirname (dir_prefix, dir_prefix);
    PR_basename (dir_name, name_prefix);
  }

  bool need_filter = false;

  std::string::size_type wild_card;

  wild_card = name_prefix.find ('*');
  if (wild_card != std::string::npos) {
    name_prefix.erase (wild_card);

    if (name_prefix.size () > 0) {
      need_filter = true;
    }
  }

  list.clear();

#ifdef _WIN32

  long dir;
  struct _finddata_t entry;

  static std::string search;

  search = dir_prefix;
  search += file_separator;
  search += "*";

  dir = _findfirst (search.c_str (), &entry);
  if (dir > 0) {
    for (;;)   {
      if ((strcmp ((char*) entry.name, ".") != 0) &&
          (strcmp ((char*) entry.name, "..") != 0) &&
          (strncmp ((char*) entry.name, ".nfs", 4) != 0)) {
        const char* name = entry.name;

        if (need_filter && 
          (strncmp (name, name_prefix.c_str (), name_prefix.size ()) == 0)) continue;
        std::string name_entry;
        name_entry = dir_prefix;
        name_entry += file_separator;
        name_entry += name;

        list.push_back (name_entry);
      }

      int status = _findnext (dir, &entry);
      if (status != 0) {
        break;
      }
    }
    _findclose (dir);
  }
#else

  DIR* dir = opendir (dir_prefix.c_str ());

  struct dirent* entry;

  if (dir != 0) {
    while ((entry = readdir (dir)) != 0) {
      //if (entry->d_name[0] == '.') continue;
      if (!strcmp ((char*) entry->d_name, ".")) continue;
      if (!strcmp ((char*) entry->d_name, "..")) continue;
      if (!strncmp ((char*) entry->d_name, ".nfs", 4)) continue;

      const char* name = entry->d_name;

      //std::cout << "scan_dir4> name=" << name << std::endl;

      if (need_filter &&
          (strncmp (name, name_prefix.c_str (), name_prefix.size ()) != 0)) continue;

      std::string name_entry;

      name_entry = dir_prefix;
      name_entry += file_separator;
      name_entry += name;

      list.push_back (name_entry);
    }
    closedir (dir);
  }
#endif
}


static bool PR_find (const std::string& search_base, 
                     const std::string& logical_file_prefix, 
                     const std::string& logical_file_name, 
                     PR_file_type file_type,
                     PathResolver::SearchType search_type,
                     std::string& result)
{
  static int level = 0;

#ifdef _WIN32
  static const char file_separator = '\\';
#else
  static const char file_separator = '/';
#endif

  std::string file_path = "";
  std::string real_name = "";

  bool found = false;

  //PR_indent (level); std::cout << "PR_find> sb=" << search_base << " lfp=" << logical_file_prefix << " lfn=" << logical_file_name << std::endl;

  if (search_base != "")
    {
      file_path = search_base;
      file_path += file_separator;
    }
  else
    {
      file_path = "";
    }

  file_path += logical_file_name;

  //PR_indent (level); std::cout << "PR_find> test1 file_path=" << file_path << std::endl;

  result = file_path;
  if (PR_test_exist (file_path, result, file_type))
    {
      found = true;
    }

  if (!found && (logical_file_prefix != ""))
    {
      if (search_base != "")
        {
          file_path = search_base;
          file_path += file_separator;
        }
      else
        {
          file_path = "";
        }

      file_path += logical_file_prefix;
      file_path += file_separator;
      file_path += logical_file_name;

      //PR_indent (level); std::cout << "PR_find> test2 file_path=" << file_path << std::endl;

      result = file_path;
      if (PR_test_exist (file_path, result, file_type))
        {
          found = true;
        }
    }

  //PR_indent (level); std::cout << "PR_find> test3 found=" << found << " search_type=" << search_type << std::endl;

  if (!found && (search_type == PathResolver::RecursiveSearch))
    {
      std::string dir_name = "";
      std::string file_name = "";
      std::vector<std::string> list;

      PR_scan_dir (search_base, list);

      std::vector<std::string>::iterator it;

      for (it = list.begin (); it != list.end (); ++it)
        {
          const std::string& d = *it;

          if (PR_test_exist (d, file_path, PR_directory))
            {
              //PR_indent (level); std::cout << "d=" << d << std::endl;

              level++;
              bool s = PR_find (d, logical_file_prefix, logical_file_name, file_type, search_type, result);
              level--;

              if (s)
                {
                  //PR_indent (level); std::cout << "PR_find> result=" << result << std::endl;
                  found = true;
                  break;
                }
            }
        }
    }

  return (found);
}

static bool PR_find_from_list (const std::string& logical_file_name,
                               const std::string& search_list,
                               PR_file_type file_type,
                               PathResolver::SearchType search_type,
                               std::string& result)
{
#ifdef _WIN32
  static const char path_separator = ';';
#else
  static const char path_separator = ':';
#endif

  std::string::size_type pos = 0;

  std::string file_name = "";
  std::string file_prefix = "";

  PR_basename (logical_file_name, file_name);
  PR_dirname (logical_file_name, file_prefix);

  std::string real_name = "";

  bool found = false;

  if (PR_find ("", file_prefix, file_name, file_type, search_type, result))
    {
      found = true;
    }

  if (!found)
    {
      for (int i = 0;;i++)
        {
          bool ending = false;

          std::string::size_type next = search_list.find (path_separator, pos);

          std::string path = search_list.substr (pos, next - pos);

          if (next == std::string::npos)
            {
              path = search_list.substr (pos);
              ending = true;
            }
          else
            {
              path = search_list.substr (pos, next - pos);
              pos = next + 1;
            }
          
          //std::cout << "path[" << i << "]=" << path << std::endl;
          
          if (PR_find (path, file_prefix, file_name, file_type, search_type, result))
            {
              found = true;
              break;
            }
          
          if (ending) break;
        }
    }

  return (found);
}

std::string PathResolver::find_file (const std::string& logical_file_name,
                                     const std::string& search_path,
                                     SearchType search_type)
{
  const char* path_env = ::getenv (search_path.c_str ());

  std::string path_list;

  if (path_env != 0)
    {
      path_list = path_env;
    }

  return (find_file_from_list (logical_file_name, path_list, search_type));
}

std::string PathResolver::find_file_from_list (const std::string& logical_file_name,
                                               const std::string& search_list,
                                               SearchType search_type)
{
  std::string result;

  if (!PR_find_from_list (logical_file_name, search_list, PR_regular_file, search_type, result))
    {
      result = "";
    }

  return (result);
}

std::string PathResolver::find_directory (const std::string& logical_file_name,
                                          const std::string& search_path,
                                          SearchType search_type)
{
  const char* path_env = ::getenv (search_path.c_str ());

  std::string path_list;

  if (path_env != 0)
    {
      path_list = path_env;
    }

  return (find_directory_from_list (logical_file_name, path_list, search_type));
}

std::string PathResolver::find_directory_from_list (const std::string& logical_file_name,
                                                    const std::string& search_list,
                                                    SearchType search_type)
{
  std::string result;

  if (!PR_find_from_list (logical_file_name, search_list, PR_directory, search_type, result))
    {
      result = "";
    }

  return (result);
}

PathResolver::SearchPathStatus PathResolver::check_search_path (const std::string& search_path)
{
  const char* path_env = ::getenv (search_path.c_str ());

  if (path_env == 0) return (EnvironmentVariableUndefined);

#ifdef _WIN32
  static const char path_separator = ';';
#else
  static const char path_separator = ':';
#endif

  std::string path_list (path_env);

  std::string::size_type pos = 0;

  for (int i = 0;;i++)
    {
      bool ending = false;

      std::string::size_type next = path_list.find (path_separator, pos);

      std::string path = path_list.substr (pos, next - pos);

      if (next == std::string::npos)
        {
          path = path_list.substr (pos);
          ending = true;
        }
      else
        {
          path = path_list.substr (pos, next - pos);
          pos = next + 1;
        }

      std::string real_name = "";

      if (!PR_test_exist (path, real_name, PR_directory))
        {
          return (UnknownDirectory);
        }

      if (ending) break;
    }

  return (Ok);
}


PathResolver::SearchPathStatus PathResolverCheckSearchPath (const std::string& search_path)
{
  return PathResolver::check_search_path (search_path);
}

std::string PathResolverFindDirectory (const std::string& logical_file_name,
                                        const std::string& search_path)
{
  return PathResolver::find_directory (logical_file_name, search_path);
}

std::string PathResolverFindDirectoryFromList (const std::string& logical_file_name,
                                                const std::string& search_list)
{
  return PathResolver::find_directory_from_list (logical_file_name, search_list);
}

std::string PathResolverFindFile (const std::string& logical_file_name,
                                  const std::string& search_path)
{
  return PathResolver::find_file (logical_file_name, search_path);
}

std::string PathResolverFindFileFromList (const std::string& logical_file_name,
                                          const std::string& search_list)
{
  return PathResolver::find_file_from_list (logical_file_name, search_list);
}

std::string PathResolverFindXMLFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "XMLPATH");
}

std::string PathResolverFindDataFile (const std::string& logical_file_name)
{
  return PathResolver::find_file (logical_file_name, "DATAPATH");
}

}  // System namespace
