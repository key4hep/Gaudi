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
#include <Decays/Decay.h>
#include <Decays/Nodes.h>
#include <Decays/NodesPIDs.h>
#include <Decays/Symbols.h>
#include <Decays/iNode.h>
#include <Gaudi/Interfaces/IParticlePropertySvc.h>
#include <Gaudi/ParticleID.h>
#include <Gaudi/ParticleProperty.h>

namespace Decays {
  namespace Dict {
    struct NodeOps {
      typedef Decays::iNode iNode;
      typedef Decays::Node  Node;
      static bool           __call__( const iNode& node, const Gaudi::ParticleID& pid ) { return node( pid ); }
      static bool __call__( const iNode& node, const Gaudi::ParticleProperty* pp ) { return node( pp->particleID() ); }
      static bool __call__( const iNode& node, const int pid ) { return node( Gaudi::ParticleID( pid ) ); }
      static bool __rrshift__( const iNode& node, const Gaudi::ParticleID& pid ) { return node( pid ); }
      static bool __rrshift__( const iNode& node, const Gaudi::ParticleProperty* pp ) {
        return node( pp->particleID() );
      }
      static bool __rrshift__( const iNode& node, const int pid ) { return node( Gaudi::ParticleID( pid ) ); }
      static Node __or__( const iNode& n1, const iNode& n2 ) { return n1 || n2; }
      static Node __or__( const iNode& n1, const std::string& n2 ) { return n1 || n2; }
      static Node __or__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 || n2; }
      static Node __or__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 || n2; }
      static Node __or__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 || n2; }
      static Node __ror__( const iNode& n1, const std::string& n2 ) { return n1 || n2; }
      static Node __ror__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 || n2; }
      static Node __ror__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 || n2; }
      static Node __ror__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 || n2; }
      static Node __and__( const iNode& n1, const iNode& n2 ) { return n1 && n2; }
      static Node __and__( const iNode& n1, const std::string& n2 ) { return n1 && n2; }
      static Node __and__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 && n2; }
      static Node __and__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 && n2; }
      static Node __and__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 && n2; }
      static Node __rand__( const iNode& n1, const std::string& n2 ) { return n1 && n2; }
      static Node __rand__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 && n2; }
      static Node __rand__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 && n2; }
      static Node __rand__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 && n2; }
      static Node __rshift__( const iNode& n1, const iNode& n2 ) { return n1 && n2; }
      static Node __invert__( const iNode& n1 ) { return ~n1; }
    };
  } // namespace Dict
} // namespace Decays

namespace {
  struct PartProp_Instantiations {
    std::vector<Gaudi::ParticleID>              m_pidv;
    std::vector<const Gaudi::ParticleProperty*> m_ppv;

    std::vector<Decays::Decay>       m_vd;
    std::vector<Decays::Decay::Item> m_vi;
  };
} // namespace
