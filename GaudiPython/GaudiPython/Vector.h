#ifndef GAUDIPYTHON_VECTOR_H
#define GAUDIPYTHON_VECTOR_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <functional>
#include <vector>
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
// ============================================================================

namespace GaudiPython
{
  /// useful type definition for  implicit loos
  typedef std::vector<double> Vector;
  typedef Vector              Row;
  typedef std::vector<Row>    Matrix;

  /** @struct _identity
   *  It is here due to 'missing'(?) std::identity
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-02
   */
  template <class TYPE>
  struct _identity : public std::unary_function<TYPE, TYPE> {
    inline Vector::value_type operator()( const Vector::value_type& value ) const { return value; }
  };

} // end of namespace GaudiPython
#endif // GAUDIPYTHON_VECTOR_H
