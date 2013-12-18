#define ROOTHISTCNV_ROOTDIRFCN_CPP

#include "GaudiKernel/Kernel.h"
#include "RootDirFcn.h"

#include <string>
#include <list>
#include "TFile.h"
#include "TDirectory.h"



namespace RootHistCnv {

//-----------------------------------------------------------------------------
bool RootCd(const std::string& full)
//-----------------------------------------------------------------------------
{
  int p,i=1;
  std::string cur,sdir;

  gDirectory->cd("/");
  while ( (p = full.find("/",i)) != -1) {
    sdir = full.substr(i,p-i);
    if (! gDirectory->GetKey(sdir.c_str()) ) {
      return false;
    }
    gDirectory->cd(sdir.c_str());

    i = p+1;
  }
  gDirectory->cd( full.substr(i,full.length()-i).c_str() );

  return true;

}


//-----------------------------------------------------------------------------
bool RootMkdir(const std::string& full)
//-----------------------------------------------------------------------------
{

  int p,i;
  std::string fil,cur,s;
  TDirectory *gDir;

  gDir = gDirectory;

  std::list<std::string> lpath;
  i = 1;

  if ( (p=full.find(":",0)) != -1 ) {
    fil = full.substr(0,p);
    i = p+1;
    fil += ":/";
    gDirectory->cd(fil.c_str());
  }

  while ( (p = full.find("/",i)) != -1) {
    s = full.substr(i,p-i);
    lpath.push_back(s);
    i = p+1;
  }
  lpath.push_back( full.substr(i,full.length()-i) );

  if ( full.substr(0,1) == "/") {
    gDirectory->cd("/");
  }

  std::list<std::string>::const_iterator litr;
  for(litr=lpath.begin(); litr!=lpath.end(); ++litr) {
    cur = *litr;
    if (! gDirectory->GetKey(litr->c_str()) ) {
      gDirectory->mkdir(litr->c_str());
    }
    gDirectory->cd(litr->c_str());
  }

  gDirectory = gDir;

  return true;
}

//-----------------------------------------------------------------------------
std::string RootPwd()
//-----------------------------------------------------------------------------
{
  std::string dir = gDirectory->GetPath();

  return (dir);
}

//-----------------------------------------------------------------------------
bool RootTrimLeadingDir(std::string &full, std::string dir)
//-----------------------------------------------------------------------------
{

  if (dir.substr(0,1) != "/") {
    dir.insert(0,"/");
  }

  if (dir.substr(dir.length()-1,1) != "/") {
    dir += "/";
  }

  long ll = full.find(dir);
  if (ll != 0) {
    return false;
  }

  full.erase(0,dir.length()-1);

  return true;

}


} // RootHistCnv namespace
