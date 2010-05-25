// $Id: $
// ============================================================================
#ifndef GAUDIKERNEL_MAPBASE_H 
#define GAUDIKERNEL_MAPBASE_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Utils 
  {
    // ========================================================================
    /** @class MapBase GaudiKernel/MapBase.h
     *  Helper base-class to allow the generic Python-decoration for
     *  all "map-like" classes in Gaudi.
     *
     *  The corresponding Python class provide the useful iterators, 
     *  look-up, etc. This base-class in python is decorated with 
     *  the following methods
     *   - <c> __len__       </c>
     *   - <c> __iter__      </c>
     *   - <c> __iteritems__ </c>
     *   - <c> __contains__  </c>
     *   - <c> __repr__      </c>
     *   - <c> __setitem__   </c>
     *   - <c> __delitem__   </c>
     *   - <c> __str__       </c>
     *   - <c> __repr__      </c>
     *   - <c> keys    </c>
     *   - <c> items   </c>
     *   - <c> values  </c>
     *   - <c> get     </c>
     *   - <c> has_key </c>
     *
     *  Thus it behaves practically identically 
     *  with builtin-type <c>dict</c>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2010-02-20
     */
    class GAUDI_API MapBase 
    {
    public:
      // ======================================================================
      /// virtual destructor
      virtual ~MapBase () ;
      // ======================================================================
    protected:
      // ======================================================================
      /** throw std::out_of_range exception
       *  @see std::out_of_range 
       */
      void throw_out_of_range_exception() const ;
      // ======================================================================
    };
    // ========================================================================
  }
  // ==========================================================================
}
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIKERNEL_MAPBASE_H
// ============================================================================
