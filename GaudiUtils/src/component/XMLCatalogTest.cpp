#include "XMLFileCatalog.h"
#include <iostream>
#include <cstdio>
#include <ctime>
#include <sstream>

using namespace Gaudi;

extern "C" int testXMLFileCatalogWrite(int argc, char** argv)   {
  int nwrite = 10;
  std::string fname = "file:test.xml";
  if ( argc>1 ) fname = argv[1];
  if ( argc>2 ) nwrite = ::atol(argv[2]);
  XMLFileCatalog c(fname,0);
  c.init();
  std::vector<std::string> fids;
  c.getFID(fids);
  time_t start = time(0);
  for(size_t n=fids.size(), i=n; i<n+nwrite; ++i)  {
    std::ostringstream txt;
    if ( 0 == ((i-n)%10000) ) std::cout << i-n << std::endl;
    std::string fid = c.createFID();
    c.registerFID(fid);
    txt << "PFN1_Test_" << i << ".dat";
    c.registerPFN(fid,txt.str(),"ROOT");
    c.registerPFN(fid,txt.str(),"ROOT");
    txt.str("");
    txt << "PFN2_Test_" << i << ".dat";
    c.registerPFN(fid,txt.str(),"ROOT");
    txt.str("");
    txt << "PFN3_Test_" << i << ".dat";
    c.registerPFN(fid,txt.str(),"ROOT");
    txt.str("");
    txt << "lfn1_Test_" << i << ".dat";
    c.registerLFN(fid,txt.str());
    txt.str("");
    txt << "lfn2_Test_" << i << ".dat";
    c.registerLFN(fid,txt.str());
    txt.str("");
    txt << "lfn3_Test_" << i << ".dat";
    c.registerLFN(fid,txt.str());
    c.setMetaData(fid,"Name1","Value1");
    c.setMetaData(fid,"Name1","Value111");
    c.setMetaData(fid,"Name2","Value2");
    c.setMetaData(fid,"Name3","Value3");
  }
  time_t end = time(0)-start;
  std::cout << "Used " << end << " seconds."
            << " corresponding to " << float(end)/float(nwrite) << " entries/second."
            << std::endl;
  if ( c.dirty() )  {
    c.commit();
    time_t saved = time(0)-(start+end);
    std::cout << "Used " << saved << " seconds."
              << " corresponding to " << float(saved)/float(nwrite) << " entries/second."
              << std::endl;
  }
  else {
    std::cout << "Error: Catalog is not dirty after inserting records." << std::endl;
  }
  return 1;
}

extern "C" int testXMLFileCatalogRead(int argc, char** argv)  {
  std::vector<std::string> fids;
  std::string fname = "file:test.xml";
  if ( argc>1 ) fname = argv[1];
  bool prt = argc<2;
  time_t start = time(0);
  XMLFileCatalog c(fname,0);
  c.init();
  std::cout << "File loaded in " << time(0)-start << " seconds. " << std::endl;
  start = time(0);
  c.getFID(fids);
  std::cout << "FIDs scanned in " << time(0)-start << " seconds. " << std::endl;
  start = time(0);
  size_t mult = prt ? 1 : 10;
  std::cout << mult*fids.size() << std::endl;
  for(size_t i=0, tot=(mult*fids.size()); i<tot; ++i)  {
    size_t ent = i%fids.size();
    if ( ent == 0 ) std::cout << i << std::endl;
    std::string fid = fids[ent];
    XMLFileCatalog::Files pfn, lfn;
    XMLFileCatalog::Attributes attrs;
    c.getLFN(fid, lfn);
    for(size_t l1=0; l1<lfn.size(); ++l1)  {
      if ( !c.existsLFN(lfn[l1].first) )  {
        std::cout << "Error LFN existence of :" << lfn[l1].second << std::endl;
      }
      std::string f = c.lookupLFN(lfn[l1].first);
      if ( f != fid )  {
        std::cout << "Error LFN lookup of :" << lfn[l1].second << std::endl;
      }
    }
    c.getPFN(fid, pfn);
    for(size_t l2=0; l2<pfn.size(); ++l2)  {
      if ( !c.existsPFN(pfn[l2].first) )  {
        std::cout << "Error PFN existence of :" << pfn[l2].second << std::endl;
      }
      std::string f = c.lookupPFN(pfn[l2].first);
      if ( f != fid )  {
        std::cout << "Error PFN lookup of :" << pfn[l2].second << std::endl;
      }
    }
    c.getMetaData(fid, attrs);
    size_t n = lfn.size() > pfn.size() ? lfn.size() : pfn.size();
    n = n > attrs.size() ? n : attrs.size();
    if ( prt ) {
      std::cout << "FID:" << fid << std::endl;
      for(size_t j=0; j<n; ++j)  {
        if ( j < lfn.size() )  std::cout << lfn[j].first << "   ";
        else  std::cout << "                   ";
        if ( j < pfn.size() )  std::cout << pfn[j].first << "   ";
        else  std::cout << "                   ";
        if ( j < attrs.size() )  std::cout << attrs[j].first << " " << attrs[j].second << "   ";
        else  std::cout << "                  ";
        std::cout << std::endl;
      }
    }
  }
  time_t end = time(0)-start;
  std::cout << "Used " << end << " seconds (" << (long)fids.size()*mult << " entries)."
            << " Corresponding to " << float(end)/float(fids.size()*mult) << " entries/second."
            << std::endl;
  return 1;
}
