/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// PartProp
// ============================================================================
#include "GaudiPartProp/CC.h"
#include "GaudiPartProp/IParticlePropertySvc.h"
#include "GaudiPartProp/NodesPIDs.h"
#include "GaudiPartProp/ParticleProperty.h"
#include "GaudiPartProp/Symbols.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/algorithm/string.hpp"
// ============================================================================
/** @file
 *  Implementation file for functions form the file GaudiPartProp/Symbols.h
 *
 *  Many thanks to Antonio PELLEGRINO for the kind help with regex expressions
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-05-07
 */
// ============================================================================
namespace {
  // ==========================================================================
  /** @var s_EMPTY
   *  invalid empty string
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-05-07
   */
  const std::string s_EMPTY = "";
  // ==========================================================================
} // namespace
// ============================================================================
Decays::Symbols& Decays::Symbols::instance() {
  static Decays::Symbols s_symbols;
  return s_symbols;
}
// ============================================================================
// constructor
// ============================================================================
Decays::Symbols::Symbols() {
  addSymbol( "X", Decays::Nodes::Any(), "Any particle" );
  //
  addSymbol( "Hadron", Decays::Nodes::Hadron(), "Any hadron" );
  addSymbol( "Meson", Decays::Nodes::Meson(), "Any meson" );
  addSymbol( "Baryon", Decays::Nodes::Baryon(), "Any baryon" );
  addSymbol( "Nucleus", Decays::Nodes::Nucleus(), "Any nucleus" );
  addSymbol( "Lepton", Decays::Nodes::Lepton(), "Any lepton" );
  addSymbol( "l", Decays::Nodes::Ell(), "Any charged lepton" );
  addSymbol( "l+", Decays::Nodes::EllPlus(), "Any positive lepton", "l-" );
  addSymbol( "l-", Decays::Nodes::EllMinus(), "Any negative lepton", "l+" );
  addSymbol( "Nu", Decays::Nodes::Nu(), "Any neutral lepton" );
  addSymbol( "Neutrino", Decays::Nodes::Nu(), "Any neutral lepton" );
  //
  addSymbol( "X0", Decays::Nodes::Neutral(), "Any neutral particle" );
  addSymbol( "Xq", Decays::Nodes::Charged(), "Any charged particle" );
  addSymbol( "X+", Decays::Nodes::Positive(), "Any positive particle", "X-" );
  addSymbol( "X-", Decays::Nodes::Negative(), "Any negative particle", "X+" );
  //
  addSymbol( "ID+", Decays::Nodes::PosID(), "Any particle with positive ID", "ID-" );
  addSymbol( "ID-", Decays::Nodes::NegID(), "Any particle with negative ID", "ID+" );
  //
  addSymbol( "Xd", Decays::Nodes::HasQuark( Gaudi::ParticleID::down ), "Any particle with d-quark" );
  addSymbol( "Xu", Decays::Nodes::HasQuark( Gaudi::ParticleID::up ), "Any particle with u-quark" );
  addSymbol( "Xs", Decays::Nodes::HasQuark( Gaudi::ParticleID::strange ), "Any particle with s-quark" );
  addSymbol( "Xc", Decays::Nodes::HasQuark( Gaudi::ParticleID::charm ), "Any particle with c-quark" );
  addSymbol( "Xb", Decays::Nodes::HasQuark( Gaudi::ParticleID::bottom ), "Any particle with b-quark" );
  addSymbol( "Xt", Decays::Nodes::HasQuark( Gaudi::ParticleID::top ), "Any particle with t-quark" );
  //
  addSymbol( "Down", Decays::Nodes::HasQuark( Gaudi::ParticleID::down ), "Any particle with d-quark" );
  addSymbol( "Up", Decays::Nodes::HasQuark( Gaudi::ParticleID::up ), "Any particle with u-quark" );
  addSymbol( "Strange", Decays::Nodes::HasQuark( Gaudi::ParticleID::strange ), "Any particle with s-quark" );
  addSymbol( "Charm", Decays::Nodes::HasQuark( Gaudi::ParticleID::charm ), "Any particle with c-quark" );
  addSymbol( "Bottom", Decays::Nodes::HasQuark( Gaudi::ParticleID::bottom ), "Any particle with b-quark" );
  addSymbol( "Beauty", Decays::Nodes::HasQuark( Gaudi::ParticleID::bottom ), "Any particle with b-quark" );
  addSymbol( "Top", Decays::Nodes::HasQuark( Gaudi::ParticleID::top ), "Any particle with t-quark" );
  //
  addSymbol( "Scalar", Decays::Nodes::JSpin( 1 ), "Any scalar particle j=0" );
  addSymbol( "Spinor", Decays::Nodes::JSpin( 2 ), "Any spinor particle j=1/2" );
  addSymbol( "OneHalf", Decays::Nodes::JSpin( 2 ), "Any spinor particle j=1/2" );
  addSymbol( "Vector", Decays::Nodes::JSpin( 3 ), "Any vector particle j=1" );
  addSymbol( "ThreeHalf", Decays::Nodes::JSpin( 4 ), "Any particle with spin j=3/2" );
  addSymbol( "Tensor", Decays::Nodes::JSpin( 5 ), "Any tensor particle j=2" );
  addSymbol( "FiveHalf", Decays::Nodes::JSpin( 6 ), "Any particle with spin j=5/2" );
  //
  addSymbol( "ShortLived", Decays::Nodes::ShortLived_(), "Any short-ilved particle" );
  addSymbol( "LongLived", Decays::Nodes::LongLived_(), "Any long-lived particle" );
  addSymbol( "Stable", Decays::Nodes::Stable(), "Any 'stable' particle" );
  addSymbol( "StableCharged", Decays::Nodes::StableCharged(), "Any 'trackable' particle: stable & charged" );
  // ==========================================================================
  // special CC-symbols, protect them...
  // ==========================================================================
  addCC( "cc" );
  addCC( "CC" );
  addCC( "os" );
  addCC( "nos" );
  addCC( "HasQuark" );
  addCC( "JSpin" );
  addCC( "LSpin" );
  addCC( "SSpin" );
  addCC( "ShortLived" );
  addCC( "LongLived" );
  addCC( "ShortLived_" );
  addCC( "LongLived_" );
  addCC( "Stable" );
  addCC( "Light" );
  addCC( "Heavy" );
  addCC( "CTau" );
  addCC( "Mass" );
  addCC( "up" );
  addCC( "down" );
  addCC( "strange" );
  addCC( "charm" );
  addCC( "beauty" );
  addCC( "bottom" );
  addCC( "top" );
  // ==========================================================================
}

// ============================================================================
// get CC-map
// ============================================================================
const Decays::Symbols::CCMap& Decays::Symbols::cc() const { return m_cc; }
// ============================================================================
/*  add new symbol to the internal structure
 *  @param symbol the symbol definition
 *  @param node   the actual node
 *  @param help   the help string
 *  @param ccsym  the symbol for charge coonjugation
 *  @return true if the symbol is added into the storage
 */
// ============================================================================
bool Decays::Symbols::addSymbol( std::string sym, const Decays::iNode& node, const std::string& help,
                                 std::string ccsym ) {
  // ==========================================================================
  // trim the arguments
  boost::trim( sym );
  // ==========================================================================
  auto ifind = m_nodes.find( sym );
  if ( m_nodes.end() != ifind ) { return false; } // RETURN
  // add the node into the map
  bool inserted = m_nodes.insert( sym, node ).second;
  if ( !inserted ) { return false; } // RETURN
  // add the help-string
  m_help[sym] = help;
  // add cc-symbols
  addCC( sym, ccsym );
  return true;
}
// ============================================================================
/*  add cc-pair to the internal map
 *  @param sym the symbol
 *  @param ccsym the symbol for charge conjugation
 */
// ============================================================================
void Decays::Symbols::addCC( std::string sym, std::string ccsym ) {
  // ==========================================================================
  // trim the arguments
  boost::trim( sym );
  boost::trim( ccsym );
  if ( ccsym.empty() ) { ccsym = sym; }
  // ==========================================================================
  m_cc[sym] = ccsym;
  if ( ccsym != sym ) { m_cc[ccsym] = sym; }
  // ==========================================================================
}
// ============================================================================
// valid symbol?
// ============================================================================
bool Decays::Symbols::valid( std::string sym ) const {
  // trim the argument
  boost::trim( sym );
  return m_nodes.end() != m_nodes.find( sym );
}
// ============================================================================
// help for the symbol
// ============================================================================
const std::string& Decays::Symbols::symbol( std::string sym ) const {
  // trim the argument
  boost::trim( sym );
  NodeMap::const_iterator ifind = m_nodes.find( sym );
  if ( m_nodes.end() == ifind ) { return s_EMPTY; } // RETURN
  HelpMap::const_iterator ihelp = m_help.find( sym );
  if ( m_help.end() == ihelp ) { return s_EMPTY; } // RETURN
  return ihelp->second;
}
// ============================================================================
/*  get the node by symbol
 *  @param (INPUT) sym the symbol name
 *  @param (OUTPUT) the symbol
 *  @return status code
 */
// ============================================================================
StatusCode Decays::Symbols::symbol( std::string sym, Decays::Node& node ) const {
  // trim the argument
  boost::trim( sym );
  auto ifind = m_nodes.find( sym );
  if ( m_nodes.end() != ifind ) {
    node = ifind->second;
    return StatusCode::SUCCESS; // RETURN
  }
  node = Decays::Nodes::Invalid();
  return StatusCode( InvalidSymbol ); // RETURN
}
// ============================================================================
// get all known symbols
// ============================================================================
size_t Decays::Symbols::symbols( Decays::Symbols::Names& names ) const {
  // clear names ;
  names.clear();
  for ( auto inode = m_nodes.begin(); m_nodes.end() != inode; ++inode ) { names.push_back( inode->first ); }
  // sort it according to CC-criteria                               ATTENTION!
  std::stable_sort( names.begin(), names.end(), Decays::CC::CmpCC() );
  return names.size();
}
// ============================================================================
/*  get all known particle names
 *  @param service (INPUT)  particle property service
 *  @param parts   (OUTPUT) vector of particle names
 *  @return status code
 */
// ============================================================================
StatusCode Decays::Symbols::particles( const Gaudi::IParticlePropertySvc* service,
                                       Decays::Symbols::Names&            parts ) const {
  // clear the output
  parts.clear();
  // check the service
  if ( 0 == service ) { return StatusCode( InvalidService ); } // RETURN
  //
  auto begin = service->begin();
  auto end   = service->end();
  for ( ; end != begin; ++begin ) {
    const Gaudi::ParticleProperty* pp = *begin;
    if ( !pp ) { continue; }
    parts.push_back( pp->particle() );
  }
  // sort it according to CC-criteria                                 ATTENTION!
  std::stable_sort( parts.begin(), parts.end(), Decays::CC::CmpCC() );
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
// The END
// ============================================================================
