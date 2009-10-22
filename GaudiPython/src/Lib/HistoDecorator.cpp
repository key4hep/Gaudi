// $Id: HistoDecorator.cpp,v 1.5 2008/10/09 09:59:14 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiHistoTool.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/Vector.h"
#include "GaudiPython/HistoDecorator.h"
// ============================================================================
/** @file
 * Implementation file for class GaudiPython::HistoDecorator
 *  @date 2005-08-04
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 */
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data  data
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo  ,
  const double                 data  ,
  const std::string&           title ,
  const double                 low   ,
  const double                 high  ,
  const unsigned long          bins  )
{ return algo.plot1D( data , title , low , high , bins ) ; }
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo  ,
  const double                 data  ,
  const GaudiAlg::HistoID&     ID    ,
  const std::string&           title ,
  const double                 low   ,
  const double                 high  ,
  const unsigned long          bins  )
{ return algo.plot1D( data , ID , title , low , high , bins ) ; }
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo  ,
  const double                 data  ,
  const long                   ID    ,
  const std::string&           title ,
  const double                 low   ,
  const double                 high  ,
  const unsigned long          bins  )
{ return algo.plot1D( data , ID , title , low , high , bins ) ; }
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo  ,
  const double                 data  ,
  const std::string&           ID    ,
  const std::string&           title ,
  const double                 low   ,
  const double                 high  ,
  const unsigned long          bins  )
{ return algo.plot1D( data , ID , title , low , high , bins ) ; }
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data vector of data
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo  ,
  const GaudiPython::Vector&   data  ,
  const std::string&           title ,
  const double                 low   ,
  const double                 high  ,
  const unsigned long          bins  )
{
  return algo.plot
    ( GaudiPython::_identity<GaudiPython::Vector::value_type>() ,
      data.begin ()              ,
      data.end   ()              ,
      title , low , high , bins  ) ;
}
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data vector of data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo   ,
  const GaudiPython::Vector&   data   ,
  const GaudiAlg::HistoID&     ID     ,
  const std::string&           title  ,
  const double                 low    ,
  const double                 high   ,
  const unsigned long          bins   )
{
  return algo.plot
    ( GaudiPython::_identity<GaudiPython::Vector::value_type>() ,
      data.begin ()                       ,
      data.end   ()                       ,
      ID , title , low , high , bins      ) ;
}
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data vector of data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo   ,
  const GaudiPython::Vector&   data   ,
  const long                   ID     ,
  const std::string&           title  ,
  const double                 low    ,
  const double                 high   ,
  const unsigned long          bins   )
{
  return algo.plot
    ( GaudiPython::_identity<GaudiPython::Vector::value_type>() ,
      data.begin ()                       ,
      data.end   ()                       ,
      ID , title , low , high , bins      ) ;
}
// ============================================================================
/*  "plot"(book&fill) a sequence of data from the vector (implicit loop)
 *  (Expected to be more efficient)
 *  @param algo the algorithm
 *  @param data vector of data
 *  @param ID   histogram ID
 *  @param title histogram title
 *  @param low   low edge
 *  @param high  high edge
 *  @param bins  number of bins
 */
// ============================================================================
AIDA::IHistogram1D*   GaudiPython::HistoDecorator::plot1D
( const GaudiHistoAlg&         algo   ,
  const GaudiPython::Vector&   data   ,
  const std::string&           ID     ,
  const std::string&           title  ,
  const double                 low    ,
  const double                 high   ,
  const unsigned long          bins   )
{
  return algo.plot
    ( GaudiPython::_identity<GaudiPython::Vector::value_type>() ,
      data.begin ()                       ,
      data.end   ()                       ,
      ID , title , low , high , bins      ) ;
}
// ============================================================================
/*  fill the 2D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram2D*  GaudiPython::HistoDecorator::plot2D
( const GaudiHistoAlg&      algo    ,
  const double              valueX  ,
  const double              valueY  ,
  const std::string&        title   ,
  const double              lowX    ,
  const double              highX   ,
  const double              lowY    ,
  const double              highY   ,
  const unsigned long       binsX   ,
  const unsigned long       binsY   ,
  const double              weight  )
{
  return algo.plot2D ( valueX , valueY ,
                       title  ,
                       lowX   , highX  ,
                       lowY   , highY  ,
                       binsX  , binsY  , weight ) ;
}
// ============================================================================
/** fill the 2D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram2D*  GaudiPython::HistoDecorator::plot2D
( const GaudiHistoAlg&      algo    ,
  const double              valueX  ,
  const double              valueY  ,
  const GaudiAlg::HistoID&  ID      ,
  const std::string&        title   ,
  const double              lowX    ,
  const double              highX   ,
  const double              lowY    ,
  const double              highY   ,
  const unsigned long       binsX   ,
  const unsigned long       binsY   ,
  const double              weight  )
{
  return algo.plot2D ( valueX , valueY ,
                       ID     , title  ,
                       lowX   , highX  ,
                       lowY   , highY  ,
                       binsX  , binsY  , weight ) ;
}
// ============================================================================
/*  fill the 2D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram2D*  GaudiPython::HistoDecorator::plot2D
( const GaudiHistoAlg&      algo    ,
  const double              valueX  ,
  const double              valueY  ,
  const long                ID      ,
  const std::string&        title   ,
  const double              lowX    ,
  const double              highX   ,
  const double              lowY    ,
  const double              highY   ,
  const unsigned long       binsX   ,
  const unsigned long       binsY   ,
  const double              weight  )
{
  return algo.plot2D ( valueX , valueY ,
                       ID     , title  ,
                       lowX   , highX  ,
                       lowY   , highY  ,
                       binsX  , binsY  , weight ) ;
}
// ============================================================================
/*  fill the 2D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram2D*  GaudiPython::HistoDecorator::plot2D
( const GaudiHistoAlg&      algo    ,
  const double              valueX  ,
  const double              valueY  ,
  const std::string&        ID      ,
  const std::string&        title   ,
  const double              lowX    ,
  const double              highX   ,
  const double              lowY    ,
  const double              highY   ,
  const unsigned long       binsX   ,
  const unsigned long       binsY   ,
  const double              weight  )
{
  return algo.plot2D ( valueX , valueY ,
                       ID     , title  ,
                       lowX   , highX  ,
                       lowY   , highY  ,
                       binsX  , binsY  , weight ) ;
}
// ============================================================================
/*  fill the 3D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram3D*  GaudiPython::HistoDecorator::plot3D
( const GaudiHistoAlg&      algo   ,
  const double        valueX       ,
  const double        valueY       ,
  const double        valueZ       ,
  const std::string&  title        ,
  const double        lowX         ,
  const double        highX        ,
  const double        lowY         ,
  const double        highY        ,
  const double        lowZ         ,
  const double        highZ        ,
  const unsigned long binsX        ,
  const unsigned long binsY        ,
  const unsigned long binsZ        ,
  const double        weight       )
{
  return algo. plot3D ( valueX , valueY , valueZ ,
                        title                    ,
                        lowX   , highX  ,
                        lowY   , highY  ,
                        lowZ   , highZ  ,
                        binsX  , binsY  , binsZ  ,
                        weight                   ) ;
}
// ============================================================================
/*  fill the 3D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram3D*  GaudiPython::HistoDecorator::plot3D
( const GaudiHistoAlg&      algo   ,
  const double        valueX       ,
  const double        valueY       ,
  const double        valueZ       ,
  const GaudiAlg::HistoID&  ID     ,
  const std::string&  title        ,
  const double        lowX         ,
  const double        highX        ,
  const double        lowY         ,
  const double        highY        ,
  const double        lowZ         ,
  const double        highZ        ,
  const unsigned long binsX        ,
  const unsigned long binsY        ,
  const unsigned long binsZ        ,
  const double        weight       )
{
  return algo. plot3D ( valueX , valueY , valueZ ,
                        ID     , title  ,
                        lowX   , highX  ,
                        lowY   , highY  ,
                        lowZ   , highZ  ,
                        binsX  , binsY  , binsZ  ,
                        weight                   ) ;
}
// ============================================================================
/*  fill the 3D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram3D*  GaudiPython::HistoDecorator::plot3D
( const GaudiHistoAlg&      algo   ,
  const double        valueX       ,
  const double        valueY       ,
  const double        valueZ       ,
  const long          ID           ,
  const std::string&  title        ,
  const double        lowX         ,
  const double        highX        ,
  const double        lowY         ,
  const double        highY        ,
  const double        lowZ         ,
  const double        highZ        ,
  const unsigned long binsX        ,
  const unsigned long binsY        ,
  const unsigned long binsZ        ,
  const double        weight       )
{
  return algo. plot3D ( valueX , valueY , valueZ ,
                        ID     , title  ,
                        lowX   , highX  ,
                        lowY   , highY  ,
                        lowZ   , highZ  ,
                        binsX  , binsY  , binsZ  ,
                        weight                   ) ;
}
// ============================================================================
/*  fill the 3D histogram (book on demand)
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
// ============================================================================
AIDA::IHistogram3D*
GaudiPython::HistoDecorator::plot3D
( const GaudiHistoAlg&      algo   ,
  const double        valueX       ,
  const double        valueY       ,
  const double        valueZ       ,
  const std::string&  ID           ,
  const std::string&  title        ,
  const double        lowX         ,
  const double        highX        ,
  const double        lowY         ,
  const double        highY        ,
  const double        lowZ         ,
  const double        highZ        ,
  const unsigned long binsX        ,
  const unsigned long binsY        ,
  const unsigned long binsZ        ,
  const double        weight       )
{
  return algo. plot3D ( valueX , valueY , valueZ ,
                        ID     , title  ,
                        lowX   , highX  ,
                        lowY   , highY  ,
                        lowZ   , highZ  ,
                        binsX  , binsY  , binsZ  ,
                        weight                   ) ;
}
// ========================================================================
// 1D-profiles: by title
// ========================================================================
AIDA::IProfile1D*
GaudiPython::HistoDecorator::profile1D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const unsigned long    binsX  ,
  const std::string&     opt    ,
  const double           lowY   ,
  const double           highY  ,
  const double           weight )
{
  return algo.profile1D
    ( valueX , valueY , title , lowX , highX , binsX ,
      opt , lowY , highY , weight ) ;
}
// ========================================================================
// 1D-profiles: by generic ID
// ========================================================================
AIDA::IProfile1D*
GaudiPython::HistoDecorator::profile1D
( const GaudiHistoAlg&     algo   ,
  const double             valueX ,
  const double             valueY ,
  const GaudiAlg::HistoID& ID     ,
  const std::string&       title  ,
  const double             lowX   ,
  const double             highX  ,
  const unsigned long      binsX  ,
  const std::string&       opt    ,
  const double             lowY   ,
  const double             highY  ,
  const double             weight )
{
  return algo.profile1D
    ( valueX , valueY , ID , title , lowX , highX , binsX ,
      opt , lowY , highY , weight ) ;
}
// ========================================================================
// 1D-profiles: by numeric ID
// ========================================================================
AIDA::IProfile1D*
GaudiPython::HistoDecorator::profile1D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const long             ID     ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const unsigned long    binsX  ,
  const std::string&     opt    ,
  const double           lowY   ,
  const double           highY  ,
  const double           weight )
{
  return algo.profile1D
    ( valueX , valueY , ID , title , lowX , highX , binsX ,
      opt , lowY , highY , weight ) ;
}
// ========================================================================
// 1D-profiles: by string ID
// ========================================================================
AIDA::IProfile1D*
GaudiPython::HistoDecorator::profile1D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const std::string&     ID     ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const unsigned long    binsX  ,
  const std::string&     opt    ,
  const double           lowY   ,
  const double           highY  ,
  const double           weight )
{
  return algo.profile1D
    ( valueX , valueY , ID , title , lowX , highX , binsX ,
      opt , lowY , highY , weight ) ;
}
// ========================================================================
// 2D-profiles: by title
// ========================================================================
AIDA::IProfile2D*
GaudiPython::HistoDecorator::profile2D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const double           valueZ ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const double           lowY   ,
  const double           highY  ,
  const unsigned long    binsX  ,
  const unsigned long    binsY  ,
  const double           weight )
{
  return algo.profile2D
    ( valueX , valueY , valueZ ,
      title  ,
      lowX   , highX  ,
      lowY   , highY  ,
      binsX  , binsY  , weight  ) ;
}
// ========================================================================
// 2D-profiles: by generic ID
// ========================================================================
AIDA::IProfile2D*
GaudiPython::HistoDecorator::profile2D
( const GaudiHistoAlg&     algo   ,
  const double             valueX ,
  const double             valueY ,
  const double             valueZ ,
  const GaudiAlg::HistoID& ID     ,
  const std::string&       title  ,
  const double             lowX   ,
  const double             highX  ,
  const double             lowY   ,
  const double             highY  ,
  const unsigned long      binsX  ,
  const unsigned long      binsY  ,
  const double             weight )
{
  return algo.profile2D
    ( valueX , valueY , valueZ ,
      ID     , title  ,
      lowX   , highX  ,
      lowY   , highY  ,
      binsX  , binsY  , weight  ) ;
}
// ========================================================================
// 2D-profiles: by numeric ID
// ========================================================================
AIDA::IProfile2D*
GaudiPython::HistoDecorator::profile2D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const double           valueZ ,
  const long             ID     ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const double           lowY   ,
  const double           highY  ,
  const unsigned long    binsX  ,
  const unsigned long    binsY  ,
  const double           weight )
{
  return algo.profile2D
    ( valueX , valueY , valueZ ,
      ID     , title  ,
      lowX   , highX  ,
      lowY   , highY  ,
      binsX  , binsY  , weight  ) ;
}
// ========================================================================
// 2D-profiles: by string ID
// ========================================================================
AIDA::IProfile2D*
GaudiPython::HistoDecorator::profile2D
( const GaudiHistoAlg&   algo   ,
  const double           valueX ,
  const double           valueY ,
  const double           valueZ ,
  const std::string&     ID     ,
  const std::string&     title  ,
  const double           lowX   ,
  const double           highX  ,
  const double           lowY   ,
  const double           highY  ,
  const unsigned long    binsX  ,
  const unsigned long    binsY  ,
  const double           weight )
{
  return algo.profile2D
    ( valueX , valueY , valueZ ,
      ID     , title  ,
      lowX   , highX  ,
      lowY   , highY  ,
      binsX  , binsY  , weight  ) ;
}
// ============================================================================
// get all histograms
// ============================================================================
namespace
{
  /// collect the histograms
  template <class KEY, class HISTO>
  size_t _getHistos
  ( const GaudiUtils::HashMap<KEY,HISTO>& a ,
    std::map<GaudiAlg::ID,HISTO>&         b )
  {
    for  ( typename GaudiUtils::HashMap<KEY,HISTO>::const_iterator ih =
             a.begin() ; a.end() != ih ; ++ih )
    {
      if ( 0 == ih->second ) { continue ; }
      GaudiAlg::ID id ( ih->first ) ;
      b [ id ] = ih->second ;
    }
    return b.size() ;
  }
  // =========================================================================
  template <class HISTO>
  size_t fromMap
  ( const std::map<GaudiAlg::ID,HISTO>&  a ,
    std::vector<GaudiAlg::ID>&           b ,
    std::vector<HISTO>&                  c )
  {
    b.clear() ;
    c.clear() ;
     for ( typename std::map<GaudiAlg::ID,HISTO>::const_iterator it =
             a.begin() ; a.end() != it ; ++it )
     {
       if ( 0 == it->second ) { continue ; }
       b.push_back ( it->first  ) ;
       c.push_back ( it->second ) ;
     }
    return b.size () ;
  }
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const GaudiHistoAlg*                   cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos1D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram1D*> _map ;
  //
  _getHistos ( cmp -> histo1DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const GaudiHistoTool*                  cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos1D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram1D*> _map ;
  //
  _getHistos ( cmp -> histo1DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const IAlgorithm*                        cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos1D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_a_ ( dynamic_cast<const GaudiHistoAlg*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const IAlgTool*                          cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos1D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_t_ ( dynamic_cast<const GaudiHistoTool*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
//                                                                2D-histograms
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const GaudiHistoAlg*                   cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos2D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram2D*> _map ;
  //
  _getHistos ( cmp -> histo2DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const GaudiHistoTool*                  cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos2D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram2D*> _map ;
  //
  _getHistos ( cmp -> histo2DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const IAlgorithm*                        cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos2D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_a_ ( dynamic_cast<const GaudiHistoAlg*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const IAlgTool*                          cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos2D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_t_ ( dynamic_cast<const GaudiHistoTool*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
//                                                                3D-histograms
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const GaudiHistoAlg*                   cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos3D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram3D*> _map ;
  //
  _getHistos ( cmp -> histo3DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const GaudiHistoTool*                  cmp    ,
  GaudiPython::HistoDecorator::IDs&      ids    ,
  GaudiPython::HistoDecorator::Histos3D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IHistogram3D*> _map ;
  //
  _getHistos ( cmp -> histo3DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const IAlgorithm*                        cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos3D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_a_ ( dynamic_cast<const GaudiHistoAlg*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const IAlgTool*                          cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Histos3D&   histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_t_ ( dynamic_cast<const GaudiHistoTool*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
//                                                                  1D-profiles
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const GaudiHistoAlg*                     cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles1D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IProfile1D*> _map ;
  //
  _getHistos ( cmp -> profile1DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const GaudiHistoTool*                    cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles1D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IProfile1D*> _map ;
  //
  _getHistos ( cmp -> profile1DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const IAlgorithm*                        cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles1D& histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_a_ ( dynamic_cast<const GaudiHistoAlg*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const IAlgTool*                          cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles1D& histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_t_ ( dynamic_cast<const GaudiHistoTool*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
//                                                                  2D-profiles
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const GaudiHistoAlg*                     cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles2D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IProfile2D*> _map ;
  //
  _getHistos ( cmp -> profile2DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const GaudiHistoTool*                    cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles2D& histos )
{
  histos.clear() ;
  ids   .clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  //
  std::map<GaudiAlg::ID,AIDA::IProfile2D*> _map ;
  //
  _getHistos ( cmp -> profile2DMapID () , _map ) ;
  //
  return fromMap ( _map , ids , histos ) ;
  // ==========================================================================
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_a_
( const IAlgorithm*                        cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles2D& histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_a_ ( dynamic_cast<const GaudiHistoAlg*> ( cmp ) , ids, histos ) ;
}
// ============================================================================
size_t GaudiPython::HistoDecorator::_histos_t_
( const IAlgTool*                          cmp    ,
  GaudiPython::HistoDecorator::IDs&        ids    ,
  GaudiPython::HistoDecorator::Profiles2D& histos )
{
  ids    . clear() ;
  histos . clear() ;
  if ( 0 == cmp ) { return 0 ; }                                      // RETURN
  return _histos_t_ ( dynamic_cast<const GaudiHistoTool*> ( cmp ) , ids, histos ) ;
}
// ============================================================================


// ============================================================================
// The END
// ============================================================================
