#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include <cstring>
#include <fstream>

void printText(const char* text)  {
    printf("%s\n\n",text);
}

const char* printLink(const char* text)  {
  printf("Link:%s\n",text);
  const char* db = 0;

  for(char* p1 = (char*)text; p1; p1 = ::strchr(++p1,'[')) {
    char* p2 = ::strchr(p1, '=');
    char* p3 = ::strchr(p1, ']');
    if ( p2 && p3 )   {
      char* val = p2+1;
      if ( ::strncmp("[DB=", p1, 4) == 0 )  {
        *p3 = 0;
        printf("Link:  Dbase=%s  ", db=p1+4);
      }
      else if ( ::strncmp("[CNT=", p1, 5) == 0 )  {
        *p3 = 0;
        printf("Container=%s \n", p1+5);
      }
      else if ( ::strncmp("[OID=", p1, 5) == 0 )  {
        *p3 = 0;
        printf(" OID=%s", p1+5);
      }
      else if ( ::strncmp("[CLID=", p1, 6) == 0 )  {
        *p3 = 0;
        printf("       CLID=%s", p1+6);
      }
      else if ( ::strncmp("[TECH=", p1, 6) == 0 )  {
        *p3 = 0;
        printf(" Technology=%s", p1+6);
      }
      else    {
        *p3 = *p2 = 0;
      }
      *p3 = ']';
      *p2 = '=';
    }
  }
  printf("\n");
  return db;
}

typedef std::map<std::string,std::string> GuidMap;
GuidMap guidmap;

void fixGUIDs(const char* fname, const char* fguids="guids.txt") {
  TFile* f=TFile::Open(fname,"UPDATE");
  char text[16000];
  std::ifstream input(fguids,ios_base::in);
  if ( input.is_open() )  {
    while ( input.good() )  {
      text[0] = 0;
      input.getline(text,sizeof(text));
      if ( input.good() && strlen(text)>0 )  {
        text[36] = 0;
        guidmap[text] = text+37;
        printf("GUID: '%s' -> '%s'\n",text,text+38);
      }
    }
  }
  f->ls();
  printf("\n\n================= Links =================\n");
  TTree* tree_old = (TTree*)f->Get("##Links");
  tree_old->SetName("OldLinks");
  tree_old->Write();
  TBranch* branch_old = tree_old->GetBranch("db_string");
  branch_old->SetAddress(text);
  f->Delete("##Links;*");
  f->ls();
  int nbytes = 0;
  TTree*   tree_new   = new TTree("##Links","##Links");
  TBranch* branch_new = tree_new->Branch("db_string",(void*)text,"db_string/C");
  branch_new->SetAddress(text);
  for(int i=0; i < branch_old->GetEntries(); ++i) {
    branch_old->GetEvent(i);
    const char* link = printLink(text);
    if ( link )  {
      TString s2 = text+4;
      s2.ToUpper();
      for(GuidMap::const_iterator i=guidmap.begin();i!=guidmap.end();++i)  {
        TString s1 = (*i).first.c_str();
        s1.ToUpper();
        if ( strncasecmp(s1,s2,36)==0 )  {
          memcpy(text+4,(*i).second.c_str(),36);
          break;
        }
      }
      printf("New link:%s\n",text);
    }
    int nb = tree_new->Fill();
    if ( nb == 0 )  printf("Error writing tree data:%s\n",tree_old->GetName());
    nbytes += nb;
  }
  printf("\n\n================= Links:%d bytes\n",nbytes);
  tree_new->Write();
  f->ls();

  f->Close();
  delete f;
}

