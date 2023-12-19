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
#ifndef PARTPROP_DECAYNODES_H
#define PARTPROP_DECAYNODES_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StatusCode.h"
// ============================================================================
// GaudiPartProp
// ============================================================================
#include "Decays/Decay.h"
#include "Decays/iNode.h"
#include "Gaudi/ParticleID.h"
// ============================================================================
/** @file Decays/Nodes.h
 *  Helper general purpuse utilities to deal with decay nodes
 *  @see Decays::iNode
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2008-04-21
 */
// ============================================================================
namespace Decays {
  // ==========================================================================
  /** check the validness of the trees or nodes
   *  @param begin begin-iterator for the sequence of trees/nodes
   *  @param end end-iterator for the sequence of trees/nodes
   *  @return true of no invalid trees are found
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  template <typename Iterator>
  bool valid( Iterator begin, Iterator end ) {
    return std::all_of( begin, end, []( const auto& i ) { return i.valid(); } );
  }
  template <typename Container>
  bool valid( Container const& c ) {
    return valid( c.begin(), c.end() );
  }
  // ==========================================================================
  /** validate trees/nodes
   *  @param begin begin-iterator for the sequence of trees/nodes
   *  @param end end-iterator for the sequence of trees/nodes
   *  @param svc the Particle Property Service for validation
   *  @return statuis code
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-21
   */
  template <class Iterator>
  StatusCode validate( Iterator begin, Iterator end, const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
    for ( ; begin != end; ++begin ) {
      StatusCode sc = begin->validate( svc );
      if ( sc.isFailure() ) { return sc; }
    }
    return StatusCode::SUCCESS;
  }
  template <class TREE>
  StatusCode validate( TREE const& tree, const Gaudi::Interfaces::IParticlePropertySvc* svc ) {
    return validate( tree.begin(), tree.end(), svc );
  }
  // ==========================================================================
  class NodeList;
  // ==========================================================================
  namespace Nodes {
    // ========================================================================
    /** @class Invalid
     *  the most simple node to represent the invalid node
     *  it matches to all valid the Gaudi::Particles
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Invalid : public Decays::iNode {
    public:
      // ======================================================================
      /// MANDATORY: default constructor
      Invalid() = default;
      /// MANDATORY: clone method ("virtual constructor")
      Invalid* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& /* p */ ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    };
    // ========================================================================
    /** @class _Node
     *  Helper structure (esspectially it is light version node-holder
     *  the default constructor
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API _Node {
    public:
      // ======================================================================
      /// STL interface : result_type
      // typedef bool                                              result_type   ;
      /// STL interface : argument_type
      // typedef Gaudi::ParitcleID                                  argument_type ;
      // ======================================================================
    public:
      // ======================================================================
    public:
      // ======================================================================
      /// the default constructor
      _Node();
      /// the constructor from iNode
      _Node( const iNode& node ) : m_node( node ) {}
      /// the constructor from  Node
      _Node( const Node& node ) : m_node( node ) {}
      // default copy constructor
      _Node( const _Node& node ) = default;
      /// Check the validity
      inline bool valid() const { return m_node.node().valid(); }
      /// The proper validation of the node
      inline StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const {
        return m_node.node().validate( svc );
      }
      /// The major method
      inline bool operator()( const Gaudi::ParticleID& pid ) const { return m_node.node( pid ); }
      // ======================================================================
    public:
      // ======================================================================
      inline bool operator==( const Gaudi::ParticleID& pid ) const { return m_node.node( pid ); }
      // ======================================================================
      inline bool operator!=( const Gaudi::ParticleID& pid ) const { return !m_node.node( pid ); }
      // ======================================================================
    public:
      // ======================================================================
      /// pseudo-assignement operator:
      _Node& operator=( const Node& right ) {
        m_node = right;
        return *this;
      }
      /// pseudo-assignement from arbitrary node
      _Node& operator=( const iNode& right ) {
        m_node = right;
        return *this;
      }
      /// pseudo-assignement from arbitrary node
      _Node& operator=( const _Node& right ) {
        m_node = right.m_node;
        return *this;
      }
      // ======================================================================
    public:
      // ======================================================================
      _Node& operator|=( const iNode& right ) {
        m_node |= right;
        return *this;
      }
      _Node& operator&=( const iNode& right ) {
        m_node &= right;
        return *this;
      }
      // ======================================================================
      _Node& operator|=( const NodeList& right ) { return op_or( right ); }
      _Node& operator&=( const NodeList& right ) { return op_and( right ); }
      // ======================================================================
    public:
      // ======================================================================
      _Node& operator|=( const _Node& right ) {
        m_node |= right;
        return *this;
      }
      _Node& operator&=( const _Node& right ) {
        m_node &= right;
        return *this;
      }
      // ======================================================================
    public:
      // ======================================================================
      /// the accessor to the node
      const Decays::iNode& node() const { return m_node.node(); }
      /// the cast operator to the actual list of nodes
      operator const Decays::iNode&() const { return node(); }
      // ======================================================================
    private:
      // ======================================================================
      _Node& op_or( const NodeList& right );
      _Node& op_and( const NodeList& right );
      // ======================================================================
    private:
      // ======================================================================
      /// the node holder itself
      Decays::Node m_node; // the node holder itself
      // ======================================================================
    };
    // ========================================================================
  } // namespace Nodes
  // ==========================================================================
  class GAUDI_API NodeList {
  public:
    // ========================================================================
    /// the actual type of the sequence of nodes
    typedef std::vector<Decays::Nodes::_Node> Nodes_;
    typedef Nodes_::const_iterator            const_iterator;
    typedef Nodes_::iterator                  iterator;
    typedef Nodes_::value_type                value_type;
    // ========================================================================
  public:
    // ========================================================================
    /// costructor from the list of Nodes
    NodeList( const Nodes_& nodes = Nodes_() );
    // ========================================================================
  public:
    // ========================================================================
    void push_back( const Decays::Nodes::_Node& node );
    void push_back( const Decays::iNode& node );
    void push_back( const Nodes_& nodes );
    void push_back( const NodeList& nodes );
    void clear() { m_nodes.clear(); }
    // ========================================================================
  public:
    // ========================================================================
    NodeList& operator=( const Decays::Nodes::_Node& node );
    NodeList& operator=( const Decays::iNode& node );
    // ========================================================================
  public:
    // ========================================================================
    NodeList& operator+=( const Decays::Nodes::_Node& node ) {
      push_back( node );
      return *this;
    }
    NodeList& operator+=( const Decays::iNode& node ) {
      push_back( node );
      return *this;
    }
    NodeList& operator+=( const Nodes_& nodes ) {
      push_back( nodes );
      return *this;
    }
    NodeList& operator+=( const NodeList& nodes ) {
      push_back( nodes );
      return *this;
    }
    // ========================================================================
  public:
    // ========================================================================
    size_t         size() const { return m_nodes.size(); }
    bool           empty() const { return m_nodes.empty(); }
    iterator       begin() { return m_nodes.begin(); }
    iterator       end() { return m_nodes.end(); }
    const_iterator begin() const { return m_nodes.begin(); }
    const_iterator end() const { return m_nodes.end(); }
    // ========================================================================
  public:
    // ========================================================================
    operator const Nodes_&() const { return m_nodes; }
    // ========================================================================
  private:
    // ========================================================================
    /// the actual list of nodes
    Nodes_ m_nodes; // the actual list of nodes
    // ========================================================================
  };
  // ==========================================================================
  namespace Nodes {
    // ========================================================================
    /** @class Or
     *  the rather simple (but powerful) node in the decay tree:
     *  it matches .OR. for sub-nodes
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Or : public Decays::iNode {
    public:
      // ======================================================================
      /// constructor from two nodes
      Or( const Decays::iNode& n1, const Decays::iNode& n2 );
      /// constructor from three nodes
      Or( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3 );
      /// constructor from four nodes
      Or( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3, const Decays::iNode& n4 );
      /// constructor from list of nodes
      Or( const Decays::NodeList& nodes );
      /// MANDATORY: clone method ("virtual constructor")
      Or* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    protected:
      // ======================================================================
      size_t add( const Decays::iNode& node );
      size_t add( const Decays::NodeList& nodes );
      // ======================================================================
    public:
      // ======================================================================
      Or& operator+=( const Decays::iNode& node );
      Or& operator+=( const Decays::NodeList& node );
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      Or(); // the default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the sub-nodes
      Decays::NodeList m_nodes; // the sub-nodes
      // ======================================================================
    };
    // ========================================================================
    /** @class And
     *  the rather simple (but powerful) node in the decay tree:
     *  it matches .AND. for sub-nodes
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API And : public Decays::iNode {
    public:
      // ======================================================================
      /// constructor from two nodes
      And( const Decays::iNode& n1, const Decays::iNode& n2 );
      /// constructor from three nodes
      And( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3 );
      /// constructor from four nodes
      And( const Decays::iNode& n1, const Decays::iNode& n2, const Decays::iNode& n3, const Decays::iNode& n4 );
      /// constructor from list of nodes
      And( const Decays::NodeList& nodes );
      /// MANDATORY: clone method ("virtual constructor")
      And* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& p ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    protected:
      // ======================================================================
      size_t add( const Decays::iNode& node );
      size_t add( const Decays::NodeList& nodes );
      // ======================================================================
    public:
      // ======================================================================
      And& operator+=( const Decays::iNode& node );
      And& operator+=( const Decays::NodeList& nodes );
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      And(); // the default constructor is disabled
      // ======================================================================
    private:
      // ======================================================================
      /// the sub-nodes
      NodeList m_nodes; // the sub-nodes
      // ======================================================================
    };
    // ========================================================================
    /** @class Not
     *  Simple node whch match "NOT" for the subnode
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    class GAUDI_API Not : public Decays::iNode {
    public:
      // ======================================================================
      /// constructor from the node
      Not( const Decays::iNode& node );
      /// MANDATORY: clone method ("virtual constructor")
      Not* clone() const override;
      /// MANDATORY: the only one essential method
      bool operator()( const Gaudi::ParticleID& pid ) const override;
      /// MANDATORY: the specific printout
      std::ostream& fillStream( std::ostream& s ) const override;
      /// MANDATORY: check the validity
      bool valid() const override;
      /// MANDATORY: the proper validation of the node
      StatusCode validate( const Gaudi::Interfaces::IParticlePropertySvc* svc ) const override;
      // ======================================================================
    public:
      // ======================================================================
      /// get the underlying node
      const Decays::iNode& node() const { return m_node.node(); }
      // ======================================================================
    private:
      // ======================================================================
      /// the node itself
      Decays::Node m_node; // the node itself
      // ======================================================================
    };
    // ========================================================================
    /** Create the "NOT" for the node
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-04-12
     */
    inline Decays::Nodes::Not operator!( const Decays::Nodes::Not& o ) { return Decays::Node( o.node() ); }
    // ========================================================================
    /// output operator
    inline std::ostream& operator<<( std::ostream& s, const Decays::Nodes::_Node& n ) { return s << n.node(); }
    // ========================================================================
  } // namespace Nodes
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  inline Decays::Nodes::Or operator||( const Decays::iNode& o1, const Decays::iNode& o2 ) {
    return Decays::Nodes::Or( o1, o2 );
  }
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  inline Decays::Nodes::Or operator|( const Decays::iNode& o1, const Decays::iNode& o2 ) {
    return Decays::Nodes::Or( o1, o2 );
  }
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  inline Decays::Nodes::And operator&&( const Decays::iNode& o1, const Decays::iNode& o2 ) {
    return Decays::Nodes::And( o1, o2 );
  }
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  inline Decays::Nodes::And operator&( const Decays::iNode& o1, const Decays::iNode& o2 ) {
    return Decays::Nodes::And( o1, o2 );
  }
  // ==========================================================================
  /** Create the "NOT" for the node
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  inline Decays::Nodes::Not operator~( const Decays::iNode& o ) { return Decays::Nodes::Not( o ); }
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const std::string& o2 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Gaudi::ParticleID& o2 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Decays::Decay::Item& o2 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Decays::iNode& o1, const Gaudi::ParticleProperty* o2 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const std::string& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Gaudi::ParticleID& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Decays::Decay::Item& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "OR" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::Or operator||( const Gaudi::ParticleProperty* o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const std::string& o2 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Gaudi::ParticleID& o2 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Decays::Decay::Item& o2 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Decays::iNode& o1, const Gaudi::ParticleProperty* o2 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const std::string& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Gaudi::ParticleID& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Decays::Decay::Item& o2, const Decays::iNode& o1 );
  // ==========================================================================
  /** Create the "AND" of two nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2008-04-12
   */
  GAUDI_API
  Decays::Nodes::And operator&&( const Gaudi::ParticleProperty* o2, const Decays::iNode& o1 );
  // ==========================================================================
} //                                                    end of namespace Decays
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // PARTPROP_DECAYNODES_H
