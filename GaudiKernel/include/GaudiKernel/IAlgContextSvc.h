/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/SmartIF.h>
#include <vector>

/** @class IAlgContextSvc
 *  An abstract interface for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-07 (modified)
 */
class GAUDI_API IAlgContextSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAlgContextSvc, 4, 0 );
  /// the actual type of algorithm' stack
  typedef std::vector<IAlgorithm*> Algorithms;

public:
  /// set     the currently executing algorithm  ("push_back")
  virtual StatusCode setCurrentAlg( IAlgorithm* a, const EventContext& context ) = 0;
  /// remove the algorithm                       ("pop_back")
  virtual StatusCode unSetCurrentAlg( IAlgorithm* a, const EventContext& context ) = 0;
  /// accessor to current algorithm:
  virtual IAlgorithm* currentAlg() const = 0;
  /// get the stack of executed algorithms
  virtual const Algorithms& algorithms() const = 0;
};

namespace Gaudi {
  namespace Utils {
    /** @class AlgContext
     *  Helper "sentry" class to automatize the safe register/unregister
     *  the algorithm's context
     *
     *  Typical explicit usage:
     *
     *  @code
     *
     *   StatusCode MyAlg::execute()
     *    {
     *       IAlgContextSvc* acs = ... ;
     *       // define the context
     *       Gaudi::Utils::AlgContext sentry ( this , acs ) ;
     *
     *       ...
     *
     *       return StatusCode::SUCCESS ;
     *    }
     *
     *  @endcode
     *
     *  Note: for the regular job the context is properly
     *  defined with the help of corresponding auditor
     *  AlgContextAuditor. This helper class is needed only
     *  if one needs to ensure that the algorithm must register
     *  its context independently on job/auditor configuration
     *
     *  @see AlgContextAuditor
     *  @author Vanya BELYAEV ibelyaev@phys.syr.edu
     *  @date   2007-03-07
     */
    class GAUDI_API AlgContext final {
    public:
      /** constructor from the service and the algorithm
       *  Internally invokes IAlgContextSvc::setCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       *  @param alg pointer to the current algorithm
       *  @param svc pointer to algorithm context service
       *  @param context current event context
       */
      AlgContext( IAlgorithm* alg, IAlgContextSvc* svc, const EventContext& context );

      /** constructor from the service and the algorithm (single thread case)
       *  Internally invokes IAlgContextSvc::setCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       *  @param alg pointer to the current algorithm
       *  @param svc pointer to algorithm context service
       */
      AlgContext( IAlgorithm* alg, IAlgContextSvc* svc );

      /// Prevent use of temporary EventContext as current context.
      /// @see [gaudi/Gaudi#73](https://gitlab.cern.ch/gaudi/Gaudi/issues/73)
      AlgContext( IAlgorithm* alg, IAlgContextSvc* svc, EventContext&& context ) = delete;

      /** destructor
       *  Internally invokes IAlgContextSvc::unSetCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       */
      ~AlgContext();

    private:
      // copy constructor is disabled
      AlgContext( const AlgContext& right ) = delete;
      // assignement operator is disabled
      AlgContext& operator=( const AlgContext& right ) = delete;

    private:
      SmartIF<IAlgContextSvc> m_svc;
      SmartIF<IAlgorithm>     m_alg;
      const EventContext&     m_context;
    };
  } // namespace Utils
} // namespace Gaudi
