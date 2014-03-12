// $Id: TupleDecorator.h,v 1.9 2007/08/07 14:05:33 marcocle Exp $
// ============================================================================
#ifndef GAUDIPYTHON_TUPLEDECORATOR_H
#define GAUDIPYTHON_TUPLEDECORATOR_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/Vector4DTypes.h"
#include "GaudiKernel/SymmetricMatrixTypes.h"
#include "GaudiKernel/GenericMatrixTypes.h"
#include "GaudiKernel/GenericVectorTypes.h"
#include "GaudiKernel/Time.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/TupleID.h"
#include "GaudiAlg/TupleObj.h"
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/ITupleTool.h"
#include "GaudiAlg/GaudiTupleAlg.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
#include "GaudiPython/Vector.h"
// ============================================================================
// Forward declarations
// ============================================================================
namespace CLHEP
{
  class HepGenMatrix ;
  class HepVector    ;
}
// ============================================================================
namespace GaudiPython
{
  // ==========================================================================
  /** @class TupleDecorator TupleDecorator.h GaudiPython/TupleDecorator.h
   *  Simple class which performs the decoration of the standard N-Tuple
   *  @see Tuples::Tuple
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2005-08-04
   */
  class GAUDI_API TupleDecorator
  {
  public:
    // ========================================================================
    /// accessors to internal
    static INTuple*         nTuple ( const Tuples::Tuple&  tuple ) ;
    // ========================================================================
    /// accessors to internal
    static NTuple::Tuple*   ntuple ( const Tuples::Tuple&  tuple ) ;
    // ========================================================================
    /// status of the tuple
    static bool             valid  ( const Tuples::Tuple&  tuple ) ;
    // ========================================================================
    /// commit the row
    static StatusCode       write  ( const Tuples::Tuple&  tuple ) ;
    // ========================================================================
  public: // primitives
    // ========================================================================
    /// more or less simple columns:    long
    static StatusCode column
    ( const Tuples::Tuple& tuple ,
      const std::string&   name  ,
      const int            value ) ;
    // ========================================================================
    /// more or less simple columns:    long
    static StatusCode column
    ( const Tuples::Tuple& tuple ,
      const std::string&   name  ,
      const int            value ,
      const int            minv  ,
      const int            maxv  ) ;
    // ========================================================================
    /// more or less simple columns:    double
    static StatusCode column
    ( const Tuples::Tuple& tuple ,
      const std::string&   name  ,
      const double         value ) ;
    // ========================================================================
    /// more or less simple columns:    bool
    static StatusCode column
    ( const Tuples::Tuple& tuple ,
      const std::string&   name  ,
      const bool           value ) ;
    // ========================================================================
    /// more or less simple columns:    long long
    static StatusCode column_ll
    ( const Tuples::Tuple&     tuple ,
      const std::string&       name  ,
      const long long          value ) ;
    // ========================================================================
    /// more or less simple columns:    unsigned long long
    static StatusCode column_ull
    ( const Tuples::Tuple&     tuple ,
      const std::string&       name  ,
      const unsigned long long value ) ;
    // ========================================================================
  public:  // event tag collections
    // ========================================================================
    /// more or less simple columns:    IOpaqueAddress
    static StatusCode column
    ( const Tuples::Tuple&  tuple ,
      const std::string&    name  ,
      IOpaqueAddress*       value ) ;
    // ========================================================================
    /// more or less simple columns:    IOpaqueAddress
    static StatusCode column
    ( const Tuples::Tuple&  tuple ,
      IOpaqueAddress*       value ) ;
    // ========================================================================
  public: // 4D kinematics
    // ========================================================================
    /// Advanced columns: LorentzVector
    static StatusCode column
    ( const Tuples::Tuple&             tuple ,
      const std::string&               name  ,
      const Gaudi::LorentzVector&      value ) ;
    // ========================================================================
  public: // 3D geometry
    // ========================================================================
    /// Advanced columns: 3D-vector
    static StatusCode column
    ( const Tuples::Tuple&             tuple ,
      const std::string&               name  ,
      const Gaudi::XYZVector&          value ) ;
    // ========================================================================
    /// Advanced columns: 3D-points
    static StatusCode column
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::XYZPoint&          value ) ;
    // ========================================================================
  public:   // floating size arrays
    // ========================================================================
    /// Advanced columns: floating-size arrays
    static StatusCode farray
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const std::vector<double>&      data   ,
      const std::string&              length ,
      const size_t                    maxv   ) ;
    // ========================================================================
  public:   // floating-size matrices
    // ========================================================================
    /// Advanced columns: floating-size matrices
    static StatusCode fmatrix
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const GaudiPython::Matrix&      data   ,
      const Tuples::TupleObj::MIndex  cols   , // fixed !!!
      const std::string&              length ,
      const size_t                    maxv   ) ;
    // ========================================================================
    /// Advanced columns: floating-size matrices
    static StatusCode fmatrix
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const GaudiUtils::VectorMap<int,double>& info   ,
      const std::string&              length ,
      const size_t                    maxv   ) ;
    // ========================================================================
  public:   // fixed size arrays
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const std::vector<double>&      data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector1&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector2&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector3&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector4&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector5&           data   ) ;
    // ========================================================================
  public:   // fixed size matrices
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector6&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector7&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector8&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const Gaudi::Vector9&           data   ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const GaudiPython::Matrix&      data  ,
      const Tuples::TupleObj::MIndex  cols  ) ; // fixed !!!
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix2x2&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix3x3&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix4x4&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix5x5&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix6x6&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix7x7&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix8x8&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix9x9&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix1x1&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix1x3&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix1x5&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix1x6&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix4x3&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix3x4&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix3x5&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix3x6&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix2x3&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: non-square matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Matrix3x2&         value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix1x1&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix2x2&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix3x3&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix4x4&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix5x5&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix6x6&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix7x7&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix8x8&      value ) ;
    // ========================================================================
    /// Advanced columns: fixed size matrices: symmetric matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::SymMatrix9x9&      value ) ;
    // ========================================================================
  public:   // some auxillary  objects
    // ========================================================================
    /// advanced column: time
    static StatusCode column
    ( const Tuples::Tuple&            tuple ,
      const std::string&              name  ,
      const Gaudi::Time&              value ) ;
    /// advanced column: time
    static StatusCode column
    ( const Tuples::Tuple&            tuple ,
      const Gaudi::Time&              value ) ;
    // ========================================================================
  public:   // CLHEP: should we keep it ?
    // ========================================================================
    /// Advanced columns: fixed size arrays
    static StatusCode array
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const CLHEP::HepVector&         data   ) ;
    /// Advanced columns: floating-size arrays
    static StatusCode farray
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const CLHEP::HepVector&         data   ,
      const std::string&              length ,
      const size_t                    maxv   ) ;
    /// Advanced columns: fixed size matrices
    static StatusCode matrix
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const CLHEP::HepGenMatrix&      data   ) ;
    /// Advanced columns: floating-size matrices
    static StatusCode fmatrix
    ( const Tuples::Tuple&            tuple  ,
      const std::string&              name   ,
      const CLHEP::HepGenMatrix&      data   ,
      const Tuples::TupleObj::MIndex  cols   , // fixed !!!
      const std::string&              length ,
      const size_t                    maxv   ) ;
    // ========================================================================
  } ;
  // ==========================================================================
  /** @class TupleAlgDecorator TupleDecorator.h GaudiPython/TupleDecorator.h
   *  Simple class to perform the "decoration" of Tuples in Python/ROOT
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2005-08-17
   */
  class GAUDI_API TupleAlgDecorator
  {
  public:
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const GaudiTupleAlg&     algo                         ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const GaudiTupleAlg&     algo                         ,
      const GaudiAlg::TupleID& ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const GaudiTupleAlg&     algo                         ,
      const int                ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const GaudiTupleAlg&     algo                         ,
      const std::string&       ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const GaudiTupleAlg&     algo                         ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const GaudiTupleAlg&     algo                         ,
      const GaudiAlg::TupleID& ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const GaudiTupleAlg&     algo                         ,
      const int                ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const GaudiTupleAlg&     algo                         ,
      const std::string&       ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple ) ;
    // ========================================================================
  } ;
  // ==========================================================================
  /** @class TupleToolDecorator TupleDecorator.h GaudiPython/TupleDecorator.h
   *  Simple class to perform the "decoration" of Tuples in Python/ROOT
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2005-08-17
   */
  class GAUDI_API TupleToolDecorator
  {
  public:
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const ITupleTool&        tool                         ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const ITupleTool&        tool                         ,
      const GaudiAlg::TupleID& ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const ITupleTool&        tool                         ,
      const int                ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       nTuple
    ( const ITupleTool&        tool                         ,
      const std::string&       ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const ITupleTool&        tool                         ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.evtCol ( title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const ITupleTool&        tool                         ,
      const GaudiAlg::TupleID& ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const ITupleTool&        tool                         ,
      const int                ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
    /// get n-tuple (book-on-demand)
    static Tuples::Tuple       evtCol
    ( const ITupleTool&        tool                         ,
      const std::string&       ID                           ,
      const std::string&       title                        ,
      const CLID&              clid  = CLID_ColumnWiseTuple )
    { return tool.nTuple ( ID , title , clid ) ; }
    // ========================================================================
  } ;
  // ==========================================================================
} // end of namespace GaudiPython
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIPYTHON_TUPLEDECORATOR_H
// ============================================================================
