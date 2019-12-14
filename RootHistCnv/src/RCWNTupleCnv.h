/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef ROOTHISTCNV_RCWNTCNV_H
#define ROOTHISTCNV_RCWNTCNV_H 1

// Include files
#include "GaudiKernel/Converter.h"
#include "RNTupleCnv.h"
#include <string>

#include "TFile.h"
#include "TTree.h"

namespace RootHistCnv {

  /** @class RootHistCnv::RCWNTupleCnv RCWNTupleCnv.h
   *
   * Converter of Column-wise NTuple into ROOT format
   * @author Charles Leggett
   */

  class RCWNTupleCnv : public RNTupleCnv {
  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_ColumnWiseTuple; }
    /// Standard constructor
    RCWNTupleCnv( ISvcLocator* svc ) : RNTupleCnv( svc, classID() ) {}

  protected:
    /// Create the transient representation of an object.
    StatusCode load( TTree* tree, INTuple*& refpObject ) override;
    /// Book the N tuple
    StatusCode book( const std::string& desc, INTuple* pObject, TTree*& tree ) override;
    /// Write N tuple data
    StatusCode writeData( TTree* rtree, INTuple* pObject ) override;
    /// Read N tuple data
    StatusCode readData( TTree* rtree, INTuple* pObject, long ievt ) override;
  };
} // namespace RootHistCnv

#endif // ROOTHISTCNV_RCWNTCNV_H
