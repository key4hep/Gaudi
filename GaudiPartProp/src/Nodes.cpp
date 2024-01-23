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
#include <Gaudi/Decays/Nodes.h>
#include <Gaudi/ParticleID.h>
#include <GaudiKernel/SerializeSTL.h>
#include <algorithm>
#include <functional>

namespace Decays = Gaudi::Decays;

/** @file
 *  The implementation file for various decay nodes
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-04-12
 */

Decays::NodeList::NodeList( const Decays::NodeList::Nodes_& nodes ) : m_nodes( nodes ) {}
void Decays::NodeList::push_back( const Decays::Nodes::_Node& node ) { m_nodes.push_back( node ); }
void Decays::NodeList::push_back( const Decays::iNode& node ) { m_nodes.push_back( node ); }
void Decays::NodeList::push_back( const Decays::NodeList::Nodes_& nodes ) {
  m_nodes.insert( m_nodes.end(), nodes.begin(), nodes.end() );
}
void              Decays::NodeList::push_back( const NodeList& nodes ) { push_back( nodes.m_nodes ); }
Decays::NodeList& Decays::NodeList::operator=( const Decays::Nodes::_Node& node ) {
  m_nodes.clear();
  push_back( node );
  return *this;
}
Decays::NodeList& Decays::NodeList::operator=( const Decays::iNode& node ) {
  m_nodes.clear();
  push_back( node );
  return *this;
}

Decays::Nodes::_Node& Decays::Nodes::_Node::op_or( const Decays::NodeList& right ) {
  if ( !right.empty() ) { m_node |= Decays::Nodes::Or( right ); }
  return *this;
}
Decays::Nodes::_Node& Decays::Nodes::_Node::op_and( const Decays::NodeList& right ) {
  if ( !right.empty() ) { m_node &= Decays::Nodes::And( right ); }
  return *this;
}

// MANDATORY: clone method ("virtual constructor")
Decays::Nodes::Invalid* Decays::Nodes::Invalid::clone() const { return new Invalid( *this ); }
// MANDATORY: check the validity
bool Decays::Nodes::Invalid::valid() const { return false; }
// MANDATORY: the proper validation of the node
StatusCode Decays::Nodes::Invalid::validate( const Gaudi::Interfaces::IParticlePropertySvc* /*svc */ ) const {
  return StatusCode::FAILURE;
}
// MANDATORY: the only one essential method
bool          Decays::Nodes::Invalid::operator()( const Gaudi::ParticleID& /* p */ ) const { return false; }
std::ostream& Decays::Nodes::Invalid::fillStream( std::ostream& s ) const { return s << " <INVALID> "; }

// the default constructor
Decays::Nodes::_Node::_Node() : m_node( Decays::Nodes::Invalid() ) {}

// constructor from two nodes
Decays::Nodes::Or::Or( const Decays::iNode& n1, const Decays::iNode& n2 ) {
  add( n1 );
  add( n2 );
}
// constructor from three nodes
Decays::Nodes::Or::Or( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3 ) {
  add( n1 );
  add( n2 );
  add( n3 );
}
// constructor from four nodes
Decays::Nodes::Or::Or( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3,
                       const Decays::iNode& n4 ) {
  add( n1 );
  add( n2 );
  add( n3 );
  add( n4 );
}
// constructor from list of nodes
Decays::Nodes::Or::Or( const Decays::NodeList& nodes ) { add( nodes ); }
// add the node
size_t Decays::Nodes::Or::add( const Decays::iNode& node ) {
  const Decays::iNode* right = &node;
  const Or*            _or   = dynamic_cast<const Or*>( right );
  if ( 0 != _or ) { return add( _or->m_nodes ); } // RETURN
  // holder ?
  const Node* _no = dynamic_cast<const Node*>( right );
  if ( 0 != _no ) { return add( _no->node() ); } // RETURN
  // just add the node
  m_nodes.push_back( node );
  return m_nodes.size();
}
// add the list of nodes
size_t Decays::Nodes::Or::add( const Decays::NodeList& nodes ) {
  for ( const auto& n : nodes ) add( n );
  return m_nodes.size();
}
// MANDATORY: clone method ("virtual constructor")
Decays::Nodes::Or* Decays::Nodes::Or::clone() const { return new Or( *this ); }
// MANDATORY: the only one essential method
bool Decays::Nodes::Or::operator()( const Gaudi::ParticleID& pid ) const {
  return m_nodes.end() != std::find_if( m_nodes.begin(), m_nodes.end(),
                                        [&]( const Decays::Nodes::_Node& node ) { return node( pid ); } );
}
// MANDATORY: the specific printout
std::ostream& Decays::Nodes::Or::fillStream( std::ostream& s ) const {
  s << " (";
  for ( Decays::NodeList::const_iterator node = m_nodes.begin(); m_nodes.end() != node; ++node ) {
    if ( m_nodes.begin() != node ) { s << "|"; }
    s << *node;
  }
  return s << ") ";
}
// MANDATORY: check the validity
bool Decays::Nodes::Or::valid() const { return !m_nodes.empty() && Decays::valid( m_nodes.begin(), m_nodes.end() ); }
// MANDATORY: the proper validation of the node
StatusCode Decays::Nodes::Or::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  if ( m_nodes.empty() ) { return StatusCode::FAILURE; }
  return Decays::validate( m_nodes.begin(), m_nodes.end(), svc );
}

// constructor from two nodes
Decays::Nodes::And::And( const Decays::iNode& n1, const Decays::iNode& n2 ) {
  add( n1 );
  add( n2 );
}
// constructor from three nodes
Decays::Nodes::And::And( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3 ) {
  add( n1 );
  add( n2 );
  add( n3 );
}
// constructor from four nodes
Decays::Nodes::And::And( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3,
                         const Decays::iNode& n4 ) {
  add( n1 );
  add( n2 );
  add( n3 );
  add( n4 );
}
// constructor form list of nodes
Decays::Nodes::And::And( const Decays::NodeList& nodes ) { add( nodes ); }
// add the node
size_t Decays::Nodes::And::add( const Decays::iNode& node ) {
  const Decays::iNode* right = &node;
  const And*           _and  = dynamic_cast<const And*>( right );
  if ( 0 != _and ) { return add( _and->m_nodes ); } // RETURN
  // holder ?
  const Node* _no = dynamic_cast<const Node*>( right );
  if ( 0 != _no ) { return add( _no->node() ); } // RETURN
  // just add the node
  m_nodes.push_back( node );
  return m_nodes.size();
}
// add the list of nodes
size_t Decays::Nodes::And::add( const Decays::NodeList& nodes ) {
  for ( const auto& n : nodes ) add( n );
  return m_nodes.size();
}
// MANDATORY: clone method ("virtual constructor")
Decays::Nodes::And* Decays::Nodes::And::clone() const { return new And( *this ); }
// MANDATORY: the only one essential method
bool Decays::Nodes::And::operator()( const Gaudi::ParticleID& pid ) const {
  if ( m_nodes.empty() ) { return false; }
  return std::all_of( m_nodes.begin(), m_nodes.end(), [&]( const Decays::Nodes::_Node& n ) { return n == pid; } );
}
// MANDATORY: the specific printout
std::ostream& Decays::Nodes::And::fillStream( std::ostream& s ) const {
  return GaudiUtils::details::ostream_joiner( s << " (", m_nodes, "&" ) << ") ";
}
// MANDATORY: check the validity
bool Decays::Nodes::And::valid() const { return !m_nodes.empty() && Decays::valid( m_nodes.begin(), m_nodes.end() ); }
// MANDATORY: the proper validation of the node
StatusCode Decays::Nodes::And::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  if ( m_nodes.empty() ) { return StatusCode::FAILURE; }
  return Decays::validate( m_nodes.begin(), m_nodes.end(), svc );
}

// constructor from the node
Decays::Nodes::Not::Not( const Decays::iNode& node ) : Decays::iNode(), m_node( node ) {}
// MANDATORY: clone method ("virtual constructor")
Decays::Nodes::Not* Decays::Nodes::Not::clone() const { return new Not( *this ); }
// valid ?
bool       Decays::Nodes::Not::valid() const { return m_node.valid(); }
bool       Decays::Nodes::Not::operator()( const Gaudi::ParticleID& pid ) const { return !m_node.node( pid ); }
StatusCode Decays::Nodes::Not::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  return m_node.validate( svc );
}

std::ostream& Decays::Nodes::Not::fillStream( std::ostream& s ) const { return s << " ~(" << m_node << ") "; }

Decays::Nodes::Or& Decays::Nodes::Or::operator+=( const Decays::iNode& node ) {
  add( node );
  return *this;
}
Decays::Nodes::And& Decays::Nodes::And::operator+=( const Decays::iNode& node ) {
  add( node );
  return *this;
}
Decays::Nodes::Or& Decays::Nodes::Or::operator+=( const Decays::NodeList& nodes ) {
  add( nodes );
  return *this;
}
Decays::Nodes::And& Decays::Nodes::And::operator+=( const Decays::NodeList& nodes ) {
  add( nodes );
  return *this;
}
