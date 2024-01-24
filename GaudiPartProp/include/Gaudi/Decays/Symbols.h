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
#pragma once

#include <Gaudi/Decays/Nodes.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/VectorMap.h>
#include <map>
#include <string>
#include <vector>

namespace Gaudi {
  class IParticlePropertySvc;
}

/** @file Kernel/Symbols.h
 *  Helper file with the definition of symbols, used for particles/nodes/decays
 *  @see Gaudi::Interfaces::IParticlePropertySvc
 *  @see Gaudi::ParticleProperty
 *  @see Gaudi::ParticleID
 *  Many thanks to Antonio PELLEGRINO for the kind help with regex expressions
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-05-07
 */
namespace Gaudi::Decays {
  /** @class Symbols
   *  Helper class to obtain the information about the valid
   *  symbols for particles, decay descriptors and Nodes.
   *
   *  Many thanks to Antonio PELLEGRINO for the kind help with regex expressions
   *
   *  @see Decays::iNode
   *  @see Decays::Nodes
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-05-07
   */
  class GAUDI_API Symbols {
  public:
    enum { InvalidService = 400, InvalidSymbol, NoParticles, NoSymbols };

    /// the actual type for the list of names
    typedef std::vector<std::string>           Names;
    typedef std::map<std::string, std::string> CCMap;

    /// get CC-map
    const CCMap& cc() const;

    /** valid basic/primitive symbol?
     *  @param sym the symbol
     *  @return true if it is vaild special symbol
     */
    bool valid( std::string sym ) const;
    /** help for the basic primitive special symbol
     *  @param sym the symbol
     *  @return the help string
     */
    const std::string& symbol( std::string sym ) const;
    /** get the node by symbol
     *  @param (INPUT)  sym the symbol name
     *  @param (OUTPUT) the symbol
     *  @return status code
     */
    StatusCode symbol( std::string sym, Decays::Node& node ) const;

    /** get all known basic primitive special symbols
     *  @param names (OUTOUT) vector of all known special symbols
     *  @return size of the vector
     */
    size_t symbols( Names& names ) const;
    /** get all known particle names
     *  @param svc   (INPUT) particle property service
     *  @param parts (OUTPUT) vector of particle names
     *  @return status code
     */
    StatusCode particles( const Gaudi::Interfaces::IParticlePropertySvc* svc, Names& parts ) const;

    /** add new symbol to the internal structure
     *  @param sym    the symbol definition
     *  @param node   the actual node
     *  @param help   the help string
     *  @param ccsym  the symbol for charge coonjugation
     *  @return true if the symbol is added into the storage
     */
    bool addSymbol( std::string sym, const Decays::iNode& node, const std::string& help, std::string ccsym = "" );

    /** add cc-pair to the internal map
     *  @param sym the symbol
     *  @param ccsym the symbol for charge conjugation
     */
    void addCC( std::string sym, std::string ccsym = "" );

    /// static accessor to teh singleton
    static Symbols& instance();

  protected:
    /// the default constructor is protected:
    Symbols();
    /// the destructor is protected
    virtual ~Symbols() = default;

  private:
    /// copy constructor is private
    Symbols( const Symbols& );
    /// assignment operator is private
    Symbols& operator=( const Symbols& );

    typedef GaudiUtils::VectorMap<std::string, Decays::Nodes::_Node> NodeMap;
    typedef std::map<std::string, std::string>                       HelpMap;

    /// the actual map of symbols
    NodeMap m_nodes;
    /// the actual help-map
    HelpMap m_help;
    /// the map of cc-symbols
    CCMap m_cc;
  };
} // namespace Gaudi::Decays
