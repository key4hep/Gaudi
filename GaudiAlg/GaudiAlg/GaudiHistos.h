// $Id: GaudiHistos.h,v 1.11 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDIHISTOS_H
#define GAUDIALG_GAUDIHISTOS_H 1
// ============================================================================
/* @file GaudiHistos.h
 *
 *  Header file for class : GaudiHistos
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-08-08
 */
// ============================================================================
// Include files#
// ============================================================================
// STD& STL
// ============================================================================
#include <limits>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HistoProperty.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Maps.h"
// ============================================================================
// Forward declarations
namespace AIDA
{
  class IHistogram1D;
  class IHistogram2D;
  class IHistogram3D;
  class IProfile1D;
  class IProfile2D;
}
// ============================================================================
/** @class GaudiHistos GaudiHistos.h GaudiAlg/GaudiHistos.h
 *
 *  Templated base class providing common histogramming methods for
 *  GaudiAlgorithm and GaudiTool like classes.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-08-08
 */
template <class PBASE>
class GAUDI_API GaudiHistos: public PBASE
{
public:
  // ==========================================================================
  /// the actual type for histogram identifier
  typedef GaudiAlg::HistoID                HistoID;
  // ==========================================================================
  /// the actual type for (ID)->(1D histogram) mapping
  typedef GaudiAlg::Histo1DMapID           Histo1DMapID;
  /// the actual type for title->(1D histogram) mapping
  typedef GaudiAlg::Histo1DMapTitle        Histo1DMapTitle;
  // ==========================================================================
  /// the actual type for (ID)->(2D histogram) mapping
  typedef GaudiAlg::Histo2DMapID           Histo2DMapID;
  /// the actual type for title->(2D  histogram) mapping
  typedef GaudiAlg::Histo2DMapTitle        Histo2DMapTitle;
  // ==========================================================================
  /// the actual type for (ID)->(3D histogram) mapping
  typedef GaudiAlg::Histo3DMapID           Histo3DMapID;
  /// the actual type for title->(3D histogram) mapping
  typedef GaudiAlg::Histo3DMapTitle        Histo3DMapTitle;
  // ==========================================================================
  /// the actual type for (ID)->(1D profile histogram) mapping
  typedef GaudiAlg::Profile1DMapID         Profile1DMapID;
  /// the actual type for title->(1D profile histogram) mapping
  typedef GaudiAlg::Profile1DMapTitle      Profile1DMapTitle;
  // ==========================================================================
  /// the actual type for (ID)->(2D profile histogram) mapping
  typedef GaudiAlg::Profile2DMapID         Profile2DMapID;
  /// the actual type for title->(2D profile histogram) mapping
  typedef GaudiAlg::Profile2DMapTitle      Profile2DMapTitle;
  // ==========================================================================
  /// Edges for variable binning
  typedef GaudiAlg::HistoBinEdges          HistoBinEdges;
  // ==========================================================================
public:
  // ==========================================================================
  // ================================= 1D Histograms ==========================
  // ================================= Fixed Binning ==========================
  // ==========================================================================
  /** fill the 1D histogram (book on demand)
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  This example illustrates the filling of the histogram
   *  titled <tt>"InvariantMass"</tt> with value @c mass .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c low equal to 2.5, parameters @c high equal to 3.5
   *  and @c bins equal to  100.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param value value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D histogram (book on demand)
   *
   *  Wrapper method for the equivalent plot1D method.
   *  Retained for backwards compatibility, please use plot1D instead.
   *
   *  @param value value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  inline AIDA::IHistogram1D*  plot
  ( const double        value        ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const
  {
    return plot1D ( value, title, low, high, bins, weight );
  }
  // ==========================================================================
  /** fill the 1D histogram (book on demand)
   *
   *  @code
   *
   *     /// get the histogram descriptor:
   *     const Gaudi::Histo1DDef& hdef = ... ;
   *
   *     const double mass = ... ;
   *     plot1D ( mass , hdef ) ;
   *
   *  @endcode
   *
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title.
   *
   *  The histogram descriptor comes e.g. from the component properties.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param value value to be filled
   *  @param hdef histogram descriptor
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  plot1D
  ( const double             value        ,
    const Gaudi::Histo1DDef& hdef         ,
    const double             weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D histogram (book on demand)
   *
   *  Wrapper method for the equivalent plot1D method.
   *  Retained for backwards compatibility, please use plot1D instead.
   *
   *  @param value value to be filled
   *  @param hdef histogram descriptor
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  inline AIDA::IHistogram1D*  plot
  ( const double             value        ,
    const Gaudi::Histo1DDef& hdef         ,
    const double             weight = 1.0 ) const
  {
    return plot1D ( value, hdef, weight );
  }
  // ==========================================================================
  /** fill the 1D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , 15 , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D histogram ID=15
   *  titled <tt>"Invariant Mass"</tt> with value @c mass .
   *
   *  If the histogram with given ID does not exist yet
   *  it will be automatically booked with parameters
   *  @c low  equal to  2.5, parameters @c high equal to 3.5
   *  and @c bins equal to  100.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , "mass" , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , "subdir/mass" , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset (property "HistoOffSet")
   *  if one need to combine these techniques together.
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param value value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  // ==========================================================================
  AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D histogram with forced ID assignment (book on demand)
   *
   *  Wrapper method for the equivalent plot1D method.
   *  Retained for backwards compatibility, please use plot1D instead.
   *
   *  @param value value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  inline AIDA::IHistogram1D*  plot
  ( const double        value        ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const
  {
    return plot1D ( value, ID, title, low, high, bins, weight );
  }
  // ==========================================================================
  /** fill the 1D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     // get the histogram descriptor
   *     const Gaudi::Histo1DDef& hdef = ... ;
   *
   *     // get the histogram ID
   *     const HistoID ID = ... ;
   *
   *     const double mass = ... ;
   *     plot1D ( mass , ID , hdef  )
   *
   *  @endcode
   *
   *  @see AIDA::IHistogram1D
   *
   *  The histogram descriptor comes e.g. from component properties
   *
   *  @param value value to be filled
   *  @param ID histogram identifier
   *  @param hdef histogram descriptor
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  plot1D
  ( const double             value        ,
    const HistoID&           ID           ,
    const Gaudi::Histo1DDef& hdef         ,
    const double             weight = 1.0 ) const ;
  // ==========================================================================
 /** fill the 1D histogram (book on demand)
   *
   *  Wrapper method for the equivalent plot1D method.
   *  Retained for backwards compatibility, please use plot1D instead.
   *
   *  @param value value to be filled
   *  @param ID histogram identifier
   *  @param hdef histogram descriptor
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  inline AIDA::IHistogram1D*  plot
  ( const double             value        ,
    const HistoID&           ID           ,
    const Gaudi::Histo1DDef& hdef         ,
    const double             weight = 1.0 ) const
  {
    return plot1D ( value, ID, hdef, weight );
  }
  /** fill the 1D histogram with information from
   *  [first,last) sequence
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  plot( sin                 , // function
   *        v.begin() , v.end() , // sequence
   *        " bla-bla "         , // title
   *        -1. , 1.0           , // low and high limits
   *        100                 ) // number of bins
   *
   *  @endcode
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  Sequence, objects and function can be non-trivial:
   *  @code
   *
   *  Particles* p = ... ;
   *
   *  plot( PT                  , // function
   *        p->begin()  , p->end() , // sequence
   *        " bla-bla "         , // title
   *        -1. , 1.0           , // low and high limits
   *        100                 ) ; // number of bins
   *
   *  @endcode
   *  where <c>PT</c> can be any function or function object
   *  for which the expression <c>PT(p)</c> , with <c>p</c> of type
   *  <c>Particle*</c> have some sense and can be evaluated to
   *  the values, which is convertible to <c>double</c>
   *
   *  @attention
   *  These plot methods using iterator ranges are more efficient than
   *  the simplier "value" only methods, since the associated histogram
   *  only requires locating from internal storage once per loop, as opposed
   *  to once per fill for the simplier functions. It is recommended to use
   *  these whenever possible.
   *
   *  @see AIDA::IHistogram1D
   *  @param func  function to be plotted
   *  @param first begin of the sequence
   *  @param last  end of the sequence
   *  @param title histogram title
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins for histogram
   */
  template <class FUNCTION,class OBJECT>
  inline AIDA::IHistogram1D*   plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins  = 100  ) const
  {
    AIDA::IHistogram1D* h(0);
    if ( produceHistos() )
    {
      // retrieve or book the histogram
      h = histo1D ( title ) ;
      if ( 0 == h )     { h = book1D  ( title , low , high , bins ); }
      // fill histogram
      while( first != last && 0 != h  )
      { h = fill ( h , func( *first ) , 1.0 , title  ) ; ++first ; }
    }
    return h ;
  }
  // ==========================================================================
  /** fill the 1D histogram with forced ID and information from
   *  [first,last) sequence
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  // Example with numeric ID
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        100 , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 );  // number of bins
   *
   *  // Example with literal ID
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        "sin" , " bla-bla " ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 );  // number of bins
   *
   *  @endcode
   *
   *  Sequence, objects and function can be non-trivial:
   *
   *  @code
   *
   *  Particles* p = ... ;
   *
   *  plot( PT                  ,   // function
   *        p->begin()   , p->end() , // sequence
   *        100 , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ) ; // number of bins
   *
   *  @endcode
   *
   *  @attention
   *  These plot methods using iterator ranges are more efficient than
   *  the simplier "value" only methods, since the associated histogram
   *  only requires locating from internal storage once per loop, as opposed
   *  to once per fill for the simplier functions. It is recommended to use
   *  these whenever possible.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param func  function to be plotted
   *  @param first begin of the sequence
   *  @param last  end of the sequence
   *  @param ID    histogram identifier
   *  @param title histogram title
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins for histogram
   */
  template <class FUNCTION,class OBJECT>
  inline AIDA::IHistogram1D*   plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins  = 100  ) const
  {
    AIDA::IHistogram1D* h(0);
    if ( produceHistos() )
    {
      // retrieve or book the histogram
      h = histo1D ( ID ) ;
      if ( 0 == h )     { h = book1D  ( ID , title , low , high , bins ); }
      // fill histogram
      while( first != last && 0 != h )
      { h = fill( h , func( *first ) , 1.0 , title  ) ; ++first ; }
    }
    return h;
  }
  // ==========================================================================
  /** book and fill the 1D histogram with information from
   *  [first,last) sequence with given weight
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        " bla-bla "         ,   // title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        tanh                );  // weight function
   *
   *  @endcode
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  Sequence, objects and function can be non-trivial:
   *  @code
   *
   *  Particles* p = ... ;
   *
   *  plot( PT                  ,  // function
   *        p->begin()  , p->end() , // sequence
   *        " bla-bla "         ,   // title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        MASS                ) ; // weight function
   *
   *  @endcode
   *  where <c>PT</c> and <c>MASS</c> can be any function
   *  or function object
   *  for which the expressions <c>PT(p)</c> and
   *  <c>MASS</c> with <c>p</c> of type
   *  <c>Particle*</c> have some sense and can be evaluated to
   *  the values, which is convertible to <c>double</c>
   *
   *  @attention
   *  These plot methods using iterator ranges are more efficient than
   *  the simplier "value" only methods, since the associated histogram
   *  only requires locating from internal storage once per loop, as opposed
   *  to once per fill for the simplier functions. It is recommended to use
   *  these whenever possible.
   *
   *  @see AIDA::IHistogram1D
   *  @param first  begin of the sequence
   *  @param last   end of the sequence
   *  @param title  histogram title
   *  @param func   function to be plotted
   *  @param low    low limit for histogram
   *  @param high   high limit for histogram
   *  @param bins   number of bins for histogram
   *  @param weight weight function
   */
  template <class FUNCTION,class OBJECT,class WEIGHT>
  inline AIDA::IHistogram1D*   plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins         ,
    const WEIGHT&       weight       ) const
  {
    AIDA::IHistogram1D* h(0);
    if ( produceHistos() )
    {
      // retrieve or book the histogram
      h = histo1D ( title ) ;
      if ( 0 == h ) { h = book1D  ( title , low , high , bins ); }
      // fill histogram
      while ( first != last && 0 != h )
      { h = fill ( h                 ,
                   func   ( *first ) ,
                   weight ( *first ) , title  ) ; ++first ; }
    }
    return h;
  }
  // ==========================================================================
  /** book and fill the 1D histogram with forced ID and information from
   *  [first,last) sequence with given weight
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  // example with numerical ID
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        100 , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        sinh                );  // weight function
   *
   *  // example with literal ID
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        "sin" , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        sinh                );  // weight function
   *
   *  @endcode
   *
   *  @attention no checks for NaN or Finite are performed!
   *
   *  Sequence, objects and function can be non-trivial:
   *
   *  @code
   *
   *  Particles* p = ... ;
   *
   *  plot( PT                  ,   // function
   *        p->begin()   , p->end() , // sequence
   *        100 , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        MASS                ) ; // weight function
   *
   *  @endcode
   *  where <c>PT</c> and <c>MASS</c> can be any function
   *  or function object
   *  for which the expressions <c>PT(p)</c> and
   *  <c>MASS</c> with <c>p</c> of type
   *  <c>Particle*</c> have some sense and can be evaluated to
   *  the values, which is convertible to <c>double</c>
   *
   *  @attention
   *  These plot methods using iterator ranges are more efficient than
   *  the simplier "value" only methods, since the associated histogram
   *  only requires locating from internal storage once per loop, as opposed
   *  to once per fill for the simplier functions. It is recommended to use
   *  these whenever possible.
   *
   *  @see AIDA::IHistogram1D
   *  @param first begin of the sequence
   *  @param last  end of the sequence
   *  @param ID    histogram identifier
   *  @param title histogram title
   *  @param func  function to be plotted
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins for histogram
   *  @param weight weight function
   */
  template <class FUNCTION,class OBJECT,class WEIGHT>
  inline AIDA::IHistogram1D*   plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins         ,
    const WEIGHT&       weight       ) const
  {
    AIDA::IHistogram1D* h(0);
    if ( produceHistos() )
    {
      // retrieve or book the histogram
      h = histo1D ( ID ) ;
      if ( 0 == h ) { h = book1D  ( ID , title , low , high , bins ); }
      // fill histogram
      while( first != last && 0 != h )
      { h  = fill ( h                 ,
                    func   ( *first ) ,
                    weight ( *first ) , title  ) ; ++first ; }
    }
    return h ;
  }
  // ==========================================================================
  // ================================= 1D Histograms ==========================
  // =============================== Variable Binning =========================
  // ==========================================================================
  /** Fill the 1D variable binning histogram (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass = ... ;
   *     plot1D( mass , "Invariant Mass" , edges )
   *
   *  @endcode
   *
   *  This example illustrates the filling of the histogram
   *  titled <tt>"InvariantMass"</tt> with value @c mass.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param value value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  plot1D
  ( const double         value        ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D variable binning histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass = ... ;
   *     plot1D( mass , 15 , "Invariant Mass" , edges )
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D histogram ID=15
   *  titled <tt>"Invariant Mass"</tt> with value @c mass.
   *
   *  If the histogram with given ID does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass = ... ;
   *     plot1D( mass , "mass" , "Invariant Mass" , edges )
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass = ... ;
   *     plot1D( mass , "subdir/mass" , "Invariant Mass" , edges )
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset (property "HistoOffSet")
   *  if one need to combine these techniques together.
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference.
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param value value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D histogram
   */
  // ==========================================================================
  AIDA::IHistogram1D*  plot1D
  ( const double         value        ,
    const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  // ================================= 2D Histograms ==========================
  // ==========================================================================
  /** fill the 2D histogram (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2,
   *            "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D histogram
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt> with
   *  values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 100 and @c binsY equal to 200.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param binsX  number of bins in x
   *  @param binsY  number of bins in y
   *  @param weight weight
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D*  plot2D
  ( const double        valueX       ,
    const double        valueY       ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const unsigned long binsX  = 50  ,
    const unsigned long binsY  = 50  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  /** fill the 2D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, 15,
   *             "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D histogram ID=15
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt>
   *  with values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 100 and @c binsY equal to 200.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, "mass",
   *             "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, "subdir/mass",
   *             "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset  (property "HistoOffSet")
   *  if one need to combine these techniques together
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param ID     Histogram ID to use
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param binsX  number of bins in x
   *  @param binsY  number of bins in y
   *  @param weight weight
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D*  plot2D
  ( const double        valueX       ,
    const double        valueY       ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const unsigned long binsX  = 50  ,
    const unsigned long binsY  = 50  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  // ================================= 2D Histograms ==========================
  // =============================== Variable Binning =========================
  // ==========================================================================
  /** Fill the 2D variable binning histogram (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2,
   *            "Invariant Mass2 versus Mass1" , edgesX, edgesY );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D histogram
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt>
   *  with values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param weight weight
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D*  plot2D
  ( const double         valueX       ,
    const double         valueY       ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 2D variable histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, 15,
   *             "Invariant Mass2 versus Mass1", edgesX, edgesY );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D histogram ID=15
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt>
   *  with values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, "mass",
   *             "Invariant Mass2 versus Mass1", edgesX, edgesY );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, "subdir/mass",
   *             "Invariant Mass2 versus Mass1", edgesX, edgesY );
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset  (property "HistoOffSet")
   *  if one need to combine these techniques together
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param ID     Histogram ID to use
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param weight weight
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D*  plot2D
  ( const double         valueX       ,
    const double         valueY       ,
    const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  // ================================= 3D Histograms ==========================
  // ==========================================================================
  /** fill the 3D histogram (book on demand)
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z, "Space Points" ,
   *            2.5 , 3.5 ,
   *            4.5 , 5.5 ,
   *            6.5 , 7.5 ,
   *            10, 20, 30 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 3D histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c lowZ equal to 6.5, @c highZ equal to 7.5,
   *  @c binsX equal to 10, @c binsY equal to 20 and @c binsZ equal to 30.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param lowZ  low z limit for histogram
   *  @param highZ high z limit for histogram
   *  @param binsX number of bins in x
   *  @param binsY number of bins in y
   *  @param binsZ number of bins in z
   *  @param weight weight
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  plot3D
  ( const double        valueX       ,
    const double        valueY       ,
    const double        valueZ       ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const double        lowZ         ,
    const double        highZ        ,
    const unsigned long binsX  = 10  ,
    const unsigned long binsY  = 10  ,
    const unsigned long binsZ  = 10  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  /** fill the 3D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z,
   *            15 , "Space Points" ,
   *             2.5 ,3.5, 4.5, 5.5, 6.5, 7.5, 10, 20, 30 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 3D histogram ID=15
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c lowZ equal to 6.5, @c highZ equal to 7.5,
   *  @c binsX equal to 10, @c binsY equal to 20 and @c binsZ equal to 30.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z,
   *            "space", "Space Points" ,
   *              2.5 ,3.5, 4.5, 5.5, 6.5, 7.5, 10, 20, 30 );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "space".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z,
   *             "subdir/space", "Space Points" ,
   *             2.5 ,3.5, 4.5, 5.5, 6.5, 7.5, 10, 20, 30 );
   *
   *  @endcode
   *
   *  Which will create the histogram "space" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset  (property "HistoOffSet")
   *  if one need to combine these techniques together
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param ID     Histogram ID to use
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param lowZ  low z limit for histogram
   *  @param highZ high z limit for histogram
   *  @param binsX number of bins in x
   *  @param binsY number of bins in y
   *  @param binsZ number of bins in z
   *  @param weight weight
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  plot3D
  ( const double        valueX       ,
    const double        valueY       ,
    const double        valueZ       ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const double        lowZ         ,
    const double        highZ        ,
    const unsigned long binsX  = 10  ,
    const unsigned long binsY  = 10  ,
    const unsigned long binsZ  = 10  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  // ================================= 3D Histograms ==========================
  // =============================== Variable Binning =========================
  // ==========================================================================
  /** Fill the 3D variable binning histogram (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const GaudiAlg::HistoBinEdges edgesZ = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z, "Space Points", edgesX, edgesY, edgesZ );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 3D histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param edgesZ The histogram z bin edges
   *  @param weight weight
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  plot3D
  ( const double         valueX       ,
    const double         valueY       ,
    const double         valueZ       ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const HistoBinEdges& edgesZ       ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  // ==========================================================================
  /** fill the 3D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const GaudiAlg::HistoBinEdges edgesZ = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z, "Space Points", edgesX, edgesY, edgesZ );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 3D histogram ID=15
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges and
   *  histogram ID.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const GaudiAlg::HistoBinEdges edgesZ = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z,
   *             "space", "Space Points" ,
   *             edgesX, edgesY, edgesZ );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "space".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const GaudiAlg::HistoBinEdges edgesZ = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     plot3D( X, Y, Z,
   *             "subdir/space", "Space Points" ,
   *             edgesX, edgesY, edgesZ );
   *
   *  @endcode
   *
   *  Which will create the histogram "space" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @attention
   *  If the histogram with given ID is already booked
   *  through automatic assignment of histogram ID,
   *  the error will not be detected.
   *  Therefore it is recommended
   *  to use non-trivial histogram ID offset  (property "HistoOffSet")
   *  if one need to combine these techniques together
   *  It is still desirable to use the unique histogram title
   *  to avoid a bad interference
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param ID     Histogram ID to use
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param edgesZ The histogram z bin edges
   *  @param weight weight
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  plot3D
  ( const double         valueX       ,
    const double         valueY       ,
    const double         valueZ       ,
    const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const HistoBinEdges& edgesZ       ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  // ================================= 1D Profile =============================
  // ================================= Fixed binning ==========================
  // ==========================================================================
  /** fill the 1D profile histogram (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2, "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 100 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D profile histogram
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt>
   *   with values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c binsX equal to 100
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IProfile1D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param opt    the options, used for evaluation of errors
   *  @param lowY   the min cut-off for y-values
   *  @param highY  the max cut-off for y-values
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D* profile1D
  ( const double        valueX       ,
    const double        valueY       ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const unsigned long binsX  = 100 ,
    const std::string&  opt    = ""  ,
    const double        lowY   = -std::numeric_limits<double>::max() ,
    const double        highY  =  std::numeric_limits<double>::max() ,
    const double        weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D profile histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *             15, "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 100 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D profile histogram with ID=15
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt> with
   *  values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c binsX equal to 100
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *             "mass", "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 100 );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *        "subdir/mass", "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 100 );
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @see AIDA::IProfile1D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param opt    the options, used for evaluation of errors
   *  @param lowY   the min cut-off for y-values
   *  @param highY  the max cut-off for y-values
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D* profile1D
  ( const double        valueX       ,
    const double        valueY       ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const unsigned long binsX  = 100 ,
    const std::string&  opt    = ""  ,
    const double        lowY   = -std::numeric_limits<double>::max() ,
    const double        highY  =  std::numeric_limits<double>::max() ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  // ================================= 1D Profile =============================
  // ============================== Variable binning ==========================
  // ==========================================================================
  /** fill the 1D variable binning profile histogram (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2, "Invariant Mass2 versus Mass1", edges );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D profile histogram
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt>
   *   with values @c mass1 and @c mass2 .
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IProfile1D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D* profile1D
  ( const double         valueX       ,
    const double         valueY       ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 1D variable binning profile histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *             15, "Invariant Mass2 versus Mass1", edges );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 1D profile histogram with ID=15
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt> with
   *  values @c mass1 and @c mass2 .
   *
   *  If the histogram with given ID does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *             "mass", "Invariant Mass2 versus Mass1", edges );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edges = ...;
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     profile1D( mass1, mass2,
   *        "subdir/mass", "Invariant Mass2 versus Mass1", edges );
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @see AIDA::IProfile1D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D* profile1D
  ( const double         valueX       ,
    const double         valueY       ,
    const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ,
    const double         weight = 1.0 ) const;
  // ==========================================================================
  // ================================= 2D Profile =============================
  // ==========================================================================
  /** fill the 2D profile histogram (book on demand)
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2( X, Y, Z, "Space Points" ,2.5 ,3.5, 4.5, 5.5, 10, 20 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D profile histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 10, @c binsY equal to 20.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IProfile2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param binsX number of bins in x
   *  @param binsY number of bins in y
   *  @param weight weight
   *  @return pointer to AIDA 2D profile histogram
   */
  AIDA::IProfile2D* profile2D
  ( const double        valueX       ,
    const double        valueY       ,
    const double        valueZ       ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const unsigned long binsX  = 50  ,
    const unsigned long binsY  = 50  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  /** fill the 2D profile histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z, 15, "Space Points" ,2.5 ,3.5, 4.5, 5.5, 10, 20 );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D profile histogram with ID=15
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 10, @c binsY equal to 20.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z, "space", "Space Points" ,2.5 ,3.5, 4.5, 5.5, 10, 20 );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "space".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z,
   *                "subdir/space", "Space Points" ,2.5 ,3.5, 4.5, 5.5, 10, 20 );
   *
   *  @endcode
   *
   *  Which will create the histogram "space" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @see AIDA::IProfile2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX  low x limit for histogram
   *  @param highX high x limit for histogram
   *  @param lowY  low y limit for histogram
   *  @param highY high y limit for histogram
   *  @param binsX number of bins in x
   *  @param binsY number of bins in y
   *  @param weight weight
   *  @return pointer to AIDA 2D profile histogram
   */
  AIDA::IProfile2D* profile2D
  ( const double        valueX       ,
    const double        valueY       ,
    const double        valueZ       ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const unsigned long binsX  = 50  ,
    const unsigned long binsY  = 50  ,
    const double        weight = 1.0 ) const;
  // ==========================================================================
  // ================================= 2D Profile =============================
  // ============================== Variable binning ==========================
  // ==========================================================================
  /** fill the 2D variable binning profile histogram (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z, "Space Points", edgesX, edgesY );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D profile histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given title does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  @attention
   *  The histogram will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first histogram booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given histogram will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the histograms are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given histogram could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @see AIDA::IProfile2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram x bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile2D* profile2D
  ( const double         valueX       ,
    const double         valueY       ,
    const double         valueZ       ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const double         weight = 1.0 ) const ;
  // ==========================================================================
  /** fill the 2D variable binning profile histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const GaudiAlg::HistoBinEdges edgesX = ...;
   *     const GaudiAlg::HistoBinEdges edgesY = ...;
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2( X, Y, Z, "Space Points", edgesX, edgesY );
   *
   *  @endcode
   *
   *  This example illustrates the filling of the 2D profile histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *
   *  If the histogram with given ID does not exist yet
   *  it will be automatically booked with the given histogram bin edges.
   *
   *  It is also possible to use literal IDs. For example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z, "space", "Space Points", edgesX, edgesY );
   *
   *  @endcode
   *
   *  Will book the same histogram, using the id "mass".
   *
   *  It is also possible using literal IDs, to place histograms in
   *  sub-directories from the main histogram directory, using for
   *  example :-
   *
   *  @code
   *
   *     const double X = ... ;
   *     const double Y = ... ;
   *     const double Z = ... ;
   *     profile2D( X, Y, Z,
   *                "subdir/space", "Space Points", edgesX, edgesY );
   *
   *  @endcode
   *
   *  Which will create the histogram "mass" in the sub-directory "subdir".
   *  Histograms can also be created in sub-directories with numeric IDs if
   *  IDs such as "subdir/1" are used.
   *
   *  @see AIDA::IProfile2D
   *
   *  @param valueX x value to be filled
   *  @param valueY y value to be filled
   *  @param valueZ z value to be filled
   *  @param ID histogram identifier
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param weight weight
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile2D* profile2D
  ( const double         valueX       ,
    const double         valueY       ,
    const double         valueZ       ,
    const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const double         weight = 1.0 ) const;
  // ==========================================================================

public: // 1D Fixed

  // ==========================================================================
  /** book the 1D histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  book1D
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const ;
  // ==========================================================================
  /** book the 1D histogram
   *
   *  Wrapper method for the equivalent book1D method.
   *  Retained for backwards compatibility, please use book1D instead.
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA 1D histogram
   */
  inline AIDA::IHistogram1D*  book
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return book1D( title, low, high, bins );
  }
  // ==========================================================================
  /** book the 1D histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IHistogram1D
   *  @param hdef histogram description/definition
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  book
  ( const Gaudi::Histo1DDef& hdef ) const ;
  // ==========================================================================
  /** book the 1D histogram with forced ID
   *
   *  @see IHistogram1D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA histogram
   */
  AIDA::IHistogram1D*  book1D
  ( const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const ;
  // ==========================================================================
  /** book the 1D histogram with forced ID
   *
   *  Wrapper method for the equivalent book1D method.
   *  Retained for backwards compatibility, please use book1D instead.
   *
   *  @see IHistogram1D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA histogram
   */
  inline AIDA::IHistogram1D*  book
  ( const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return book1D( ID, title, low, high, bins );
  }
  // ==========================================================================
  /** book the 1D histogram with forced ID
   *
   *  @see IHistogram1D
   *  @param ID  unique histogram ID
   *  @param hdef histogram descriptor
   *  @return pointer to AIDA histogram
   */
  inline AIDA::IHistogram1D*  book
  ( const HistoID&           ID   ,
    const Gaudi::Histo1DDef& hdef ) const ;
  // ==========================================================================

public: // 1D Variable

  // ==========================================================================
  /** book the 1D variable binning histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  book1D
  ( const std::string&   title       ,
    const HistoBinEdges& edges       ) const ;
  // ==========================================================================
  /** book the 1D variable binning histogram with given ID
   *
   *  @see AIDA::IHistogram1D
   *
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D*  book1D
  ( const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ) const ;
  // ==========================================================================

public: // 2D Fixed

  // ==========================================================================
  /** book the 2D histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram2D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX   low x limit for histogram
   *  @param highX  high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param lowY   low y limit for histogram
   *  @param highY  high y limit for histogram
   *  @param binsY  number of bins in y
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D*  book2D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const ;
  // ==========================================================================
  /** book the 2D histogram with forced ID
   *
   *  @see IHistogram2D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA histogram
   */
  AIDA::IHistogram2D*  book2D
  ( const HistoID&      ID            ,
    const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const ;
  // ==========================================================================

public: // 2D Variable

  // ==========================================================================
  /** book the 2D variable binning histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D * book2D
  ( const std::string&   title    ,
    const HistoBinEdges& edgesX   ,
    const HistoBinEdges& edgesY   ) const ;
  // ==========================================================================
  /** book the 2D variable binning histogram with given ID
   *
   *  @see AIDA::IHistogram2D
   *
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D * book2D
  ( const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ) const ;
  // ==========================================================================

public: // 3D Fixed

  // ==========================================================================
  /** book the 3D histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram3D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX   low x limit for histogram
   *  @param highX  high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param lowY   low y limit for histogram
   *  @param highY  high y limit for histogram
   *  @param binsY  number of bins in y
   *  @param lowZ   low y limit for histogram
   *  @param highZ  high y limit for histogram
   *  @param binsZ  number of bins in y
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  book3D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  10 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  10 ,
    const double        lowZ    =   0 ,
    const double        highZ   = 100 ,
    const unsigned long binsZ   =  10 ) const ;
  // ==========================================================================
  /** book the 3D histogram with forced ID
   *
   *  @see IHistogram3D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX   low x limit for histogram
   *  @param highX  high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param lowY   low y limit for histogram
   *  @param highY  high y limit for histogram
   *  @param binsY  number of bins in y
   *  @param lowZ   low y limit for histogram
   *  @param highZ  high y limit for histogram
   *  @param binsZ  number of bins in y
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D*  book3D
  ( const HistoID&      ID            ,
    const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  10 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  10 ,
    const double        lowZ    =   0 ,
    const double        highZ   = 100 ,
    const unsigned long binsZ   =  10 ) const ;
  // ==========================================================================

public: // 3D Variable

  // ==========================================================================
  /** book the 3D variable binning histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param edgesZ The histogram z bin edges
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D * book3D
  ( const std::string&   title    ,
    const HistoBinEdges& edgesX   ,
    const HistoBinEdges& edgesY   ,
    const HistoBinEdges& edgesZ   ) const ;
  // ==========================================================================
  /** book the 3D variable binning histogram with given ID
   *
   *  @see AIDA::IHistogram3D
   *
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX The histogram x bin edges
   *  @param edgesY The histogram y bin edges
   *  @param edgesZ The histogram z bin edges
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D * book3D
  ( const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edgesX       ,
    const HistoBinEdges& edgesY       ,
    const HistoBinEdges& edgesZ       ) const ;
  // ==========================================================================

public: // 1D Fixed Profiles

  // ==========================================================================
  /** book the 1D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param opt    the options, used for evaluation of errors
   *  @param lowY   the min cut-off for y-values
   *  @param highY  the max cut-off for y-values
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D*  bookProfile1D
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ,
    const std::string&  opt    = ""  ,
    const double        lowY   = -std::numeric_limits<double>::max() ,
    const double        highY  =  std::numeric_limits<double>::max() ) const;
  // ==========================================================================
  /** book the 1D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @param opt    the options, used for evaluation of errors
   *  @param lowY   the min cut-off for y-values
   *  @param highY  the max cut-off for y-values
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D*  bookProfile1D
  ( const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ,
    const std::string&  opt    = ""  ,
    const double        lowY   = -std::numeric_limits<double>::max() ,
    const double        highY  =  std::numeric_limits<double>::max() ) const;
 // ==========================================================================

public: // 1D Variable Profiles

  // ==========================================================================
  /** book the 1D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D*  bookProfile1D
  ( const std::string&  title        ,
    const HistoBinEdges&        edges        ) const;
  // ==========================================================================
  /** book the 1D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edges The histogram bin edges
   *  @return pointer to AIDA 1D profile histogram
   */
  AIDA::IProfile1D*  bookProfile1D
  ( const HistoID&       ID           ,
    const std::string&   title        ,
    const HistoBinEdges& edges        ) const;
 // ==========================================================================

public: // 2D Profiles

  // ==========================================================================
  /** book the 2D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IProfile2D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX   low x limit for histogram
   *  @param highX  high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param lowY   low y limit for histogram
   *  @param highY  high y limit for histogram
   *  @param binsY  number of bins in y
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IProfile2D*  bookProfile2D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const ;
  // ==========================================================================
  /** book the 2D profile histogram with forced ID
   *
   *  @see AIDA::IProfile2D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param lowX   low x limit for histogram
   *  @param highX  high x limit for histogram
   *  @param binsX  number of bins in x
   *  @param lowY   low y limit for histogram
   *  @param highY  high y limit for histogram
   *  @param binsY  number of bins in y
   *  @return pointer to AIDA histogram
   */
  AIDA::IProfile2D*  bookProfile2D
  ( const HistoID&      ID            ,
    const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const ;
  // ==========================================================================

public: // 2D Profiles

  // ==========================================================================
  /** book the 2D profile histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see AIDA::IProfile2D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX x bin edges
   *  @param edgesY y bin edges
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IProfile2D*  bookProfile2D
  ( const std::string&   title         ,
    const HistoBinEdges& edgesX,
    const HistoBinEdges& edgesY ) const ;
  // ==========================================================================
  /** book the 2D profile histogram with forced ID
   *
   *  @see AIDA::IProfile2D
   *  @param ID  unique histogram ID
   *  @param title histogram title (must be unique within the algorithm)
   *  @param edgesX x bin edges
   *  @param edgesY y bin edges
   *  @return pointer to AIDA histogram
   */
  AIDA::IProfile2D*  bookProfile2D
  ( const HistoID&       ID            ,
    const std::string&   title         ,
    const HistoBinEdges& edgesX,
    const HistoBinEdges& edgesY  ) const ;
  // ==========================================================================

public:

  // ==========================================================================
  /** fill the 1D histogram with the value and weight
   *  @param histo 1D histogram to be filled
   *  @param value value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IHistogram1D* fill
  ( AIDA::IHistogram1D* histo  ,
    const double        value  ,
    const double        weight ,
    const std::string&  title  = "" ) const ;
  // ==========================================================================
  /** fill the 2D histogram with the value and weight
   *  @param histo 2D histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 2D histogram
   */
  AIDA::IHistogram2D* fill
  ( AIDA::IHistogram2D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        weight ,
    const std::string&  title  = "" ) const ;
  // ==========================================================================
  /** fill the 3D histogram with the value and weight
   *  @param histo 3D histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param valueZ z value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 3D histogram
   */
  AIDA::IHistogram3D* fill
  ( AIDA::IHistogram3D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        valueZ ,
    const double        weight ,
    const std::string&  title  = "" ) const ;
  // ==========================================================================
  /** fill the 1D profile histogram with the values and weight
   *  @param histo 1D profile histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IProfile1D* fill
  ( AIDA::IProfile1D*   histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        weight ,
    const std::string&  title  = "" ) const ;
  // ==========================================================================
  /** fill the 2D profile histogram with the values and weight
   *  @param histo 2D profile histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param valueZ z value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 1D histogram
   */
  AIDA::IProfile2D* fill
  ( AIDA::IProfile2D*   histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        valueZ ,
    const double        weight ,
    const std::string&  title  = "" ) const ;
  // ==========================================================================
public:
  // ==========================================================================
  /** access the EXISTING 1D histogram by title
   *  return the pointer to existing 1D histogram or NULL
   */
  inline AIDA::IHistogram1D* histo1D ( const std::string& title  )  const
  {
    Histo1DMapTitle::const_iterator found = histo1DMapTitle().find( title ) ;
    return ( histo1DMapTitle().end() == found ? 0 : found->second );
  }
  // ==========================================================================
  /** access the EXISTING 1D histogram by title
   *
   *  Wrapper method for the equivalent histo1D method.
   *  Retained for backwards compatibility, please use histo1D instead.
   *
   *  return the pointer to existing 1D histogram or NULL
   */
  inline AIDA::IHistogram1D* histo ( const std::string& title  )  const
  {
    return histo1D( title );
  }
  // ==========================================================================
  /** access the EXISTING 2D histogram by title
   *  return the pointer to existing 2D histogram or NULL
   */
  inline AIDA::IHistogram2D* histo2D ( const std::string& title  )  const
  {
    Histo2DMapTitle::const_iterator found = histo2DMapTitle().find( title ) ;
    return ( histo2DMapTitle().end() == found ? 0 : found->second );
  }
  // ==========================================================================
  /** access the EXISTING 3D histogram by title
   *  return the pointer to existing 3D histogram or NULL
   */
  inline AIDA::IHistogram3D* histo3D ( const std::string& title  )  const
  {
    Histo3DMapTitle::const_iterator found = histo3DMapTitle().find( title ) ;
    return ( histo3DMapTitle().end() == found ? 0 : found->second );
  }
  // ==========================================================================
  /** access the EXISTING 1D profile histogram by title
   *  return the pointer to existing 1D profile histogram or NULL
   */
  inline AIDA::IProfile1D* profile1D ( const std::string& title  )  const
  {
    Profile1DMapTitle::const_iterator found = profile1DMapTitle().find( title ) ;
    return ( profile1DMapTitle().end() == found ? 0 : found->second );
  }
  // ==========================================================================
  /** access the EXISTING 2D profile histogram by title
   *  return the pointer to existing 2D profile histogram or NULL
   */
  inline AIDA::IProfile2D* profile2D ( const std::string& title  )  const
  {
    Profile2DMapTitle::const_iterator found = profile2DMapTitle().find( title ) ;
    return ( profile2DMapTitle().end() == found ? 0 : found->second );
  }
  // ==========================================================================
public:
  // ==========================================================================
  /** access the EXISTING 1D histogram by ID
   *  return the pointer to existing 1D histogram or NULL
   */
  AIDA::IHistogram1D* histo1D ( const HistoID&     ID     )  const;
  // ==========================================================================
  /** access the EXISTING 1D histogram by ID
   *
   *  Wrapper method for the equivalent histo1D method.
   *  Retained for backwards compatibility, please use histo1D instead.
   *
   *  return the pointer to existing 1D histogram or NULL
   */
  inline AIDA::IHistogram1D* histo ( const HistoID&     ID     )  const
  {
    return histo1D( ID );
  }
  // ==========================================================================
  /** access the EXISTING 2D histogram by ID
   *  return the pointer to existing 2D histogram or NULL
   */
  AIDA::IHistogram2D* histo2D ( const HistoID&     ID     )  const;
  // ==========================================================================
  /** access the EXISTING 3D histogram by ID
   *  return the pointer to existing 3D histogram or NULL
   */
  AIDA::IHistogram3D* histo3D ( const HistoID&     ID     )  const;
  // ==========================================================================
  /** access the EXISTING 1D profile histogram by ID
   *  return the pointer to existing 1D profile histogram or NULL
   */
  AIDA::IProfile1D* profile1D ( const HistoID&     ID  )  const;
  // ==========================================================================
  /** access the EXISTING 2D profile histogram by ID
   *  return the pointer to existing 2D profile histogram or NULL
   */
  AIDA::IProfile2D* profile2D ( const HistoID&     ID  )  const;
  // ==========================================================================
public:
  // ==========================================================================
  /// check the existence AND validity of the histogram with given title
  inline bool histoExists ( const std::string& title  )  const
  {
    return
      ( 0 != histo     ( title ) ||
        0 != histo2D   ( title ) ||
        0 != histo3D   ( title ) ||
        0 != profile1D ( title ) ||
        0 != profile2D ( title ) );
  }
  // ==========================================================================
  /// check the existence AND validity of the histogram with given title
  inline bool histoExists ( const HistoID&     ID     )  const
  {
    return
      ( 0 != histo     ( ID ) ||
        0 != histo2D   ( ID ) ||
        0 != histo3D   ( ID ) ||
        0 != profile1D ( ID ) ||
        0 != profile2D ( ID ) );
  }
  /// Returns the total number of histograms (of all types) currently booked
  unsigned int totalNumberOfHistos() const;
  // ==========================================================================
public: // trivial & non-trivial accessors
  // ==========================================================================
  /// get the flag for histogram production (property "HistoProduce")
  inline bool               produceHistos () const { return m_produceHistos ; }
  /// get flag to control output level of histograms
  inline bool               fullDetail    () const { return m_fullDetail    ; }
  /// get the flag for NaN checks           (property "HistoCheckForNan")
  inline bool               checkForNaN   () const { return m_checkForNaN   ; }
  /// get the flag for histogram path split (property "HistoSplitDir")
  inline bool               splitHistoDir () const { return m_splitHistoDir ; }
  /// get the value for histogram offset    (property "HistoOffSet")
  inline HistoID::NumericID histoOffSet   () const { return m_histoOffSet ; }
  /// get top-level histogram directory (property "HistoTopDir")
  inline const std::string& histoTopDir   () const { return m_histoTopDir   ; }
  /// get histogram directory           (property "HistoDir")
  inline const std::string& histoDir      () const { return m_histoDir      ; }
  /// get the constructed histogram path
  std::string histoPath () const;
  /// print histograms at finalization ?
  inline bool histosPrint () const  { return m_histosPrint ; }
  /// print histogram counters at finalization ?
  inline bool histoCountersPrint () const  { return m_histoCountersPrint ; }
  /// Use old style sequencial numerical automatically assigned IDs ?
  inline bool useNumericAutoIDs() const { return m_useNumericAutoIDs; }
  // ==========================================================================
  /** perform the actual printout of histograms
   *  @param  level The message level to print at
   *  @return number of active histograms
   */
  int printHistos ( const MSG::Level level = MSG::ALWAYS ) const ;
  // ==========================================================================
  /** get access to the map of all 1D histograms indexed via their title
   *
   *  Using this method one can inspect e.g. a list of active histograms
   *
   *  @code
   *
   *  const Histo1DMapTitle& histos = histo1DMapTitle() ;
   *  // iterate over the map!
   *  for ( Histo1DMapTitle::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram title
   *        const std::string&   title = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram1D* hist  = entry->second ;
   *        if( 0 == hist )  { continue ; }                // ATTENTION!
   *
   *        std::cout << " Histogram title " << title << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *  @attention The map *COULD* contains NULL pointers,
   *     the check before use is mandatory!
   *
   */
  const Histo1DMapTitle & histo1DMapTitle() const { return m_histo1DMapTitle; }
  // ==========================================================================
  /** get access to the map of all 1D histograms index via ID
   *
   *  @code
   *
   *  const Histo1DMap& histos = histo1DMap () ;
   *  // iterate over the map!
   *  for ( Histo1DMap::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram ID
   *        const HistoID        ID = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram1D* h  = entry->second ;
   *        if ( 0 == h ) { continue ;}
   *
   *        std::cout << " Histogram ID    " << ID
   *                  << " Histogram title " << h->title() << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *
   */
  const Histo1DMapID & histo1DMapID () const { return m_histo1DMapID ; }
  // ==========================================================================
  /** get access to the map of all 2D histograms indexed via their title
   *
   *  Using this method one can inspect e.g. a list of active histograms
   *
   *  @code
   *
   *  const Histo2DMapTitle& histos = histo2DMapTitle() ;
   *  // iterate over the map!
   *  for ( Histo2DMapTitle::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram title
   *        const std::string&   title = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram2D* hist  = entry->second ;
   *        if( 0 == hist )  { continue ; }                // ATTENTION!
   *
   *        std::cout << " Histogram title " << title << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *  @attention The map *COULD* contains NULL pointers,
   *     the check before use is mandatory!
   *
   */
  const Histo2DMapTitle & histo2DMapTitle() const { return m_histo2DMapTitle ; }
  // ==========================================================================
  /** get access to the map of 2D histograms index via ID
   *
   *  @code
   *
   *  const Histo2DMapID& histos = histo2DMapID () ;
   *
   *  // iterate over the map!
   *  for ( Histo2DMapID::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram ID
   *        const HistoID        ID = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram2D* h  = entry->second ;
   *        if ( 0 == h ) { continue ;}
   *
   *        std::cout << " Histogram ID    " << ID
   *                  << " Histogram title " << h->title() << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   */
  const Histo2DMapID& histo2DMapID () const { return m_histo2DMapID ; }
  // ==========================================================================
  /** get access to the map of all 3D histograms indexed via their title
   *
   *  Using this method one can inspect e.g. a list of active histograms
   *
   *  @code
   *
   *  const Histo3DMapTitle& histos = histo3DMapTitle() ;
   *  // iterate over the map!
   *  for ( Histo3DMapTitle::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram title
   *        const std::string&   title = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram3D* hist  = entry->second ;
   *        if( 0 == hist )  { continue ; }                // ATTENTION!
   *
   *        std::cout << " Histogram title " << title << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *  @attention The map *COULD* contains NULL pointers,
   *     the check before use is mandatory!
   *
   */
  const Histo3DMapTitle & histo3DMapTitle () const { return m_histo3DMapTitle ; }
  // ==========================================================================
  /** get access to the map of all 3D histograms index via a ID
   *
   *  @code
   *
   *  const Histo3DMapID& histos = histo3DMapID() ;
   *
   *  // iterate over the map!
   *  for ( Histo3DMapID::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram ID
   *        const HistoID        ID = entry->first ;
   *        // histogram itself
   *        AIDA::IHistogram3D* h  = entry->second ;
   *        if ( 0 == h ) { continue ;}
   *
   *        std::cout << " Histogram ID    " << ID
   *                  << " Histogram title " << h->title() << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   */
  const Histo3DMapID & histo3DMapID () const { return m_histo3DMapID; }
  // ==========================================================================
  /** get access to the map of all 1D profile histograms indexed via their title
   *
   *  Using this method one can inspect e.g. a list of active histograms
   *
   *  @code
   *
   *  const Profile1DMapTitle& histos = profile1DMapTitle() ;
   *  // iterate over the map!
   *  for ( Profile1DMapTitle::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram title
   *        const std::string&   title = entry->first ;
   *        // histogram itself
   *        AIDA::IProfile1D* hist  = entry->second ;
   *        if( 0 == hist )  { continue ; }                // ATTENTION!
   *
   *        std::cout << " Histogram title " << title << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *  @attention The map *COULD* contains NULL pointers,
   *     the check before use is mandatory!
   *
   */
  const Profile1DMapTitle & profile1DMapTitle() const { return m_profile1DMapTitle; }
  // ==========================================================================
  /** get access to the map of 1D profile histograms index via a ID
   *
   *  @code
   *
   *  const Profile1DMapID& histos = profile1DMapID() ;
   *
   *  // iterate over the map!
   *  for ( Profile1DMapID::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram ID
   *        const HistoID        ID = entry->first ;
   *        // histogram itself
   *        AIDA::IProfile1D* h  = entry->second ;
   *        if ( 0 == h ) { continue ;}
   *
   *        std::cout << " Histogram ID    " << ID
   *                  << " Histogram title " << h->title() << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   */
  const Profile1DMapID & profile1DMapID () const { return m_profile1DMapID; }
  // ==========================================================================
  /** get access to the map of all 2D profile histograms indexed via their title
   *
   *  Using this method one can inspect e.g. a list of active histograms
   *
   *  @code
   *
   *  const Profile2DMapTitle& histos = profile2DMapTitle() ;
   *  // iterate over the map!
   *  for ( Profile2DMapTitle::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram title
   *        const std::string&   title = entry->first ;
   *        // histogram itself
   *        AIDA::IProfile2D* hist  = entry->second ;
   *        if( 0 == hist )  { continue ; }                // ATTENTION!
   *
   *        std::cout << " Histogram title " << title << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   *  @attention The map *COULD* contains NULL pointers,
   *     the check before use is mandatory!
   *
   */
  const Profile2DMapTitle & profile2DMapTitle() const { return m_profile2DMapTitle; }
  // ==========================================================================
  /** get access to the map of 2D profile histograms index via a ID
   *
   *  @code
   *
   *  const Profile2DMapID& histos = profile2DMapID() ;
   *
   *  // iterate over the map!
   *  for ( Profile2DMapID::const_iterator entry = histos.begin() ;
   *        histos.end() != entry ; ++entry  )
   *     {
   *        // histogram ID
   *        const HistoID        ID = entry->first ;
   *        // histogram itself
   *        AIDA::IProfile2D* h  = entry->second ;
   *        if ( 0 == h ) { continue ;}
   *
   *        std::cout << " Histogram ID    " << ID
   *                  << " Histogram title " << h->title() << std::endl ;
   *
   *     }
   *
   *  @endcode
   *
   */
  const Profile2DMapID & profile2DMapID () const { return m_profile2DMapID; }
  // ==========================================================================
public: // trivial setters
  // ==========================================================================
  /// set the flag for histogram production (property "HistoProduce")
  inline void setProduceHistos ( const bool         val ) { m_produceHistos = val ; }
  /// set flag to control output level of histograms
  inline void setFullDetail    ( const bool         val ) { m_fullDetail    = val ; }
  /// set the flag for NaN checks           (property "HistoCheckForNan")
  inline void setCheckForNaN   ( const bool         val ) { m_checkForNaN   = val ; }
  /// set the flag for histogram path split (property "HistoSplitDir")
  inline void setSplitHistoDir ( const bool         val ) { m_splitHistoDir = val ; }
  /// set a value for histogram offset      (property "HistoOffSet"
  inline void setHistoOffSet   ( const HistoID::NumericID val )
  { m_histoOffSet   = val ; }
  // ==========================================================================
  /// set top-level histogram directory (property "HistoTopDir")
  inline void setHistoTopDir   ( const std::string& val ) { m_histoTopDir   = val ; }
  // ==========================================================================
  /// set histogram directory           (property "HistoDir")
  inline void setHistoDir      ( const std::string& val ) { m_histoDir      = val ; }
  // ==========================================================================
public:
  // ==========================================================================
  /// Algorithm constructor
  GaudiHistos ( const std::string & name,
                ISvcLocator * pSvcLocator );
  // ==========================================================================
  /// Tool constructor
  GaudiHistos ( const std::string& type   ,
                const std::string& name   ,
                const IInterface*  parent );
  // ==========================================================================
  /// Destructor
  virtual ~GaudiHistos() {}
  // ==========================================================================
protected:
  // ==========================================================================
  /** standard initialization method
   *  @return status code
   */
  virtual StatusCode initialize()
#ifdef __ICC
    { return i_ghInitialize(); }
  StatusCode i_ghInitialize()
#endif
  ;
  // ==========================================================================
  /** standard finalization method
   *  @return status code
   */
  virtual StatusCode finalize()
#ifdef __ICC
    { return i_ghFinalize(); }
  StatusCode i_ghFinalize()
#endif
  ;
  // ==========================================================================
private:
  // ==========================================================================
  /// Check if all histogram maps are empty
  bool noHistos() const;
  // ===========================================unsigned===============================
  /// Constructor initialisation and job options
  void initGaudiHistosConstructor();
  // ==========================================================================
  /** @brief Declare a histogram to the monitor service
   *
   *  Uses the histogram ID as the 'name' sent to the monitor service and
   *  the histogram title as the long description
   */
  void monitorHisto( const AIDA::IBaseHistogram* hist,
                     const HistoID& ID ) const;
  // ==========================================================================
  /** Create a new histogram ID using the given title
   *  @param[in]  title Histogram title
   *  @param[out] ID The ID to use for the new histogram
   */
  void newHistoID( const std::string & title,
                   HistoID& ID ) const;
  // ==========================================================================
  /// Searches 'title' for all instancies of 'A' and replaces them with 'B'
  void stringSearchReplace( std::string & title,
                            const std::string & A,
                            const std::string & B ) const;
  // ==========================================================================
protected:
  // ==========================================================================
  /// Create an ID string from a title string
  std::string convertTitleToID( const std::string & title ) const;
  // ==========================================================================
private:
  // ==========================================================================
  /// the handler for "HistoPrint" property
  void printHistoHandler ( Property& /* theProp */ ) ;          // "HistoPrint"
  // ==========================================================================
private:
  // ==========================================================================
  /// flag to SWITCH ON/SWITCH OFF  the histogrm fillling and booking
  bool        m_produceHistos ;
  /// flag to control output level of histograms
  bool        m_fullDetail;
  /// flag to control check for Nan/Finite while filling the histogram
  bool        m_checkForNaN   ;
  /// split histogram directory name (very useful for Hbook)
  bool        m_splitHistoDir ;
  /// general histogram ID offset (only works for automatically assigned numeric IDs)
  HistoID::NumericID     m_histoOffSet ;
  /// histogram top level directory
  std::string m_histoTopDir   ;
  /// histogram directory
  std::string m_histoDir      ;
  /// print histograms at finalization
  bool        m_histosPrint   ;
  /// print histogram counters at finalization
  bool        m_histoCountersPrint   ;
  /// Flag to turn on/off the registration of histograms to the Monitoring Service
  bool        m_declareMoniHists;
  // ==========================================================================
  /// the actual storage/access of 1D histograms by unique title
  mutable Histo1DMapTitle     m_histo1DMapTitle ;
  /// the actual storage/access of 1D histograms by unique ID
  mutable Histo1DMapID        m_histo1DMapID    ;
  // ==========================================================================
  /// the actual storage/access of 2D histograms by unique title
  mutable Histo2DMapTitle     m_histo2DMapTitle ;
  /// the actual storage/access of 2D histograms by unique ID
  mutable Histo2DMapID        m_histo2DMapID    ;
  // ==========================================================================
  /// the actual storage/access of 3D histograms by unique title
  mutable Histo3DMapTitle     m_histo3DMapTitle ;
  /// the actual storage/access of 3D histograms by unique ID
  mutable Histo3DMapID        m_histo3DMapID    ;
  // ==========================================================================
  /// the actual storage/access of 1D profile histograms by unique title
  mutable Profile1DMapTitle   m_profile1DMapTitle ;
  /// the actual storage/access of 1D profile histograms by unique ID
  mutable Profile1DMapID      m_profile1DMapID    ;
  // ==========================================================================
  /// the actual storage/access of 2D profile histograms by unique title
  mutable Profile2DMapTitle   m_profile2DMapTitle ;
  /// the actual storage/access of 2D profile histograms by unique ID
  mutable Profile2DMapID      m_profile2DMapID    ;
  // ==========================================================================
  /// format for printout of 1D-histograms as a table
  std::string  m_histo1DTableFormat      ;
  /// format for printout of 1D-histograms as a table
  std::string  m_histo1DTableFormatShort ;
  /// the header for the table of 1-D historgrams
  std::string  m_histo1DTableHeader      ;
  /// Flag to switch back to the old style sequencial numerical automatic IDs
  bool        m_useNumericAutoIDs;
  /** Map of strings to search and replace when using the title as the basis
   *  of automatically generated literal IDs
   */
  std::map<std::string,std::string> m_idReplaceInfo;
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOS_H
// ============================================================================
