#define ALLOW_ALL_TYPES

// Compiler include files
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"

#include "RRWNTupleCnv.h"
#include "TTree.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_CONVERTER( RootHistCnv::RRWNTupleCnv )

// FIXME - leak sanitizers generate a number of warnings here
//
// clang-format off
// Indirect leak of 160 byte(s) in 2 object(s) allocated from:
//    #0 0x7ff846fbbec8 in operator new[](unsigned long) /afs/cern.ch/cms/CAF/CMSCOMM/COMM_ECAL/dkonst/GCC/build/contrib/gcc-8.2.0/src/gcc/8.2.0/libsanitizer/lsan/lsan_interceptors.cc:231
//    #1 0x7ff8336e7653 in TBranch::Init(char const*, char const*, int) /mnt/build/jenkins/workspace/lcg_release_tar/BUILDTYPE/Debug/COMPILER/gcc8binutils/LABEL/centos7/build/projects/ROOT-6.18.00/src/ROOT/6.18.00/tree/tree/src/TBranch.cxx:309
//    #2 0x7ff8336e7039 in TBranch::TBranch(TTree*, char const*, void*, char const*, int, int) /mnt/build/jenkins/workspace/lcg_release_tar/BUILDTYPE/Debug/COMPILER/gcc8binutils/LABEL/centos7/build/projects/ROOT-6.18.00/src/ROOT/6.18.00/tree/tree/src/TBranch.cxx:239
//    #3 0x7ff833773ceb in TTree::Branch(char const*, void*, char const*, int) /mnt/build/jenkins/workspace/lcg_release_tar/BUILDTYPE/Debug/COMPILER/gcc8binutils/LABEL/centos7/build/projects/ROOT-6.18.00/src/ROOT/6.18.00/tree/tree/src/TTree.cxx:1893
//    #4 0x7ff831763748 in RootHistCnv::RRWNTupleCnv::book(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, INTuple*, TTree*&) ../RootHistCnv/src/RRWNTupleCnv.cpp:27
// clang-format on
//
// These leaks are currently suppressed in Gaudi/job/Gaudi-LSan.supp - remove entry there to reactivate

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::book( const std::string& desc, INTuple* nt, TTree*& rtree )
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RRWNTupleCnv" );
  // Book the tree
  rtree = new TTree( desc.c_str(), nt->title().c_str() );
  // Add the branches
  for ( const auto& i : nt->items() ) {
    std::string tag = i->name() + rootVarType( i->type() );
    // add the branch
    log << MSG::INFO << "ID " << desc << ": added branch: " << i->name() << " / " << tag << endmsg;
    rtree->Branch( i->name().c_str(), const_cast<void*>( i->buffer() ), tag.c_str() );
  }
  log << MSG::INFO << "Booked TTree with ID:" << desc << " \"" << nt->title() << "\"" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::writeData( TTree* rtree, INTuple* nt )
//------------------------------------------------------------------------------
{
  if ( rtree ) {
    // Fill the tree;
    rtree->Fill();
    // Reset the NTuple
    nt->reset();
    return StatusCode::SUCCESS;
  }
  MsgStream log( msgSvc(), "RRWNTupleCnv" );
  log << MSG::ERROR << "Attempt to write invalid N-tuple.";
  if ( nt ) log << nt->title();
  log << endmsg;
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::readData( TTree* /* tree */, INTuple* /* nt */, long /* ievt */ )
//------------------------------------------------------------------------------
{
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::load( TTree* /* tree */, INTuple*& /* refpObject */ )
//------------------------------------------------------------------------------
{
  return StatusCode::FAILURE;
}
