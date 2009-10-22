
//---------------------------------------------------------------------------------
/** @file SymmetricMatrixTypes.h
 *
 *  Symmetric Matrix typedefs
 *
 *  CVS Log :-
 *  $Id: SymmetricMatrixTypes.h,v 1.1 2006/12/08 15:00:00 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_SYMMETRICMATRIXTYPES_H 
#define GAUDIKERNEL_SYMMETRICMATRIXTYPES_H 1

// Include files
#include "Math/SMatrix.h"

/** @namespace Gaudi
 *  
 *  General Gaudi namespace
 * 
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi 
{

  typedef ROOT::Math::SMatrix<double, 1, 1, 
                              ROOT::Math::MatRepSym<double,1> > SymMatrix1x1; ///< Symmetrix 1x1 matrix (double)
  typedef ROOT::Math::SMatrix<double, 2, 2,
                              ROOT::Math::MatRepSym<double,2> > SymMatrix2x2; ///< Symmetrix 2x2 matrix (double)
  typedef ROOT::Math::SMatrix<double, 3, 3,
                              ROOT::Math::MatRepSym<double,3> > SymMatrix3x3; ///< Symmetrix 3x3 matrix (double)
  typedef ROOT::Math::SMatrix<double, 4, 4,
                              ROOT::Math::MatRepSym<double,4> > SymMatrix4x4; ///< Symmetrix 4x4 matrix (double)
  typedef ROOT::Math::SMatrix<double, 5, 5,
                              ROOT::Math::MatRepSym<double,5> > SymMatrix5x5; ///< Symmetrix 5x5 matrix (double)
  typedef ROOT::Math::SMatrix<double, 6, 6,
                              ROOT::Math::MatRepSym<double,6> > SymMatrix6x6; ///< Symmetrix 6x6 matrix (double)
  typedef ROOT::Math::SMatrix<double, 7, 7,
                              ROOT::Math::MatRepSym<double,7> > SymMatrix7x7; ///< Symmetrix 7x7 matrix (double)
  typedef ROOT::Math::SMatrix<double, 8, 8,
                              ROOT::Math::MatRepSym<double,8> > SymMatrix8x8; ///< Symmetrix 8x8 matrix (double)
  typedef ROOT::Math::SMatrix<double, 9, 9,
                              ROOT::Math::MatRepSym<double,9> > SymMatrix9x9; ///< Symmetrix 9x9 matrix (double)

  typedef ROOT::Math::SMatrix<float, 1, 1, 
                              ROOT::Math::MatRepSym<float,1> > SymMatrix1x1F; ///< Symmetrix 1x1 matrix (float)
  typedef ROOT::Math::SMatrix<float, 2, 2,
                              ROOT::Math::MatRepSym<float,2> > SymMatrix2x2F; ///< Symmetrix 2x2 matrix (float)
  typedef ROOT::Math::SMatrix<float, 3, 3,
                              ROOT::Math::MatRepSym<float,3> > SymMatrix3x3F; ///< Symmetrix 3x3 matrix (float)
  typedef ROOT::Math::SMatrix<float, 4, 4,
                              ROOT::Math::MatRepSym<float,4> > SymMatrix4x4F; ///< Symmetrix 4x4 matrix (float)
  typedef ROOT::Math::SMatrix<float, 5, 5,
                              ROOT::Math::MatRepSym<float,5> > SymMatrix5x5F; ///< Symmetrix 5x5 matrix (float)
  typedef ROOT::Math::SMatrix<float, 6, 6,
                              ROOT::Math::MatRepSym<float,6> > SymMatrix6x6F; ///< Symmetrix 6x6 matrix (float)
  typedef ROOT::Math::SMatrix<float, 7, 7,
                              ROOT::Math::MatRepSym<float,7> > SymMatrix7x7F; ///< Symmetrix 7x7 matrix (float)
  typedef ROOT::Math::SMatrix<float, 8, 8,
                              ROOT::Math::MatRepSym<float,8> > SymMatrix8x8F; ///< Symmetrix 8x8 matrix (float)
  typedef ROOT::Math::SMatrix<float, 9, 9,
                              ROOT::Math::MatRepSym<float,9> > SymMatrix9x9F; ///< Symmetrix 9x9 matrix (float)


};
#endif // KERNEL_SYMMETRICMATRIXTYPES_H
