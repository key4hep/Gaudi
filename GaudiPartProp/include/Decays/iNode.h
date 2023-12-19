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
#ifndef PARTPROP_DECAYS_NODE_H
#define PARTPROP_DECAYS_NODE_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <functional>
#include <iosfwd>
#include <memory>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// forward declarations
// ============================================================================
namespace Gaudi {
  namespace Interfaces {
    class IParticlePropertySvc;
  }
  class ParticleID;
} // namespace Gaudi
// ============================================================================
namespace Decays {
  // ========================================================================
  /** @class iNode Kernel/iNode.h
   *  The abstract class which represents the single "node" of decay tree
   *
   *  The class is imported from LoKi project
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2008-04-12
   */
  class GAUDI_API iNode {
  public:
    // ========================================================================
    /** the basic operation: comparison of the particle PID with the node
     *  @param   pid the particle to be compared
     *  @return  true if the actual PID matched the node dectriptor
     */
    virtual bool operator()( const Gaudi::ParticleID& pid ) const = 0;
    // ========================================================================
    /// clone method ("virtual constructor")
    virtual iNode* clone() const = 0;
    // ========================================================================
    /** printout of the stream
     *  @param s the reference to the output stream
     *  @return the reference to the output stream
     */
    virtual std::ostream& fillStream( std::ostream& s ) const = 0;
    /// check the validity of the node
    virtual bool valid() const = 0;
    /// invalid node?
    virtual bool operator!() const;
    // ========================================================================
    /** validate the decay node
     *  @param svc pointer to Particle Property Service
     *  @return StatusCode
     */
    virtual StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const = 0;
    // ========================================================================
    /// the string representation of the node
    virtual std::string toString() const;
    // ========================================================================
    /// virtual desctructor
    virtual ~iNode() = default;
    // ========================================================================
  };
  // ==========================================================================
  /** @class Node
   *  The generic class to hold the pointer to other node
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  class GAUDI_API Node : public iNode {
  public:
    // ========================================================================
    /// constructor from the node
    Node( const Decays::iNode& node );
    /// copy constructor
    Node( const Decays::Node& right );
    /// MANDATORY: clone method ("virtual constructor")
    Node* clone() const override;
    /// MANDATORY: the only one essential method
    bool operator()( const Gaudi::ParticleID& p ) const override;
    /// MANDATORY: the specific printout
    std::ostream& fillStream( std::ostream& s ) const override;
    /// MANDATORY: check the validity of the node
    bool valid() const override;
    /// MANDATORY: the proper validation of the node
    StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const override;
    // ========================================================================
  public:
    // ========================================================================
    Node& operator&=( const iNode& right ) { return op_and( right ); }
    Node& operator|=( const iNode& right ) { return op_or( right ); }
    // ========================================================================
  public:
    // ========================================================================
    // get the underlying node
    inline const iNode& node() const { return *m_node; }
    // ========================================================================
    // evaluate the underlying node
    inline bool node( const Gaudi::ParticleID& pid ) const { return node()( pid ); }
    // ========================================================================
  public:
    // ========================================================================
    /// assignement operator:
    Node& operator=( const Node& right );
    /// assignement from arbitrary node
    Node& operator=( const iNode& right );
    // ========================================================================
  private:
    // ========================================================================
    Node& op_and( const iNode& right );
    Node& op_or( const iNode& right );
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled
    Node(); // no default constructor
    // ========================================================================
  private:
    // ========================================================================
    /// the node itself:
    std::unique_ptr<iNode> m_node; // the node itself:
    // ========================================================================
  };
  // ==========================================================================
  /** printout to the output stream
   *  @param s the output stream
   *  @param n the node
   *  @return the output stream (Reference)
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline std::ostream& operator<<( std::ostream& s, const iNode& n ) { return n.fillStream( s ); }
  // ==========================================================================
  /** equality
   *  The node is "equal" to the PID , if the given pid satisfies the node criteria
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline bool operator==( const Decays::iNode& node, const Gaudi::ParticleID& pid ) { return node( pid ); }
  // ==========================================================================
  /** "right" equality
   *  The node is "equal" to the PID , if the given pid satisfies the node criteria
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline bool operator==( const Gaudi::ParticleID& pid, const Decays::iNode& node ) { return node == pid; }
  // ==========================================================================
  /** non-equality
   *  The node is "equal" to the PID , if the given PID satisfies the node criteria
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline bool operator!=( const Decays::iNode& node, const Gaudi::ParticleID& pid ) { return !( node == pid ); }
  // ==========================================================================
  /** non-equality
   *  The node is "equal" to the PID , if the given PID satisfies the node criteria
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline bool operator!=( const Gaudi::ParticleID& pid, const Decays::iNode& node ) { return node != pid; }
  // ==========================================================================
  /** "on-flight" validation of the node
   *  @param n the node
   *  @param svc particle property service
   *  @return status code
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline StatusCode operator+( const Decays::iNode& n, const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
    return n.validate( svc );
  }
  // ==========================================================================
  /** "on-flight" validation of the node
   *  @param n the node
   *  @param svc particle property service
   *  @return status code
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  inline StatusCode operator*( const Decays::iNode& n, const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
    return n.validate( svc );
  }
  // ==========================================================================
} // end of namespace Decays
// ============================================================================
//                                                                      The END
// ============================================================================
#endif // PARTPROP_DECAYS_NODE_H
