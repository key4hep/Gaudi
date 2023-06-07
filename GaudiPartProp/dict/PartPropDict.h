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
#ifndef PARTPROP_PARTPROPDICT_H
#define PARTPROP_PARTPROPDICT_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiPartProp
// ============================================================================
#include "GaudiPartProp/Decay.h"
#include "GaudiPartProp/IParticlePropertySvc.h"
#include "GaudiPartProp/Nodes.h"
#include "GaudiPartProp/NodesPIDs.h"
#include "GaudiPartProp/ParticleID.h"
#include "GaudiPartProp/ParticleProperty.h"
#include "GaudiPartProp/Symbols.h"
#include "GaudiPartProp/iNode.h"
// ============================================================================
namespace Decays {
  // ==========================================================================
  namespace Dict {
    // ========================================================================
    struct NodeOps {
      // ======================================================================
      typedef Decays::iNode iNode;
      typedef Decays::Node  Node;
      // ======================================================================
    public:
      // ======================================================================
      // __call__
      static bool __call__( const iNode& node, const Gaudi::ParticleID& pid ) { return node( pid ); }
      // __call__
      static bool __call__( const iNode& node, const Gaudi::ParticleProperty* pp ) { return node( pp->particleID() ); }
      // __call__
      static bool __call__( const iNode& node, const int pid ) { return node( Gaudi::ParticleID( pid ) ); }
      // =====================================================================
    public:
      // =====================================================================
      // __rrshift__
      static bool __rrshift__( const iNode& node, const Gaudi::ParticleID& pid ) { return node( pid ); }
      // __rrshift__
      static bool __rrshift__( const iNode& node, const Gaudi::ParticleProperty* pp ) {
        return node( pp->particleID() );
      }
      // __rrshift__
      static bool __rrshift__( const iNode& node, const int pid ) { return node( Gaudi::ParticleID( pid ) ); }
      // =====================================================================
    public:
      // =====================================================================
      // __or__
      static Node __or__( const iNode& n1, const iNode& n2 ) { return n1 || n2; }
      // __or__
      static Node __or__( const iNode& n1, const std::string& n2 ) { return n1 || n2; }
      // __or__
      static Node __or__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 || n2; }
      // __or__
      static Node __or__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 || n2; }
      // __or__
      static Node __or__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 || n2; }
      // ======================================================================
    public:
      // ======================================================================
      // __ror__
      static Node __ror__( const iNode& n1, const std::string& n2 ) { return n1 || n2; }
      // __ror__
      static Node __ror__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 || n2; }
      // __ror__
      static Node __ror__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 || n2; }
      // __ror__
      static Node __ror__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 || n2; }
      // ======================================================================
    public:
      // =====================================================================
      // __and__
      static Node __and__( const iNode& n1, const iNode& n2 ) { return n1 && n2; }
      // __and__
      static Node __and__( const iNode& n1, const std::string& n2 ) { return n1 && n2; }
      // __and__
      static Node __and__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 && n2; }
      // __and__
      static Node __and__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 && n2; }
      // __and__
      static Node __and__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 && n2; }
      // ======================================================================
    public:
      // =====================================================================
      // __rand__
      static Node __rand__( const iNode& n1, const std::string& n2 ) { return n1 && n2; }
      // __rand__
      static Node __rand__( const iNode& n1, const Gaudi::ParticleID& n2 ) { return n1 && n2; }
      // __rand__
      static Node __rand__( const iNode& n1, const Gaudi::ParticleProperty* n2 ) { return n1 && n2; }
      // __rand__
      static Node __rand__( const iNode& n1, const Decays::Decay::Item& n2 ) { return n1 && n2; }
      // ======================================================================
    public:
      // =====================================================================
      // __rshift__
      static Node __rshift__( const iNode& n1, const iNode& n2 ) { return n1 && n2; }
      // =====================================================================
    public:
      // =====================================================================
      // __invert__
      static Node __invert__( const iNode& n1 ) { return ~n1; }
      // =====================================================================
    };
    // ========================================================================
  } // namespace Dict
  // ==========================================================================
} //                                                    end of namespace Decays
// ============================================================================
namespace {
  // ==========================================================================
  struct PartProp_Instantiations {
    // ========================================================================
    Gaudi::ParticleID                           m_pid;
    Gaudi::ParticleProperty                     m_pp;
    std::vector<Gaudi::ParticleID>              m_pidv;
    std::vector<const Gaudi::ParticleProperty*> m_ppv;
    //
    std::vector<Decays::Decay>       m_vd;
    std::vector<Decays::Decay::Item> m_vi;
    // ioperations with nodes
    Decays::Dict::NodeOps m_no;
    // fictive constructor
    PartProp_Instantiations();
    // ========================================================================
  };
  // ==========================================================================
} // namespace
// ============================================================================
#endif // PARTPROP_PARTPROPDICT_H
