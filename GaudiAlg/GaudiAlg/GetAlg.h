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
#ifndef GAUDIALG_GETALG_H
#define GAUDIALG_GETALG_H 1
// ============================================================================
// Include files
// ============================================================================
#include <type_traits>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// forward declarations
// ============================================================================
class IAlgContextSvc;
// ============================================================================
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** @class AlgSelector
     *  Simple interface class for selection of algorithms
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    class GAUDI_API AlgSelector {
    public:
      /// the only one essential method:
      virtual bool operator()( const IAlgorithm* ) const = 0;
      // virtual destructor
      virtual ~AlgSelector() = default;
    };
    // ========================================================================
    /** @class AlgTypeSelector
     *  The trivial selector of algorithm by type
     *  @see Gaudi::Utils::AlgSelector
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    template <class TYPE>
    class GAUDI_API AlgTypeSelector : public AlgSelector {
    public:
      /// the only one essential method:
      bool operator()( const IAlgorithm* a ) const override {
        using TYPE_ = std::decay_t<TYPE>;
        using CTYPE = std::add_const_t<TYPE_>;
        using cptr  = std::add_pointer_t<CTYPE>;
        return dynamic_cast<cptr>( a );
      }
    };
    // ========================================================================
    /** @class AlgNameSelector
     *  The trivial selector of algorithm by type
     *  @see Gaudi::Utils::AlgSelector
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    class GAUDI_API AlgNameSelector : public AlgSelector {
    public:
      AlgNameSelector() = delete;
      /// constructor form the name
      AlgNameSelector( std::string name ) : m_name( std::move( name ) ) {}
      /// the only one essential method:
      bool operator()( const IAlgorithm* a ) const override { return a && a->name() == m_name; }

    private:
      // algorithm name
      std::string m_name; ///< algorithm name
    };
    // ========================================================================
  } // namespace Utils
} // end of namespace Gaudi
// ============================================================================
namespace Gaudi {
  namespace Utils {
    // ========================================================================
    /** simple function to get the algorithm from Context Service
     *
     *  @code
     *
     *  // get the selector
     *  const AlgSelector& selector = ... ;
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  // get the appropriate algorithm:
     *  IAlgorithm* alg = getAlgorithm ( svc , selector ) ;
     *
     *  @endcode
     *
     *  @see IAlgContextSvc
     *  @see Gaudi::Utils::AlgSelector
     *  @param svc pointer to Algororithm Contetx Service
     *  @param sel the selection functor
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API IAlgorithm* getAlgorithm( const IAlgContextSvc* svc, const AlgSelector& sel );
    // ========================================================================
    /** simple function to get the algorithm from Context Service
     *
     *  @code
     *
     *  // get the selector
     *  const AlgSelector& selector = ... ;
     *
     *  // get the list of algorithms:
     *  const std::vector<IAlgorithm*>& algs = ... ;
     *
     *  // get the appropriate algorithm:
     *  IAlgorithm* alg = getAlgorithm ( algs  , selector ) ;
     *
     *  @endcode
     *
     *  @see IAlgContextSvc
     *  @see Gaudi::Utils::AlgSelector
     *  @param lst list of the algorithms
     *  @param sel the selection functor
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API IAlgorithm* getAlgorithm( const std::vector<IAlgorithm*>& lst, const AlgSelector& sel );
    // ========================================================================
  } // namespace Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GETALG_H
