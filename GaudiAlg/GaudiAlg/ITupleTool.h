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
#ifndef GAUDIALG_ITUPLETOOL_H
#define GAUDIALG_ITUPLETOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IAlgTool.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleID.h"
// ============================================================================

// ============================================================================
/** @class ITupleTool ITupleTool.h GaudiTools/ITupleTool.h
 *
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */
// ============================================================================

class GAUDI_API ITupleTool : virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID( ITupleTool, 2, 0 );

  typedef Tuples::TupleID TupleID;
  typedef Tuples::Tuple   Tuple;

  /** get N-tuple object ( book on-demand ) with unique identifier
   *
   *  @code
   *
   *  ITupleTool* tool = .. ;
   *  Tuple tuple = tool->nTuple( 'My tuple' ) ;
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write()
   *
   *  @endcode
   *
   *  NTuple will be booked with automatically assigned numerical ID
   *
   *  @param title unique title for ntuple
   *  @param clid  class identifier
   *  @return ntuple object
   */
  virtual Tuple nTuple( const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const = 0;

  virtual Tuple nTuple( const TupleID& ID, const std::string& title = "",
                        const CLID& clid = CLID_ColumnWiseTuple ) const = 0;

  virtual Tuple evtCol( const std::string& title, const CLID& clid = CLID_RowWiseTuple ) const = 0;

  virtual Tuple evtCol( const TupleID& ID, const std::string& title = "",
                        const CLID& clid = CLID_RowWiseTuple ) const = 0;
};

// ============================================================================
#endif // GAUDIALG_ITUPLETOOL_H
