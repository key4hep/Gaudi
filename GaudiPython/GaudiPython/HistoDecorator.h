// $Id: HistoDecorator.h,v 1.4 2008/10/09 09:59:14 marcocle Exp $
// ============================================================================
#ifndef GAUDIPYTHON_HISTODECORATOR_H
#define GAUDIPYTHON_HISTODECORATOR_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/GaudiPython.h"
#include "GaudiPython/Vector.h"
// ============================================================================
class GaudiHistoTool ;
// ============================================================================
namespace GaudiPython
{
  /** @class HistoDecorator                        GaudiPython/HistoDecorator.h
   *  Simple decorator class to allow to reuse the functionality of
   *  GaudiHistos<TYPE> class in pythin
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date   2005-08-04
   */
  class GAUDI_API HistoDecorator
  {
  public:
    // ========================================================================
    typedef std::vector<GaudiAlg::ID>        IDs        ;
    typedef std::vector<AIDA::IHistogram1D*> Histos1D   ;
    typedef std::vector<AIDA::IHistogram2D*> Histos2D   ;
    typedef std::vector<AIDA::IHistogram3D*> Histos3D   ;
    typedef std::vector<AIDA::IProfile1D*>   Profiles1D ;
    typedef std::vector<AIDA::IProfile2D*>   Profiles2D ;
    // ========================================================================
  public:
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data  data
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const double                 data         ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const double                 data         ,
      const GaudiAlg::HistoID&     ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const double                 data         ,
      const long                   ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const double                 data         ,
      const std::string&           ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data vector of data
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const GaudiPython::Vector&   data         ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data vector of data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const GaudiPython::Vector&   data         ,
      const GaudiAlg::HistoID&     ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data vector of data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const GaudiPython::Vector&   data         ,
      const long                   ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** "plot"(book&fill) a sequence of data from the vector (implicit loop)
     *  (Expected to be more efficient)
     *  @param algo the algorihtm
     *  @param data vector of data
     *  @param ID   histogram ID
     *  @param title histogram title
     *  @param low   low edge
     *  @param high  high edge
     *  @param bins  number of bins
     */
    static AIDA::IHistogram1D*   plot1D
    ( const GaudiHistoAlg&         algo         ,
      const GaudiPython::Vector&   data         ,
      const std::string&           ID           ,
      const std::string&           title        ,
      const double                 low          ,
      const double                 high         ,
      const unsigned long          bins  = 100  ) ;
    // ========================================================================
    /** fill the 2D histogram (book on demand)
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
    static AIDA::IHistogram2D*  plot2D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const unsigned long       binsX  = 50  ,
      const unsigned long       binsY  = 50  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 2D histogram with forced ID assignment (book on demand)
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
    static AIDA::IHistogram2D*  plot2D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const GaudiAlg::HistoID&  ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const unsigned long       binsX  = 50  ,
      const unsigned long       binsY  = 50  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 2D histogram with forced ID assignment (book on demand)
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
    static AIDA::IHistogram2D*  plot2D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const long                ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const unsigned long       binsX  = 50  ,
      const unsigned long       binsY  = 50  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 2D histogram with forced ID assignment (book on demand)
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
    static AIDA::IHistogram2D*  plot2D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const std::string&        ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const unsigned long       binsX  = 50  ,
      const unsigned long       binsY  = 50  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 3D histogram (book on demand)
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
    static AIDA::IHistogram3D*  plot3D
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
      const unsigned long binsX  = 10  ,
      const unsigned long binsY  = 10  ,
      const unsigned long binsZ  = 10  ,
      const double        weight = 1.0 ) ;
    // ========================================================================
    /** fill the 3D histogram (book on demand)
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
    static AIDA::IHistogram3D*  plot3D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const double              valueZ       ,
      const GaudiAlg::HistoID&  ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const double              lowZ         ,
      const double              highZ        ,
      const unsigned long       binsX  = 10  ,
      const unsigned long       binsY  = 10  ,
      const unsigned long       binsZ  = 10  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 3D histogram (book on demand)
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
    static AIDA::IHistogram3D*  plot3D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const double              valueZ       ,
      const long                ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const double              lowZ         ,
      const double              highZ        ,
      const unsigned long       binsX  = 10  ,
      const unsigned long       binsY  = 10  ,
      const unsigned long       binsZ  = 10  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    /** fill the 3D histogram (book on demand)
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
    static AIDA::IHistogram3D*  plot3D
    ( const GaudiHistoAlg&      algo         ,
      const double              valueX       ,
      const double              valueY       ,
      const double              valueZ       ,
      const std::string&        ID           ,
      const std::string&        title        ,
      const double              lowX         ,
      const double              highX        ,
      const double              lowY         ,
      const double              highY        ,
      const double              lowZ         ,
      const double              highZ        ,
      const unsigned long       binsX  = 10  ,
      const unsigned long       binsY  = 10  ,
      const unsigned long       binsZ  = 10  ,
      const double              weight = 1.0 ) ;
    // ========================================================================
    // 1D-profiles: by title
    // ========================================================================
    static AIDA::IProfile1D* profile1D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const unsigned long      binsX  = 100 ,
      const std::string&       opt    = ""  ,
      const double             lowY   = -std::numeric_limits<double>::max() ,
      const double             highY  =  std::numeric_limits<double>::max() ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 1D-profiles: by generic ID
    // ========================================================================
    static AIDA::IProfile1D* profile1D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const GaudiAlg::HistoID& ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const unsigned long      binsX  = 100 ,
      const std::string&       opt    = ""  ,
      const double             lowY   = -std::numeric_limits<double>::max() ,
      const double             highY  =  std::numeric_limits<double>::max() ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 1D-profiles: by numeric ID
    // ========================================================================
    static AIDA::IProfile1D* profile1D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const long               ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const unsigned long      binsX  = 100 ,
      const std::string&       opt    = ""  ,
      const double             lowY   = -std::numeric_limits<double>::max() ,
      const double             highY  =  std::numeric_limits<double>::max() ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 1D-profiles: by string ID
    // ========================================================================
    static AIDA::IProfile1D* profile1D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const std::string&       ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const unsigned long      binsX  = 100 ,
      const std::string&       opt    = ""  ,
      const double             lowY   = -std::numeric_limits<double>::max() ,
      const double             highY  =  std::numeric_limits<double>::max() ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 2D-profiles: by title
    // ========================================================================
    static AIDA::IProfile2D* profile2D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const double             valueZ       ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const double             lowY         ,
      const double             highY        ,
      const unsigned long      binsX  = 50  ,
      const unsigned long      binsY  = 50  ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 2D-profiles: by generic ID
    // ========================================================================
    static AIDA::IProfile2D* profile2D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const double             valueZ       ,
      const GaudiAlg::HistoID& ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const double             lowY         ,
      const double             highY        ,
      const unsigned long      binsX  = 50  ,
      const unsigned long      binsY  = 50  ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 2D-profiles: by numeric ID
    // ========================================================================
    static AIDA::IProfile2D* profile2D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const double             valueZ       ,
      const long               ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const double             lowY         ,
      const double             highY        ,
      const unsigned long      binsX  = 50  ,
      const unsigned long      binsY  = 50  ,
      const double             weight = 1.0 ) ;
    // ========================================================================
    // 2D-profiles: by string ID
    // ========================================================================
    static AIDA::IProfile2D* profile2D
    ( const GaudiHistoAlg&     algo         ,
      const double             valueX       ,
      const double             valueY       ,
      const double             valueZ       ,
      const std::string&       ID           ,
      const std::string&       title        ,
      const double             lowX         ,
      const double             highX        ,
      const double             lowY         ,
      const double             highY        ,
      const unsigned long      binsX  = 50  ,
      const unsigned long      binsY  = 50  ,
      const double             weight = 1.0 ) ;
    // ========================================================================
  public:
    // ========================================================================
    static size_t _histos_a_
    ( const GaudiHistoAlg*       cmp    , 
      IDs&                       ids    ,
      Histos1D&                  histos ) ;
    static size_t _histos_t_
    ( const GaudiHistoTool*      cmp    , 
      IDs&                       ids    ,
      Histos1D&                  histos ) ;
    static size_t _histos_a_
    ( const IAlgorithm*          cmp    , 
      IDs&                       ids    ,
      Histos1D&                  histos ) ;
    static size_t _histos_t_
    ( const IAlgTool*            cmp    , 
      IDs&                       ids    ,
      Histos1D&                  histos ) ;
    // ========================================================================
    static size_t _histos_a_ 
    ( const GaudiHistoAlg*       cmp    , 
      IDs&                       ids    ,
      Histos2D&                  histos ) ;
    static size_t _histos_t_
    ( const GaudiHistoTool*      cmp    , 
      IDs&                       ids    ,
      Histos2D&                  histos ) ;
    static size_t _histos_a_
    ( const IAlgorithm*          cmp    , 
      IDs&                       ids    ,
      Histos2D&                  histos ) ;
    static size_t _histos_t_
    ( const IAlgTool*            cmp    , 
      IDs&                       ids    ,
      Histos2D&                  histos ) ;
    // ========================================================================
    static size_t _histos_a_
    ( const GaudiHistoAlg*       cmp    , 
      IDs&                       ids    ,
      Histos3D&                  histos ) ;
    static size_t _histos_t_
    ( const GaudiHistoTool*      cmp    , 
      IDs&                       ids    ,
      Histos3D&                  histos ) ;
    static size_t _histos_a_
    ( const IAlgorithm*          cmp    , 
      IDs&                       ids    ,
      Histos3D&                  histos ) ;
    static size_t _histos_t_
    ( const IAlgTool*            cmp    , 
      IDs&                       ids    ,
      Histos3D&                  histos ) ;
    // ========================================================================
    static size_t _histos_a_
    ( const GaudiHistoAlg*       cmp    , 
      IDs&                       ids    ,
      Profiles1D&                histos ) ;
    static size_t _histos_t_
    ( const GaudiHistoTool*      cmp    , 
      IDs&                       ids    ,
      Profiles1D&                histos ) ;
    static size_t _histos_a_
    ( const IAlgorithm*          cmp    , 
      IDs&                       ids    ,
      Profiles1D&                histos ) ;
    static size_t _histos_t_
    ( const IAlgTool*            cmp    , 
      IDs&                       ids    ,
      Profiles1D&                histos ) ;
    // ========================================================================
    static size_t _histos_a_
    ( const GaudiHistoAlg*       cmp    , 
      IDs&                       ids    ,
      Profiles2D&                histos ) ;
    static size_t _histos_t_
    ( const GaudiHistoTool*      cmp    , 
      IDs&                       ids    ,
      Profiles2D&                histos ) ;
    static size_t _histos_a_
    ( const IAlgorithm*          cmp    , 
      IDs&                       ids    ,
      Profiles2D&                histos ) ;
    static size_t _histos_t_
    ( const IAlgTool*            cmp    , 
      IDs&                       ids    ,
      Profiles2D&                histos ) ;
    // ========================================================================
  };
  // ==========================================================================
} // end of namespace GaudiPython
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIPYTHON_HISTODECORATOR_H
// ============================================================================
