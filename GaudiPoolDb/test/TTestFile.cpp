#include "TROOT.h"
#include "TFile.h"
#include "TSystem.h"
#include <iostream>
#include <string>

#define PLUGIN_NAME "^gfal"

struct TFileName  {
  std::string m_name;
  TFileName(const char *n) {
    std::string u(n);
    if ( u == "gfal:guid:547963F7-7921-DB11-815A-5B0145A5FC2D" )  {
      m_name = "RootDst0.root";
    }
    else if ( u == "gfal:guid:C61526F9-7921-DB11-815A-5B0145A5FC2D" )  {
      m_name = "RootMini0.root";
    }
    else {
      m_name = u.substr(u.rfind("/")+1);
    }
  }
  const char* c_str() const      {    return m_name.c_str();  }
  operator const char* () const  {    return m_name.c_str();  }
};

struct TGFALFile : public TFile {
  TGFALFile() : TFile() {}
  TGFALFile(const char *url, Option_t *o,const char *t, Int_t c)
  : TFile(TFileName(url),o,t,c)  {
    std::cout << "Created TGFAL File (mockup):" << GetName() << std::endl;
  }
  virtual ~TGFALFile() { 
    std::cout << "Destroyed TGFAL File (mockup):" << GetName() << std::endl;  
  }
};

struct TGFALSystem : public TSystem {
  const char* fname(const char* n)  {
    const char* nn = n + strlen(n);
    while(*nn != '/' && nn>=n) --nn;
    return ++nn;
  }
  TGFALSystem() : TSystem ("-gfal", "TFal Helper System") {
    // name must start with '-' to bypass the TSystem singleton check
    SetName("gfal");
  }
  virtual ~TGFALSystem() { }
  Int_t       MakeDirectory(const char *name)
  { return gSystem->MakeDirectory(fname(name)); }
  void       *OpenDirectory(const char *name)
  { return gSystem->OpenDirectory(fname(name)); }
  void        FreeDirectory(void * /* dirp */) {}
  const char *GetDirEntry(void * /* dirp */)   { return 0; }
  int         GetPathInfo(const char *name, Long_t *id, Long64_t *s, Long_t *f, Long_t *m)
  { return gSystem->GetPathInfo(fname(name),id,s,f,m); }
  int         GetPathInfo(const char *path, FileStat_t &buf)
  { return TSystem::GetPathInfo(path, buf);                   }
  Bool_t      AccessPathName(const char *name, EAccessMode mode)
  { return gSystem->AccessPathName(fname(name),mode);               }

};

#include "Reflex/Builder/ClassBuilder.h"
#include "TPluginManager.h"
using namespace ROOT::Reflex;

typedef const std::vector<void*>& _Args;
template <class T>
static void ctor(void* retaddr, void* mem, _Args, void*) {
  if (retaddr) *(void**)retaddr = ::new(mem) T();
  else ::new(mem) T();
}
template <class T>
static void dtor(void*, void* m, _Args, void*) {
  ((T*)m)->~T();
}
static void TGFALFile_ctor(void* retaddr, void* mem, _Args args, void*){
  if (retaddr) *(void**)retaddr = ::new(mem) TGFALFile((const char*)args[0],(const char*)args[1],(const char*)args[2],*(int*)args[3]);
  else ::new(mem) TGFALFile((const char*)args[0],(const char*)args[1],(const char*)args[2],*(int*)args[3]);
}

namespace { 
  struct __init  {
    __init()  {
      ClassBuilderT<TGFALFile>()
        .AddBase<TFile>()
        .AddFunctionMember<void(void)>("TGFALFile",  ctor<TGFALFile>, 0, 0, PUBLIC | CONSTRUCTOR )
        .AddFunctionMember<void(void)>("~TGFALFile", dtor<TGFALFile>, 0, 0, PUBLIC | DESTRUCTOR )
        .AddFunctionMember<void(const char*,const char*,const char*,int)>("TGFALFile", TGFALFile_ctor, 0, 0, PUBLIC | CONSTRUCTOR );
      gROOT->GetPluginManager()->AddHandler("TFile",PLUGIN_NAME,"TGFALFile", "testGaudiPoolDb", "TGFALFile(const char*,const char*,const char*,int)");

      ClassBuilderT<TGFALSystem>()
        .AddBase<TSystem>()
        .AddFunctionMember<void(void)>("TGFALSystem",  ctor<TGFALSystem>, 0, 0, PUBLIC | CONSTRUCTOR )
        .AddFunctionMember<void(void)>("~TGFALSystem", dtor<TGFALSystem>, 0, 0, PUBLIC | DESTRUCTOR );
      gROOT->GetPluginManager()->AddHandler("TSystem",PLUGIN_NAME,"TGFALSystem", "testGaudiPoolDb", "TGFALSystem()");
    }
  };
  static __init _i;
}
void* __init_TGFALFile ()  {return &_i;}
