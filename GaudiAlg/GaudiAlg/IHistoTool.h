// $Id: IHistoTool.h,v 1.3 2005/01/18 15:51:53 mato Exp $
// ============================================================================
#ifndef GAUDIALG_IHISTOTOOL_H
#define GAUDIALG_IHISTOTOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// from STL
// ============================================================================
#include <string>
// ============================================================================
// from Gaudi
// ============================================================================
#include "GaudiKernel/IAlgTool.h"
#include "GaudiAlg/HistoID.h"
// ============================================================================

namespace AIDA
{
  class IHistogram1D;
  class IHistogram2D;
  class IHistogram3D;
}

/** @class IHistoTool IHistoTool.h GaudiTools/IHistoTool.h
 *
 *  An abstract interface for "histogramming tool"
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */

class GAUDI_API IHistoTool: virtual public IAlgTool
{
public:
  /// InterfaceID
  DeclareInterfaceID(IHistoTool,2,0);

  /// the actual type for histogram identifier (HBOOK style)
  typedef Histos::HistoID       HistoID       ;

public:

  // ================================= 1D Histograms ========================================

  /** fill the 1D histogram (book on demand)
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the histogram
   *  titled <tt>"InvariantMass"</tt> with value @c mass .
   *  - If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c low  equal to  2.5, parameters @c high equal to 3.5
   *  and @c bins equal to  100.
   *
   *  The histogram will get a unique integer identifier automatically assigned.
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
  virtual AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const  = 0 ;

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
  AIDA::IHistogram1D*  plot
  ( const double        value        ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const
  {
    return plot1D ( value, title, low, high, bins, weight );
  }

  /** fill the 1D histogram with forced ID assignement (book on demand)
   *
   *  @code
   *
   *     const double mass = ... ;
   *     plot1D( mass , 15 , "Invariant Mass" , 2.5 , 3.5 , 100 )
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the 1D histogram ID=15
   *  titled <tt>"Invariant Mass"</tt> with value @c mass .
   *  - If the histogram with given ID does not exist yet
   *  it will be automatically booked with parameters
   *  @c low  equal to  2.5, parameters @c high equal to 3.5
   *  and @c bins equal to  100.
   *
   *  @attention
   *   If the histogram with given ID is already booked
   *   through automatic assignement of histogram ID,
   *   the error will not be detected.
   *   Therefore it is recommended
   *   to use non-trivial histogram ID offset  (property "HistoOffSet")
   *   if one need to combine these techniques together
   *   It is still desirable to use the unique histogram title
   *   to avoid a bad interference.
   *
   *  Note : This method is more efficient that the similar method without
   *         forced ID, since the histogram lookup is faster using a numerical ID.
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
   *  @return pointer to AIDA histogram
   */
  virtual AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const = 0 ;

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
   *  @return pointer to AIDA histogram
   */
  AIDA::IHistogram1D*  plot
  ( const double        value        ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const
  {
    return plot1D ( value, ID, title, low, high, bins, weight );
  };

  // ================================= 2D Histograms ========================================

  /** fill the 2D histogram (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the 2D histogram
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt> with values @c mass1 and @c mass2 .
   *  - If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 100 and @c binsY equal to 200.
   *
   *  The histogram will get a unique integer identifier automatically assigned
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
  virtual AIDA::IHistogram2D*  plot2D
  ( const double        valueX       ,
    const double        valueY       ,
    const std::string&  title        ,
    const double        lowX         ,
    const double        highX        ,
    const double        lowY         ,
    const double        highY        ,
    const unsigned long binsX  = 50  ,
    const unsigned long binsY  = 50  ,
    const double        weight = 1.0 ) const = 0;

  /** fill the 2D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     plot2D( mass1, mass2, 15, "Invariant Mass2 versus Mass1" ,2.5 ,3.5, 4.5, 5.5, 100, 200 );
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the 2D histogram ID=15
   *  titled <tt>"Invariant Mass2 versus Mass1"</tt> with values @c mass1 and @c mass2 .
   *  - If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c binsX equal to 100 and @c binsY equal to 200.
   *
   *  @attention
   *   If the histogram with given ID is already booked
   *   through automatic assignment of histogram ID,
   *   the error will not be detected.
   *   Therefore it is recommended
   *   to use non-trivial histogram ID offset  (property "HistoOffSet")
   *   if one need to combine these techniques together
   *   It is still desirable to use the unique histogram title
   *   to avoid a bad interference
   *
   *  Note : This method is more efficient that the similar method without
   *         forced ID, since the histogram lookup is faster using a numerical ID.
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
  virtual AIDA::IHistogram2D*  plot2D
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
    const double        weight = 1.0 ) const = 0;

  // ================================= 3D Histograms ========================================

  /** fill the 3D histogram (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     const double mass3 = ... ;
   *     plot3D( X, Y, Z, "Space Points" ,2.5 ,3.5, 4.5, 5.5, 6.5, 7.5, 10, 20, 30 );
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the 3D histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *  - If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c lowZ equal to 6.5, @c highZ equal to 7.5,
   *  @c binsX equal to 10, @c binsY equal to 20 and @c binsZ equal to 30.
   *
   *  The histogram will get a unique integer identifier automatically assigned
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
  virtual AIDA::IHistogram3D*  plot3D
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
    const double        weight = 1.0 ) const = 0;

  /** fill the 3D histogram with forced ID assignment (book on demand)
   *
   *  @code
   *
   *     const double mass1 = ... ;
   *     const double mass2 = ... ;
   *     const double mass3 = ... ;
   *     plot3D( X, Y, Z, "Space Points" ,2.5 ,3.5, 4.5, 5.5, 6.5, 7.5, 10, 20, 30 );
   *
   *  @endcode
   *
   *  - This example illustrates the filling of the 3D histogram
   *  titled <tt>"Space Points"</tt> with values @c X, @c Y and @c Z.
   *  - If the histogram with given title does not exist yet
   *  it will be automatically booked with parameters
   *  @c lowX equal to 2.5, @c highX equal to 3.5,
   *  @c lowY equal to 4.5, @c highY equal to 5.5,
   *  @c lowZ equal to 6.5, @c highZ equal to 7.5,
   *  @c binsX equal to 10, @c binsY equal to 20 and @c binsZ equal to 30.
   *
   *  @attention
   *   If the histogram with given ID is already booked
   *   through automatic assignment of histogram ID,
   *   the error will not be detected.
   *   Therefore it is recommended
   *   to use non-trivial histogram ID offset  (property "HistoOffSet")
   *   if one need to combine these techniques together
   *   It is still desirable to use the unique histogram title
   *   to avoid a bad interference
   *
   *  Note : This method is more efficient that the similar method without
   *         forced ID, since the histogram lookup is faster using a numerical ID.
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
  virtual AIDA::IHistogram3D*  plot3D
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
    const double        weight = 1.0 ) const = 0;

public:

  /** book the 1D histogram
   *
   *  The histogram will be assigned a unique identifier
   *
   *  @see IHistogram1D
   *  @param title histogram title (must be unique within the algorithm)
   *  @param low   low limit for histogram
   *  @param high  high limit for histogram
   *  @param bins  number of bins
   *  @return pointer to AIDA 1D histogram
   */
  virtual AIDA::IHistogram1D*  book1D
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const = 0 ;

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
  AIDA::IHistogram1D*  book
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return book1D( title, low, high, bins );
  }

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
  virtual AIDA::IHistogram2D*  book2D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const = 0;

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
  virtual AIDA::IHistogram3D*  book3D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  10 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  10 ,
    const double        lowZ    =   0 ,
    const double        highZ   = 100 ,
    const unsigned long binsZ   =  10 ) const = 0;

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
  virtual AIDA::IHistogram1D*  book1D
  ( const HistoID&      ID           ,
    const std::string&  title  = ""  ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const = 0 ;

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
  AIDA::IHistogram1D*  book
  ( const HistoID&      ID           ,
    const std::string&  title  = ""  ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return book1D( ID, title, low, high, bins );
  }

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
  virtual AIDA::IHistogram2D*  book2D
  ( const HistoID&      ID            ,
    const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   = 100 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   = 100 ) const = 0;

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
  virtual AIDA::IHistogram3D*  book3D
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
    const unsigned long binsZ   =  10 ) const = 0;

  /** fill the 1D histo with the value and weight
   *  @param histo 1D histogram to be filled
   *  @param value value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 1D histogram
   */
  virtual AIDA::IHistogram1D* fill
  ( AIDA::IHistogram1D* histo  ,
    const double        value  ,
    const double        weight ,
    const std::string&  title  = "") const = 0 ;

  /** fill the 2D histo with the value and weight
   *  @param histo 2D histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 2D histogram
   */
  virtual AIDA::IHistogram2D* fill
  ( AIDA::IHistogram2D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        weight ,
    const std::string&  title  = "" ) const = 0 ;

  /** fill the 3D histo with the value and weight
   *  @param histo 3D histogram to be filled
   *  @param valueX x value to be put into the histogram
   *  @param valueY y value to be put into the histogram
   *  @param valueZ z value to be put into the histogram
   *  @param weight weight to be used
   *  @param title  histogram title (to be used for error report)
   *  @return pointer to AIDA 3D histogram
   */
  virtual AIDA::IHistogram3D* fill
  ( AIDA::IHistogram3D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        valueZ ,
    const double        weight ,
    const std::string&  title  = "" ) const = 0 ;

  /** access the EXISTING 1D histogram by title
   *  return the pointer to existing 1D histogram or NULL
   */
  virtual AIDA::IHistogram1D*  histo1D ( const std::string& title  )  const = 0 ;

  /** access the EXISTING 1D histogram by title
   *
   *  Wrapper method for the equivalent histo1D method.
   *  Retained for backwards compatibility, please use histo1D instead.
   *
   *  return the pointer to existing 1D histogram or NULL
   */
  AIDA::IHistogram1D* histo ( const std::string& title  )  const
  {
    return histo1D( title );
  };

  /** access the EXISTING 2D histogram by title
   *  return the pointer to existing 2D histogram or NULL
   */
  virtual AIDA::IHistogram2D* histo2D ( const std::string& title  )  const = 0;


  /** access the EXISTING 3D histogram by title
   *  return the pointer to existing 3D histogram or NULL
   */
  virtual AIDA::IHistogram3D* histo3D ( const std::string& title  )  const = 0;

  /** access the EXISTING 1D histogram by ID
   *  return the pointer to existing 1D histogram or NULL
   */
  virtual AIDA::IHistogram1D*  histo1D ( const HistoID&     ID     )  const = 0 ;

  /** access the EXISTING 1D histogram by ID
   *
   *  Wrapper method for the equivalent histo1D method.
   *  Retained for backwards compatibility, please use histo1D instead.
   *
   *  return the pointer to existing 1D histogram or NULL
   */
  AIDA::IHistogram1D* histo ( const HistoID&     ID     )  const
  {
    return histo1D( ID );
  };

  /** access the EXISTING 2D histogram by ID
   *  return the pointer to existing 2D histogram or NULL
   */
  virtual AIDA::IHistogram2D* histo2D ( const HistoID&     ID     )  const = 0;

  /** access the EXISTING 3D histogram by ID
   *  return the pointer to existing 3D histogram or NULL
   */
  virtual AIDA::IHistogram3D* histo3D ( const HistoID&     ID     )  const = 0;

  /// check the existence AND validity of the histogram with given title
  virtual bool histoExists ( const std::string& title  )  const = 0 ;

  /// check the existence AND validity of the histogram with given title
  virtual bool histoExists ( const HistoID&     ID     )  const = 0 ;

public:  // non-virtual methods

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
   *  The histogram will get a unique integer identifier automatically assigned
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
   *  Note : These plot methods using iterator ranges are more efficient than
   *         the simplier "value" only methods, since the associated histogram
   *         only requires locating from internal storage once per loop, as opposed
   *         to once per fill for the simplier functions. It is recommended to use
   *         these whenever possible.
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
  AIDA::IHistogram1D*  plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins  = 100  ) const
  {
    // retrieve or book the histogram
    AIDA::IHistogram1D* h = histo1D ( title ) ;
    if ( 0 == h )     { h = book1D  ( title , low , high , bins ); }
    while ( first != last && 0 != h  )
    { h = fill ( h , func( *first ) , 1.0 , title  ) ; ++first ; }
    return h ;
  }

  /** fill the 1D histogram with forced ID and information from
   *  [first,last) sequence
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        100 , " bla-bla "   ,   // ID and title
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
   *  Note : These plot methods using iterator ranges are more efficient than
   *         the simplier "value" only methods, since the associated histogram
   *         only requires locating from internal storage once per loop, as opposed
   *         to once per fill for the simplier functions. It is recommended to use
   *         these whenever possible.
   *
   *  Note : This method is more efficient that the similar method without
   *         forced ID, since the histogram lookup is faster using a numerical ID.
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
  AIDA::IHistogram1D*  plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins  = 100  ) const
  {
    // retrieve or book the histogram
    AIDA::IHistogram1D* h = histo1D ( ID ) ;
    if ( 0 == h )     { h = book1D  ( ID , title , low , high , bins ); }
    while ( first != last && 0 != h )
    { h = fill( h , func( *first ) , 1.0 , title  ) ; ++first ; }
    return h ;
  }

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
   *  The histogram will get a unique integer identifier automatically assigned
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
   *  Note : These plot methods using iterator ranges are more efficient than
   *         the simplier "value" only methods, since the associated histogram
   *         only requires locating from internal storage once per loop, as opposed
   *         to once per fill for the simplier functions. It is recommended to use
   *         these whenever possible.
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
  AIDA::IHistogram1D*  plot
  ( const FUNCTION&     func         ,
    OBJECT              first        ,
    OBJECT              last         ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins         ,
    const WEIGHT&       weight       ) const
  {
    // retrieve or book the histogram
    AIDA::IHistogram1D* h = histo1D ( title ) ;
    if ( 0 == h     ) { h = book1D  ( title , low , high , bins ); }
    while( first != last && 0 != h )
    { h = fill ( h                 ,
                 func   ( *first ) ,
                 weight ( *first ) , title  ) ; ++first ; }
    return h ;
  }

  /** book and fill the 1D histogram with forced ID and information from
   *  [first,last) sequence with given weight
   *
   *  @code
   *
   *  std::vector<double> v = ... ;
   *
   *  plot( sin                 ,   // function
   *        v.begin() , v.end() ,   // sequence
   *        100 , " bla-bla "   ,   // ID and title
   *        -1. , 1.0           ,   // low and high limits
   *        100                 ,   // number of bins
   *        sinh                );  // weight function
   *
   *  @endcode
   *
   *  @attention no checks for NaN and Finite is performed!
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
   *  Note : These plot methods using iterator ranges are more efficient than
   *         the simplier "value" only methods, since the associated histogram
   *         only requires locating from internal storage once per loop, as opposed
   *         to once per fill for the simplier functions. It is recommended to use
   *         these whenever possible.
   *
   *  Note : This method is more efficient that the similar method without
   *         forced ID, since the histogram lookup is faster using a numerical ID.
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
  AIDA::IHistogram1D*  plot
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
    // retrieve or book the histogram
    AIDA::IHistogram1D* h = histo1D ( ID ) ;
    if ( 0 == h     ) { h = book1D  ( ID , title , low , high , bins ); }
    while( first != last && 0 != h )
    { h  = fill ( h                 ,
                  func   ( *first ) ,
                  weight ( *first ) , title  ) ; ++first ; }
    return h ;
  }

protected:

  virtual ~IHistoTool() ;

};

#endif // GAUDIALG_IHISTOTOOL_H
