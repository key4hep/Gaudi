/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiAlg/Print.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "GaudiAlg/HistoID.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/INTuple.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/StatEntity.h"
#include <ctype.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <functional>
#include <string>
#include <vector>
// ============================================================================
/** @file
 *  Implementation file for functions for namespace GaudiAlg::Print
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-05
 */
// ============================================================================
namespace {
  /** local constant to indicate "invalid location"
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-05
   */
  static const std::string s_invalidLocation = "<UNKNOWN LOCATION>";
} // namespace
// ============================================================================
const std::string& GaudiAlg::Print::location( const AIDA::IHistogram* aida ) {
  if ( !aida ) { return s_invalidLocation; }
  const DataObject* object = dynamic_cast<const DataObject*>( aida );
  if ( !object ) { return s_invalidLocation; }
  IRegistry* registry = object->registry();
  if ( !registry ) { return s_invalidLocation; }
  return registry->identifier();
}
// ============================================================================
void GaudiAlg::Print1D::print( MsgStream& stream, const AIDA::IHistogram1D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print1D::toString( const AIDA::IHistogram1D* aida, const GaudiAlg::HistoID& ID ) {
  return fmt::format( " ID={:25}  {:47}  Ents/All={:>5}/{:<5}<X>/sX={:.5}/{:<.5}", ID.idAsString(),
                      fmt::format( "\"{:.45}\"", aida->title() ), ( aida->allEntries() - aida->extraEntries() ),
                      aida->allEntries(), aida->mean(), aida->rms() );
}
// ============================================================================
void GaudiAlg::Print2D::print( MsgStream& stream, const AIDA::IHistogram2D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print2D::toString( const AIDA::IHistogram2D* aida, const GaudiAlg::HistoID& ID ) {
  return fmt::format( " ID={:25}  {:47}  Ents/All={:>5}/{:<5}<X>/sX={:.5}/{:<.5},<Y>/sY={:.5}/{:<.5}", ID.idAsString(),
                      fmt::format( "\"{:.45}\"", aida->title() ), ( aida->allEntries() - aida->extraEntries() ),
                      aida->allEntries(), aida->meanX(), aida->rmsX(), aida->meanY(), aida->rmsY() );
}
// ============================================================================
void GaudiAlg::Print3D::print( MsgStream& stream, const AIDA::IHistogram3D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print3D::toString( const AIDA::IHistogram3D* aida, const GaudiAlg::HistoID& ID ) {
  return fmt::format(
      " ID={:25}  {:47}  Ents/All={:>5}/{:<5}<X>/sX={:.5}/{:<.5},<Y>/sY={:.5}/{:<.5},<Z>/sZ={:.5}/{:<.5}",
      ID.idAsString(), fmt::format( "\"{:.45}\"", aida->title() ), ( aida->allEntries() - aida->extraEntries() ),
      aida->allEntries(), aida->meanX(), aida->rmsX(), aida->meanY(), aida->rmsY(), aida->meanZ(), aida->rmsZ() );
}
// ============================================================================
void GaudiAlg::Print1DProf::print( MsgStream& stream, const AIDA::IProfile1D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print1DProf::toString( const AIDA::IProfile1D* aida, const GaudiAlg::HistoID& ID ) {
  return fmt::format( " ID={:25}  {:47}  Ents/All={:>5}/{:<5}<X>/sX={:.5}/{:<.5}", ID.idAsString(),
                      fmt::format( "\"{:.45}\"", aida->title() ), ( aida->allEntries() - aida->extraEntries() ),
                      aida->allEntries(), aida->mean(), aida->rms() );
}
// ============================================================================
void GaudiAlg::Print2DProf::print( MsgStream& stream, const AIDA::IProfile2D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print2DProf::toString( const AIDA::IProfile2D* aida, const GaudiAlg::HistoID& ID ) {
  return fmt::format( " ID={:25}  {:47}  Ents/All={:>5}/{:<5}<X>/sX={:.5}/{:<.5},<Y>/sY={:.5}/{:<.5}", ID.idAsString(),
                      fmt::format( "\"{:.45}\"", aida->title() ), ( aida->allEntries() - aida->extraEntries() ),
                      aida->allEntries(), aida->meanX(), aida->rmsX(), aida->meanY(), aida->rmsY() );
}
// ============================================================================
std::string GaudiAlg::PrintStat::print( const StatEntity& stat, const std::string& tag ) {
  std::ostringstream ost;
  stat.print( ost, true, tag );
  return ost.str();
}
// ============================================================================
std::string GaudiAlg::PrintTuple::print( const INTuple* tuple, const GaudiAlg::TupleID& ID ) {
  return fmt::format( " ID={:<14}{}", ID.idAsString(), print( tuple ) );
}
// ============================================================================
namespace {
  std::string _print( const INTuple::ItemContainer& items ) {
    std::string str;
    for ( const auto& item : items ) {
      if ( !item ) { continue; }
      if ( !str.empty() ) { str += ","; }
      str += item->name();
      if ( 0 != item->ndim() ) { str += '[' + std::to_string( item->ndim() ) + ']'; }
      if ( item->hasIndex() ) { str += "/V"; }
    }
    return str;
  }
} // namespace
// ============================================================================
std::string GaudiAlg::PrintTuple::print( const INTuple* tuple ) {
  return fmt::format( "Title={:41} #items={:<3}{{{:.81}}}", fmt::format( "\"{:.39}\"", tuple->title() ),
                      tuple->items().size(), _print( tuple->items() ) );
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
