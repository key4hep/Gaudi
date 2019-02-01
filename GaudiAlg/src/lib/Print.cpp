// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <ctype.h>
#include <functional>
#include <string>
#include <vector>
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/INTuple.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/HistoID.h"
#include "GaudiAlg/Print.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
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
  boost::format fmt( " ID=%|-25|%|30t| \"%|.45s|\" %|79t| Ents/All=%|5|/%|-5|<X>/sX=%|.5|/%|-.5|" );
  fmt % ID.idAsString() % aida->title();
  fmt % ( aida->allEntries() - aida->extraEntries() ) % aida->allEntries();
  fmt % aida->mean() % aida->rms();
  //
  return fmt.str();
}
// ============================================================================
void GaudiAlg::Print2D::print( MsgStream& stream, const AIDA::IHistogram2D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print2D::toString( const AIDA::IHistogram2D* aida, const GaudiAlg::HistoID& ID ) {
  boost::format fmt( " ID=%|-25|%|30t| \"%|.45s|\" %|79t| Ents/All=%|5|/%|-5|<X>/sX=%|.5|/%|-.5|,<Y>/sY=%|.5|/%|-.5|" );
  fmt % ID.idAsString() % aida->title();
  fmt % ( aida->allEntries() - aida->extraEntries() ) % aida->allEntries();
  fmt % aida->meanX() % aida->rmsX();
  fmt % aida->meanY() % aida->rmsY();
  //
  return fmt.str();
}
// ============================================================================
void GaudiAlg::Print3D::print( MsgStream& stream, const AIDA::IHistogram3D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print3D::toString( const AIDA::IHistogram3D* aida, const GaudiAlg::HistoID& ID ) {
  boost::format fmt( " ID=%|-25|%|30t| \"%|.45s|\" %|79t| "
                     "Ents/All=%|5|/%|-5|<X>/sX=%|.5|/%|-.5|,<Y>/sY=%|.5|/%|-.5|,<Z>/sZ=%|.5|/%|-.5|" );
  fmt % ID.idAsString() % aida->title();
  fmt % ( aida->allEntries() - aida->extraEntries() ) % aida->allEntries();
  fmt % aida->meanX() % aida->rmsX();
  fmt % aida->meanY() % aida->rmsY();
  fmt % aida->meanZ() % aida->rmsZ();
  //
  return fmt.str();
}
// ============================================================================
void GaudiAlg::Print1DProf::print( MsgStream& stream, const AIDA::IProfile1D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print1DProf::toString( const AIDA::IProfile1D* aida, const GaudiAlg::HistoID& ID ) {
  boost::format fmt( " ID=%|-25|%|30t| \"%|.55s|\" %|79t| Ents/All=%|5|/%|-5|<X>/sX=%|.5|/%|-.5|" );
  fmt % ID.idAsString() % aida->title();
  fmt % ( aida->allEntries() - aida->extraEntries() ) % aida->allEntries();
  fmt % aida->mean() % aida->rms();
  //
  return fmt.str();
}
// ============================================================================
void GaudiAlg::Print2DProf::print( MsgStream& stream, const AIDA::IProfile2D* aida, const GaudiAlg::HistoID& ID ) {
  stream << toString( aida, ID ) << endmsg;
}
// ============================================================================
std::string GaudiAlg::Print2DProf::toString( const AIDA::IProfile2D* aida, const GaudiAlg::HistoID& ID ) {
  boost::format fmt( " ID=%|-25|%|30t| \"%|.55s|\" %|79t| Ents/All=%|5|/%|-5|<X>/sX=%|.5|/%|-.5|,<Y>/sY=%|.5|/%|-.5|" );
  fmt % ID.idAsString() % aida->title();
  fmt % ( aida->allEntries() - aida->extraEntries() ) % aida->allEntries();
  fmt % aida->meanX() % aida->rmsX();
  fmt % aida->meanY() % aida->rmsY();
  //
  return fmt.str();
}
// ============================================================================
std::string GaudiAlg::PrintStat::print( const StatEntity& stat, const std::string& tag ) {
  std::ostringstream ost;
  stat.print( ost, true, tag );
  return ost.str();
}
// ============================================================================
std::string GaudiAlg::PrintTuple::print( const INTuple* tuple, const GaudiAlg::TupleID& ID ) {
  boost::format fmt( " ID=%|-12|%|18t|%|-s|" );
  fmt % ID.idAsString() % print( tuple );
  return fmt.str();
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
  boost::format fmt( "Title=\"%|.39s|\" %|48t|#items=%|-3|%|50t|{%|.81s|}" );
  fmt % tuple->title();
  fmt % tuple->items().size();
  fmt % _print( tuple->items() );
  return fmt.str();
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
