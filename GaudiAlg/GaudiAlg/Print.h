// $Id: Print.h,v 1.5 2008/04/03 22:13:13 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_PRINT_H
#define GAUDIALG_PRINT_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/MsgStream.h"
// ============================================================================
#include <string>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/HistoID.h"
#include "GaudiAlg/TupleID.h"
// ============================================================================
// forward declarations
// ============================================================================
namespace AIDA
{
  class IHistogram   ;
  class IHistogram1D ;
  class IHistogram2D ;
  class IHistogram3D ;
  class IProfile1D ;
  class IProfile2D ;
}
class StatEntity ;
class INTuple    ;
// ============================================================================
/** @file
 *  collection of useful utilities to print certain objects
 *  (currently used for implementation in class GaudiAlgorithm,
 *  GaudiTool, GaudiHistoAlg, GaudiHistoTool, GaudiTupleAlg,
 *  GaudiTupleTool
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-04
 */
namespace GaudiAlg
{
  /** @class  Print Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IHistogram1D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IHistogram
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print
  {
  public:
    static const std::string& location
    ( const AIDA::IHistogram*   aida ) ;
  } ;
  /** @class  Print1D Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IHistogram1D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IHistogram1D
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print1D
  {
  public:
    static void        print
    ( MsgStream &               stream ,
      const AIDA::IHistogram1D* aida   ,
      const GaudiAlg::HistoID&  ID     ) ;
    //
    static std::string toString
    ( const AIDA::IHistogram1D* aida  ,
      const GaudiAlg::HistoID&  ID    ) ;
  } ;
  /** @class  Print2D Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IHistogram1D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IHistogram2D
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print2D
  {
  public:
    //
    static void print
    ( MsgStream &               stream ,
      const AIDA::IHistogram2D* aida   ,
      const GaudiAlg::HistoID&  ID     ) ;
    //
    static std::string toString
    ( const AIDA::IHistogram2D* aida  ,
      const GaudiAlg::HistoID&  ID    ) ;
  } ;
  /** @class  Print3D Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IHistogram1D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IHistogram3D
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print3D
  {
  public:
    //
    static void print
    ( MsgStream & stream,
      const AIDA::IHistogram3D* aida ,
      const GaudiAlg::HistoID&  ID   ) ;
    //
    static std::string toString
    ( const AIDA::IHistogram3D* aida  ,
      const GaudiAlg::HistoID&  ID    ) ;
  } ;
  /** @class  Print1DProf Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IProfile1D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IProfile1D
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print1DProf
  {
  public:
    //
    static void        print
    ( MsgStream & stream,
      const AIDA::IProfile1D* aida ,
      const GaudiAlg::HistoID&  ID   ) ;
    //
    static std::string toString
    ( const AIDA::IProfile1D* aida  ,
      const GaudiAlg::HistoID&  ID    ) ;
  } ;
  /** @class  Print2DProf Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print IProfile2D
   *  (currently used for implementation of class
   *  GaudiHistoAlg and class GaudiHistoTool)
   *  @see AIDA::IProfile2D
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API Print2DProf
  {
  public:
    //
    static void       print
    ( MsgStream & stream,
      const AIDA::IProfile2D* aida ,
      const GaudiAlg::HistoID&  ID   ) ;
    //
    static std::string toString
    ( const AIDA::IProfile2D* aida  ,
      const GaudiAlg::HistoID&  ID    ) ;
  } ;
  // ==========================================================================
  /** @class  PrintTuple Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print INTuple object
   *  (currently used for implementation of class
   *  GaudiTupleAlg and class GaudiTupleTool)
   *  @see INTuple
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API PrintTuple
  {
  public :
    //
    static std::string print
    ( const INTuple*           tuple ,
      const GaudiAlg::TupleID& ID    ) ;
    //
    static std::string print
    ( const INTuple*           tuple ) ;
  } ;
  // ==========================================================================
  /** @class  PrintStat Print.h GaudiAlg/Print.h
   *  collection of useful utilities to print StatEntity
   *  (currently used for implementation of class
   *  GaudiAlgorithm and class GaudiTool)
   *  @see StatEntity
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-04
   */
  class GAUDI_API PrintStat
  {
  public:
    static std::string   print
    ( const StatEntity&  stat ,
      const std::string& tag  ) ;
  } ;
  // ==========================================================================
} // end of namespace GaudiAlg

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_PRINT_H
// ============================================================================
