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
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/Kernel.h>
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Utils {
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
     *   - <c> has_key </c> (for Python 2)
     *
     *  Thus it behaves practically identically
     *  with builtin-type <c>dict</c>
     *
     *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
     *  @date 2010-02-20
     */
    class GAUDI_API MapBase {
    public:
      // ======================================================================
      /// virtual destructor
      virtual ~MapBase();
      // ======================================================================
    protected:
      // ======================================================================
      /** throw std::out_of_range exception
       *  @see std::out_of_range
       */
      void throw_out_of_range_exception() const;
      // ======================================================================
    };
    // ========================================================================
  } // namespace Utils
  // ==========================================================================
} // namespace Gaudi
// ============================================================================
// The END
// ============================================================================
