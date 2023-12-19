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
#include <Decays/Nodes.h>
#include <Decays/iNode.h>
#include <GaudiKernel/StatusCode.h>
#include <sstream>
#include <typeinfo>

/** @file
 *  Implementation file for class Decays::iNode
 *  @date 2008-05-04
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 */
std::string Decays::iNode::toString() const {
  std::ostringstream s;
  fillStream( s );
  return s.str();
}

bool Decays::iNode::operator!() const { return !( this->valid() ); }

Decays::Node::Node( const Decays::iNode& node ) {
  if ( typeid( Decays::Node ) == typeid( node ) ) {
    const Node& _node = dynamic_cast<const Node&>( node );
    m_node.reset( _node.m_node->clone() );
  } else {
    m_node.reset( node.clone() );
  }
}

Decays::Node::Node( const Decays::Node& right ) : Decays::iNode( right ), m_node( right.m_node->clone() ) {}

bool Decays::Node::operator()( const Gaudi::ParticleID& pid ) const { return node( pid ); }

std::ostream& Decays::Node::fillStream( std::ostream& s ) const { return m_node->fillStream( s ); }

bool Decays::Node::valid() const { return m_node->valid(); }

StatusCode Decays::Node::validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
  return m_node->validate( svc );
}

Decays::Node& Decays::Node::operator=( const Decays::Node& right ) {
  if ( &right == this ) { return *this; }
  m_node.reset( right.m_node->clone() );
  return *this;
}

Decays::Node& Decays::Node::operator=( const Decays::iNode& right ) {
  if ( &right == this ) { return *this; }
  Decays::iNode* node = nullptr;
  if ( typeid( Decays::Node ) == typeid( right ) ) {
    const Node& _node = dynamic_cast<const Node&>( right );
    node              = _node.m_node->clone();
  } else {
    node = right.clone();
  }
  m_node.reset( node );
  return *this;
}

Decays::Node* Decays::Node::clone() const { return new Decays::Node( *this ); }

Decays::Node& Decays::Node::op_and( const Decays::iNode& right ) { return ( *this = ( *m_node && right ) ); }

Decays::Node& Decays::Node::op_or( const Decays::iNode& right ) { return ( *this = ( *m_node || right ) ); }
