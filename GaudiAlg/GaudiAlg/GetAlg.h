// $Id: GetAlg.h,v 1.2 2007/11/20 13:01:06 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GETALG_H
#define GAUDIALG_GETALG_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernnel
// ============================================================================
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// forward declarations
// ============================================================================
class IAlgContextSvc ;
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    // ========================================================================
    /** @class AlgSelector
     *  Simple interface class for selection of algorithms
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    class GAUDI_API AlgSelector
    {
    public:
      /// the only one essential method:
      virtual bool operator() ( const IAlgorithm* ) const = 0 ;
      // virtual destructor
      virtual ~AlgSelector () ;
    };
    // ========================================================================
    /** @class AlgTypeSelector
     *  The trivial selector of algorithm by type
     *  @see Gaudi::Utils::AlgSelector
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    template <class TYPE>
    class GAUDI_API AlgTypeSelector : public AlgSelector
    {
    public:
      /// the only one essential method:
      virtual bool operator() ( const IAlgorithm* a ) const
      { return dynamic_cast<const TYPE*>( a ) != 0; }
    } ;
    // ========================================================================
    template <class TYPE>
    class AlgTypeSelector<TYPE*>       : public AlgTypeSelector<TYPE>   {} ;
    // ========================================================================
    template <class TYPE>
    class AlgTypeSelector<const TYPE*> : public AlgTypeSelector<TYPE>   {} ;
    // ========================================================================
    template <class TYPE>
    class AlgTypeSelector<TYPE&>       : public AlgTypeSelector<TYPE>   {} ;
    // ========================================================================
    template <class TYPE>
    class AlgTypeSelector<const TYPE&> : public AlgTypeSelector<TYPE>   {} ;
    // ========================================================================
    template <class TYPE>
    class AlgTypeSelector<const TYPE>  : public AlgTypeSelector<TYPE>   {} ;
    // ========================================================================
    /** @class AlgNameSelector
     *  The trivial selector of algorithm by type
     *  @see Gaudi::Utils::AlgSelector
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    class GAUDI_API AlgNameSelector : public AlgSelector
    {
    public:
      /// constructor form the name
      AlgNameSelector ( const std::string& name  ) : m_name ( name ) {}
      /// the only one essential method:
      virtual bool operator() ( const IAlgorithm* a ) const
      { return 0 != a ? a->name() == m_name : false ; }
    private:
      // the default constructor is disabled
      AlgNameSelector() ;
    private:
      // algorithm name
      std::string m_name ; ///< algorithm name
    } ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
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
    GAUDI_API IAlgorithm* getAlgorithm
    ( const IAlgContextSvc* svc ,
      const AlgSelector&    sel ) ;
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
    GAUDI_API IAlgorithm* getAlgorithm
    ( const std::vector<IAlgorithm*>& lst ,
      const AlgSelector&              sel ) ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GETALG_H
// ============================================================================
