// $Id: RRWNTupleCnv.cpp,v 1.6 2006/01/10 20:12:02 hmd Exp $
#define ROOTHISTCNV_RRWNTUPLECNV_CPP

#define ALLOW_ALL_TYPES

// Compiler include files
#include "GaudiKernel/CnvFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/INTupleSvc.h"

#include "RRWNTupleCnv.h"
#include "TTree.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_NAMESPACE_CONVERTER_FACTORY(RootHistCnv,RRWNTupleCnv)

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::book(const std::string& desc,
                                           INTuple* nt,
                                           TTree*& rtree)
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), "RRWNTupleCnv");
  const INTuple::ItemContainer& itms = nt->items();
  // Book the tree
  rtree = new TTree(desc.c_str(),nt->title().c_str());
  // Add the branches
  for (size_t length = itms.size(), i = 0; i < length; i++ )    {
    const char* itm = itms[i]->name().c_str();
    std::string tag = itm;
    tag += rootVarType( itms[i]->type() );
    // add the branch
    log << MSG::INFO << "ID " << desc << ": added branch: "
        << itm << " / " << tag << endmsg;
    rtree->Branch(itm,const_cast<void*>(itms[i]->buffer()),tag.c_str());
  }
  log << MSG::INFO << "Booked TTree with ID:" << desc
      << " \"" << nt->title() << "\"" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::writeData(TTree* rtree, INTuple* nt)
//------------------------------------------------------------------------------
{
  if ( 0 != rtree )   {
    // Fill the tree;
    rtree->Fill();
    // Reset the NTuple
    nt->reset();
    return StatusCode::SUCCESS;
  }
  MsgStream log(msgSvc(), "RRWNTupleCnv");
  log << MSG::ERROR << "Attempt to write invalid N-tuple.";
  if ( nt != 0 ) log << nt->title();
  log << endmsg;
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::readData(TTree* /* tree */,
                                               INTuple* /* nt */,
                                               long /* ievt */)
//------------------------------------------------------------------------------
{
  return StatusCode::FAILURE;
}

//------------------------------------------------------------------------------
StatusCode RootHistCnv::RRWNTupleCnv::load(TTree* /* tree */,
                                           INTuple*& /* refpObject */)
//------------------------------------------------------------------------------
{
  return StatusCode::FAILURE;
}
