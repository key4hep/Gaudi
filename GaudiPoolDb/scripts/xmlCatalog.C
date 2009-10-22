#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"


void dumpCatalogHeader()  {
  printf("\n\n<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
  printf("<!-- Edited By XML Catalog generator -->\n");
  printf("<!DOCTYPE POOLFILECATALOG SYSTEM \"InMemory\">\n");
  printf("<POOLFILECATALOG>\n\n");
}

void dumpCatalogTrailer()  {
  printf("</POOLFILECATALOG>\n\n");
}

void dumpCatalogInfo(TFile* f)  {
  char text[16000];
  TBranch* b=0;
  int i;
  b = ((TTree*)f->Get("##Params"))->GetBranch("db_string");

  for(i=0,b->SetAddress(text); i < b->GetEntries();++i) {
    b->GetEvent(i);
    char* id1 = strstr(text,"[NAME=");
    char* id2 = strstr(text,"[VALUE=");
    if ( id1 && id2 )  {
      id1 += 6;
      id2 += 7;
      char* id11 = strstr(id1, "]");
      char* id22 = strstr(id2, "]");
      if ( id11 && id22 )  {
        *id11 = 0;
        *id22 = 0;
        if ( strcmp(id1,"FID") == 0 )  {
          printf("<File ID=\"%s\">\n", id2);
          printf("  <physical>\n");
          printf("    <pfn filetype=\"ROOT_All\" name=\"%s\"/>\n", f->GetName());
          printf("  </physical>\n");
          printf("  <logical/>\n");
          printf("</File>\n");
        }
      }
    }
  }
  delete f;
}

void xmlCatalog(char* fname) {
  TFile* f=TFile::Open(fname);
  dumpCatalogHeader();
  dumpCatalogInfo(f);
  dumpCatalogTrailer();
}


void createCatalog(char* files[])  {
  int count = 0, i;
  for (i=0; files[i]; ++i) count++;
  TFile** f = new TFile*[count];
  for (i=0; files[i]; ++i) f[i] = TFile::Open(files[i]);
  dumpCatalogHeader();
  for (int i=0; 0 != files[i]; ++i)  {
    dumpCatalogInfo(f[i]);
  }
  dumpCatalogTrailer();
}
