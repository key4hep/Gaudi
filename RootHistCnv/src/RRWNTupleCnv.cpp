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
