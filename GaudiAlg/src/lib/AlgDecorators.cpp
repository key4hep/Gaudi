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
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiTool.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/AlgDecorators.h"
// ============================================================================
// Disable warnings on gcc
// ============================================================================
#if defined( __GNUC__ )
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
// ============================================================================
/** @file
 *  Implementation file for "Decorators"
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-03
 */
// ============================================================================
/*  get the tool from GaudiAlgorithm
 *  @param alg GaudiAlgorithm
 *  @param type tool type
 *  @param name tool name
 *  @param parent tool parent
 *  @param create flag to create
 *  @return the tool
 */
// ============================================================================
IAlgTool* GaudiPython::AlgDecorator::tool_( const GaudiAlgorithm* alg, const std::string& type, const std::string& name,
                                            const IInterface* parent, const bool create ) {
  return alg ? alg->tool<IAlgTool>( type, name, parent, create ) : nullptr;
}
// ============================================================================
/*  get the tool from GaudiAlgorithm
 *  @param alg GaudiAlgorithm
 *  @param typeAndName tool type/name
 *  @param parent tool parent
 *  @param create flag to create
 *  @return the tool
 */
// ============================================================================
IAlgTool* GaudiPython::AlgDecorator::tool_( const GaudiAlgorithm* alg, const std::string& typeAndName,
                                            const IInterface* parent, const bool create ) {
  return alg ? alg->tool<IAlgTool>( typeAndName, parent, create ) : nullptr;
}
// ============================================================================
/*  get the service from GaudiAlgorithm
 *  @param alg GaudiAlgorithm
 *  @param name service name
 *  @param create flag to create
 *  @return the tool
 */
// ============================================================================
IInterface* GaudiPython::AlgDecorator::svc_( const GaudiAlgorithm* alg, const std::string& name, const bool create ) {
  return alg ? alg->svc<IInterface>( name, create ) : nullptr;
}
// ============================================================================
// get the counter by name
// ============================================================================
StatEntity* GaudiPython::AlgDecorator::_counter_a_( const GaudiAlgorithm* cmp, const std::string& name ) {
  return cmp ? &( cmp->counter( name ) ) : nullptr; // RETURN
}
// ============================================================================
// get the counter by name
// ============================================================================
StatEntity* GaudiPython::AlgDecorator::_counter_t_( const GaudiTool* cmp, const std::string& name ) {
  return cmp ? &( cmp->counter( name ) ) : nullptr; // RETURN
}
// ============================================================================
// get the counter by name
// ============================================================================
StatEntity* GaudiPython::AlgDecorator::_counter_a_( const IAlgorithm* cmp, const std::string& name ) {
  return cmp ? _counter_a_( dynamic_cast<const GaudiAlgorithm*>( cmp ), name ) : nullptr;
}
// ============================================================================
// get the counter by name
// ============================================================================
StatEntity* GaudiPython::AlgDecorator::_counter_t_( const IAlgTool* cmp, const std::string& name ) {
  return cmp ? _counter_t_( dynamic_cast<const GaudiTool*>( cmp ), name ) : nullptr;
}
// ============================================================================
// get all tools
// ============================================================================
size_t GaudiPython::AlgDecorator::_tools_a_( const GaudiAlgorithm* cmp, GaudiPython::AlgDecorator::Tools& tools ) {
  tools.clear();
  if ( cmp ) tools = cmp->tools();
  return tools.size(); // RETURN
}
// ============================================================================
// get all tools
// ============================================================================
size_t GaudiPython::AlgDecorator::_tools_t_( const GaudiTool* cmp, GaudiPython::AlgDecorator::Tools& tools ) {
  tools.clear();
  if ( cmp ) tools = cmp->tools();
  return tools.size(); // RETURN
}
// ============================================================================
// get all tools
// ============================================================================
size_t GaudiPython::AlgDecorator::_tools_a_( const IAlgorithm* cmp, GaudiPython::AlgDecorator::Tools& tools ) {
  tools.clear();
  return cmp ? _tools_a_( dynamic_cast<const GaudiAlgorithm*>( cmp ), tools ) : 0;
}
// ============================================================================
// get all tools
// ============================================================================
size_t GaudiPython::AlgDecorator::_tools_t_( const IAlgTool* cmp, GaudiPython::AlgDecorator::Tools& tools ) {
  tools.clear();
  return cmp ? _tools_t_( dynamic_cast<const GaudiTool*>( cmp ), tools ) : 0;
}
// ============================================================================
/*  check the data in Transient Event Store
 *  @param alg          GaudiAlgorithm
 *  @param location     data location in TES
 *  @param useRoonInTes flag to respect RootInTes
 *  @return the data
 */
// ============================================================================
bool GaudiPython::AlgDecorator::exist( const GaudiAlgorithm* alg, const std::string& location,
                                       const bool useRootInTes ) {
  return alg ? alg->exist<DataObject>( alg->evtSvc(), location, useRootInTes ) : false;
}
// ============================================================================
/*  get the data from Transient Event Store
 *  @param alg          GaudiAlgorithm
 *  @param location     data location in TES
 *  @param useRoonInTes flag to respect RootInTes
 *  @return the data
 */
// ============================================================================
DataObject* GaudiPython::AlgDecorator::get_( const GaudiAlgorithm* alg, const std::string& location,
                                             const bool useRootInTes ) {
  return alg ? alg->get<DataObject>( alg->evtSvc(), location, useRootInTes ) : nullptr;
}
// ============================================================================
// Re-enable warnings on gcc
// ============================================================================
#if defined( __GNUC__ )
#  pragma GCC diagnostic pop
#endif

// ============================================================================
// The END
// ============================================================================
