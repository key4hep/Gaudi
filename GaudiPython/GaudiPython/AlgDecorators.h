/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
#ifndef GAUDIPYTHON_ALGDECORATORS_H
#define GAUDIPYTHON_ALGDECORATORS_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================
// forward declarations
// ============================================================================
class IInterface;
class IProperty;
class IAlgTool;
class IAlgorithm;
class GaudiAlgorithm;
class GaudiTool;
class DataObject;
// ============================================================================
/** @file
 *
 *  Collection of "decorators" for python algorithms
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date   2005-08-03
 */
// ============================================================================
namespace GaudiPython {
  // ==========================================================================
  /** @class Decorator
   *  simple class with allows to "decorate" the python
   *  algorithm with 'basic' funtionality from class GaudiAlgorithm
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2007-08-03
   */
  class GAUDI_API AlgDecorator {
  public:
    // ========================================================================
    /// the actual type of vector of counters
    typedef std::vector<Gaudi::Accumulators::PrintableCounter*> Counters;
    typedef std::vector<IAlgTool*>                              Tools;
    typedef std::vector<IService*>                              Services;
    // ========================================================================
    // the general functional methods
    // ========================================================================
  public:
    /** get the tool from GaudiAlgorithm
     *  @param alg GaudiAlgorithm
     *  @param type tool type
     *  @param name tool name
     *  @param parent tool parent
     *  @param create flag to create
     *  @return the tool
     */
    static IAlgTool* tool_( const GaudiAlgorithm* alg, const std::string& type, const std::string& name,
                            const IInterface* parent = 0, const bool create = true );
    /** get the tool from GaudiAlgorithm
     *  @param alg GaudiAlgorithm
     *  @param typeAndName tool type/name
     *  @param parent tool parent
     *  @param create flag to create
     *  @return the tool
     */
    static IAlgTool* tool_( const GaudiAlgorithm* alg, const std::string& typeAndName, const IInterface* parent = 0,
                            const bool create = true );
    /** get the service from GaudiAlgorithm
     *  @param alg GaudiAlgorithm
     *  @param name service name
     *  @param create flag to create
     *  @return the tool
     */
    static IInterface* svc_( const GaudiAlgorithm* alg, const std::string& name, const bool create = false );
    // ========================================================================
    // get the data from TES
    // ========================================================================
  public:
    /** get the data from TES
     *  @param alg          GaudiAlgorithm
     *  @param location     data location in TES
     *  @param useRoonInTes flag to respect RootInTes
     *  @return the data
     */
    static DataObject* get_( const GaudiAlgorithm* alg, const std::string& location, const bool useRootInTes );
    // ========================================================================
  public:
    // ========================================================================
    /** get the data from TES
     *  @param alg          GaudiAlgorithm
     *  @param location     data location in TES
     *  @param useRoonInTes flag to respect RootInTes
     *  @return the data
     */
    static bool exist( const GaudiAlgorithm* alg, const std::string& location, const bool useRootInTes );
    // ========================================================================
  public: // inspection: get all counters:
    // ========================================================================
    static size_t _counters_a_( const GaudiAlgorithm* alg, std::vector<std::string>& names, Counters& out );
    static size_t _counters_t_( const GaudiTool* alg, std::vector<std::string>& names, Counters& out );
    static size_t _counters_a_( const IAlgorithm* alg, std::vector<std::string>& names, Counters& out );
    static size_t _counters_t_( const IAlgTool* alg, std::vector<std::string>& names, Counters& out );
    // ========================================================================
  public: // inspection: get the counter
    // ========================================================================
    static StatEntity* _counter_a_( const GaudiAlgorithm* alg, const std::string& name );
    static StatEntity* _counter_t_( const GaudiTool* alg, const std::string& name );
    static StatEntity* _counter_a_( const IAlgorithm* alg, const std::string& name );
    static StatEntity* _counter_t_( const IAlgTool* alg, const std::string& name );
    // ========================================================================
  public: // inspection: get all tools
    // ========================================================================
    static size_t _tools_a_( const GaudiAlgorithm*, Tools& tools );
    static size_t _tools_t_( const GaudiTool*, Tools& tools );
    static size_t _tools_a_( const IAlgorithm*, Tools& tools );
    static size_t _tools_t_( const IAlgTool*, Tools& tools );
    // ========================================================================
  };
  // ==========================================================================
} //                                               end of namespace GaudiPython
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIPYTHON_ALGDECORATORS_H
