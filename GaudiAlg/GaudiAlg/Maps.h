// $Id: Maps.h,v 1.7 2006/12/10 19:11:56 leggett Exp $
#ifndef GAUDIALG_MAPS_H
#define GAUDIALG_MAPS_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HashMap.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/HistoID.h"
#include "GaudiAlg/TupleID.h"
// ============================================================================
/* @file Maps.h
 *
 *  Definitions of few useful hash-maps, classes and typedefs
 *  used for classes GaudiHistos and GaudiTuples.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
// ============================================================================
// forward declarations
namespace AIDA
{
  class IHistogram1D ;
  class IHistogram2D ;
  class IHistogram3D ;
  class IProfile1D   ;
  class IProfile2D   ;
}
namespace Tuples { class TupleObj; }
// ============================================================================
/** @namespace GaudiAlg
 *
 *  Definitions of few useful hash-maps, classes and typedefs
 *  used for classes GaudiHistos and GaudiTuples.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
// ============================================================================
namespace GaudiAlg
{
  // ==========================================================================
  // ============================ 1D ==========================================
  // ==========================================================================
  /// the actual type for  (title)->(1D histogram) map
  typedef GaudiUtils::HashMap
  <
    std::string          ,                // Key
    AIDA::IHistogram1D*                   // Value
    > Histo1DMapTitle    ;
  /// the actual type for (ID)->(1D histogram) map
  typedef GaudiUtils::HashMap
  <
    HistoID              ,                 // Key
    AIDA::IHistogram1D*                    // Value
    > Histo1DMapID       ;
  // ==========================================================================
  // ============================ 2D ==========================================
  // ==========================================================================
  /// the actual type for  (title)->(2D histogram) map
  typedef GaudiUtils::HashMap
  <
    std::string                         , // Key
    AIDA::IHistogram2D*                   // Value
    > Histo2DMapTitle ;
  /// the actual type for (ID)->(2D histogram) map
  typedef GaudiUtils::HashMap
  <
    HistoID                              , // Key
    AIDA::IHistogram2D*                   // Value
    > Histo2DMapID ;
  // ==========================================================================
  // ============================ 2D ==========================================
  // ==========================================================================
  /// the actual type for  (title)->(3D histogram) map
  typedef GaudiUtils::HashMap
  <
    std::string                         , // Key
    AIDA::IHistogram3D*                   // Value
    > Histo3DMapTitle ;
  /// the actual type for (ID)->(3D histogram) map
  typedef GaudiUtils::HashMap
  <
    HistoID                              , // Key
    AIDA::IHistogram3D*                   // Value
    > Histo3DMapID ;
  // ==========================================================================
  // ============================ 1D Prof =====================================
  // ==========================================================================
  /// the actual type for  (title)->(1D Profile Histogram) map
  typedef GaudiUtils::HashMap
  <
    std::string                         , // Key
    AIDA::IProfile1D*                     // Value
    > Profile1DMapTitle ;
  /// the actual type for (ID)->(1D Profile histogram) map
  typedef GaudiUtils::HashMap
  <
    HistoID                             , // Key
    AIDA::IProfile1D*                     // Value
    > Profile1DMapID ;
  // ==========================================================================
  // ============================ 2D Prof =====================================
  // ==========================================================================
  /// the actual type for  (title)->(2D Profile Histogram) map
  typedef GaudiUtils::HashMap
  <
    std::string                         , // Key
    AIDA::IProfile2D*                     // Value
    > Profile2DMapTitle ;
  /// the actual type for (ID)->(2D Profile histogram) map
  typedef GaudiUtils::HashMap
  <
    HistoID                             , // Key
    AIDA::IProfile2D*                     // Value
    > Profile2DMapID ;
  // ==========================================================================
  // ============================ NTuples =====================================
  // ==========================================================================  
  /// the actual type for  (title)->(tuple) map
  typedef GaudiUtils::HashMap
  <
    std::string                         , // Key
    Tuples::TupleObj*                     // Value
    > TupleMapTitle ;
  /// the actual type for  (ID)->(tuple) map
  typedef GaudiUtils::HashMap
  <
    TupleID                              , // Key
    Tuples::TupleObj*                     // Value
    > TupleMapID  ;
  // ==========================================================================
}
// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIALG_MAPS_H
// ============================================================================
