#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TClassStreamer.h"

class LinkMap;
static TClass* cle = 0;
static LinkMap* s_links = 0;
const char* contName = "";

namespace pool {
  class Token  {
    public:
      std::string db;         //!
      std::string container;  //!
      std::string clid;       //!
      int technology;         //!
      int a;                  //!
      int b;                  //!
      Token() : a(-1), b(-1), technology(-1)  {}
      virtual ~Token() {}
  };
}


class LinkMap {
public:
  std::vector<pool::Token*> m_links;
  LinkMap(TFile* f)  {
    int i;
    char text[1024];
    TBranch* b = ((TTree*)f->Get("##Links"))->GetBranch("db_string");
    for(i=0,b->SetAddress(text); i < b->GetEntries(); ++i) {
      b->GetEvent(i);
      pool::Token* t=new pool::Token();
      m_links.push_back(t);
      for(char* p1 = (char*)text; p1; p1 = ::strchr(++p1,'[')) {
        char* p2 = ::strchr(p1, '=');
        char* p3 = ::strchr(p1, ']');
        if ( p2 && p3 )   {
          char* val = p2+1;
          if ( ::strncmp("[DB=", p1, 4) == 0 )  {
            *p3 = 0;
            t->db = p1+4;
          }
          else if ( ::strncmp("[CNT=", p1, 5) == 0 )  {
            *p3 = 0;
            t->container = p1+4;
          }
          else if ( ::strncmp("[OID=", p1, 5) == 0 )  {
            *p3 = 0;
            ::sscanf(p1+5,"%08X,%08X",&t->a,&t->b);
          }
          else if ( ::strncmp("[CLID=", p1, 6) == 0 )  {
            *p3 = 0;
            t->clid = p1+6;
          }
          else if ( ::strncmp("[TECH=", p1, 6) == 0 )  {
            *p3 = 0;
            ::sscanf(p1+6,"%08X",&t->technology);
          }
          else    {
            *p3 = *p2 = 0;
          }
          *p3 = ']';
          *p2 = '=';
        }
      }
    }
  }
};

class PoolDbTokenWrap  {
public:
  pool::Token token; //||
  PoolDbTokenWrap  (){}
  virtual ~PoolDbTokenWrap() {}
};

class TokenStreamer : public TClassStreamer  {
public:
  TokenStreamer() : TClassStreamer(0) {}
  void operator()(TBuffer& b, void* objp)  {
     UInt_t start,count;
     Version_t vers;
     int vsn = ((TFile*)b.GetParent())->GetVersion();
     pool::Token* p = (pool::Token*)objp;
     if ( vsn < 40000 )  {
       b >> p->a;
     }
     else {
       vers = b.ReadVersion(&start, &count, cle);
     }
     b >> p->a;
     b >> p->b;
     if ( vsn < 40000 )  {
       b >> p->b;
     }
     if ( p->a - 2 >= 0 && p->a - 2 < s_links->m_links.size() )  {
       pool::Token* t = s_links->m_links[p->a - 2];
       p->db         = t->db;
       p->container  = t->container;
       p->clid       = t->clid;
       p->technology = t->technology;
     }
     else  {
       printf("Error streaming token: %X %d %X %X\n",vers,count,p->a,p->b);
     }
  }
};

int dumpTokens(const char* fname,const char* cnt)  {

  char txt[1024];
  sprintf(txt,".x dumpFile.C+(\"%s\")",fname);
  cle = gROOT->GetClass("pool::Token");
  cle->AdoptStreamer(new TokenStreamer());
  gROOT->ProcessLine(txt);

  TFile* f = TFile::Open(fname);
  s_links = new LinkMap(f);
  printf("File:        0x%08X  Version:%d \n", f, f->GetVersion());
  TTree* t = (TTree*)f->Get(cnt);
  TBranch* b = t->GetBranch("Address");
  printf("Branch:      %s\n", b->IsA()->GetName());
  printf("Branch:      %s\n", ((TBranchElement*)b)->GetTypeName());
  b->Print();
  PoolDbTokenWrap* token = new PoolDbTokenWrap();
  b->SetAddress(&token);
  for (int i = 0; i < b->GetEntries(); ++i )  {
    int nb = b->GetEntry(i);
    const pool::Token& t = token->token;
    printf("Recv[%d] DB:%-32s  CNT:%s CLID:%s TECH:%X OID:%X,%X \n",nb,
      t.db.c_str(), t.container.c_str(), t.clid.c_str(),t.technology, t.a,t.b);
  }
  f->Close();
  delete f;
  return 1;
}
int PoolToken()  {
  const char* fname;
  fname="U:/cern.ch/user/l/lhcbprod/w2/prodtest/work/7777/METC_00900000_00000823.root";
  fname="U:/cern.ch/user/l/lhcbprod/w2/prodtest/work/7777/ETC_00900000_00000823.root_1";
  fname="U:/cern.ch/user/l/lhcbprod/w2/prodtest/work/7777/ETC_00900000_00000823.root_2";
  contName = "<local>_TagCreator_1";
  contName = "<local>_Dir1_Dir2_Dir3_Collection";
  return dumpTokens(fname,contName);
}

