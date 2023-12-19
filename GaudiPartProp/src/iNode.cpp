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
#include <sstream>
#include <typeinfo>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// LoKi
// ============================================================================
#include "GaudiPartProp/Nodes.h"
#include "GaudiPartProp/iNode.h"
// ============================================================================
/** @file
 *  Implementation file for class Decays::iNode
 *  @date 2008-05-04
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 */
// ===========================================================================
// iNode
// ===========================================================================
/*  conversion to the string
 *  @return the string representation of the node
 */
// ===========================================================================
std::string Decays::iNode::toString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}
// ============================================================================
// invalid node ?
// ============================================================================
bool Decays::iNode::operator!() const { return !( this->valid() ); }
// ===========================================================================
// Decays::Node
// ===========================================================================
// constructor from the node
// ===========================================================================
Decays::Node::Node( const Decays::iNode& node ) {
  if ( typeid( Decays::Node ) == typeid( node ) ) {
    const Node& _node = dynamic_cast<const Node&>( node );
    m_node.reset( _node.m_node->clone() );
  } else {
    m_node.reset( node.clone() );
  }
}
// ===========================================================================
// copy constructor
// ===========================================================================
Decays::Node::Node( const Decays::Node& right ) : Decays::iNode( right ), m_node( right.m_node->clone() ) {}
// ===========================================================================
// MANDATORY: the only one essential method
// ===========================================================================
bool Decays::Node::operator()( const Gaudi::ParticleID& pid ) const { return node( pid ); }
// ===========================================================================
// MANDATORY: the specific printout
// ===========================================================================
std::ostream& Decays::Node::fillStream( std::ostream& s ) const { return m_node->fillStream( s ); }
// ===========================================================================
// check the validity of the node
// ===========================================================================
bool Decays::Node::valid() const { return m_node->valid(); }
// ===========================================================================
// MANDATORY: the proper validation of the node
// ===========================================================================
StatusCode Decays::Node::validate( const Gaudi::IParticlePropertySvc* svc ) const { return m_node->validate( svc ); }
// ===========================================================================
// assignement operator
// ===========================================================================
Decays::Node& Decays::Node::operator=( const Decays::Node& right ) {
  if ( &right == this ) { return *this; }
  m_node.reset( right.m_node->clone() );
  return *this;
}
// ===========================================================================
// assignement operator
// ===========================================================================
Decays::Node& Decays::Node::operator=( const Decays::iNode& right ) {
  if ( &right == this ) { return *this; }
  Decays::iNode* node = nullptr;
  if ( typeid( Decays::Node ) == typeid( right ) ) {
    const Node& _node = dynamic_cast<const Node&>( right );
    node              = _node.m_node->clone();
  } else {
    node = right.clone();
  }
  //
  m_node.reset( node );
  //
  return *this;
}
// ===========================================================================
// MANDATORY: clone method ("virtual constructor")
// ===========================================================================
Decays::Node* Decays::Node::clone() const { return new Decays::Node( *this ); }
// ============================================================================
// operator &=
// ============================================================================
Decays::Node& Decays::Node::op_and( const Decays::iNode& right ) { return ( *this = ( *m_node && right ) ); }
// ============================================================================
// operator |=
// ============================================================================
Decays::Node& Decays::Node::op_or( const Decays::iNode& right ) { return ( *this = ( *m_node || right ) ); }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
