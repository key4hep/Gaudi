// $Id: HistoTool.h,v 1.4 2006/11/30 10:16:12 mato Exp $
// ============================================================================
#ifndef GAUDIALG_HISTOTOOL_H
#define GAUDIALG_HISTOTOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/IHistoTool.h"
// ============================================================================

/** @file HistoTool.h
 *
 *  Header file for class : HistoTool
 *
 *  @date 2004-06-28
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 */

/** @class HistoTool HistoTool.h components/HistoTool.h
 *
 *  Simple tool for histogramming (component)
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */

class HistoTool : virtual public GaudiHistoTool ,
                  virtual public     IHistoTool
{
public:
  typedef IHistoTool::HistoID HistoID ;

  /** Standard constructor
   *  @see GaudiHistoTool
   *  @see      GaudiTool
   *  @see        AlgTool
   *  @param type   tool type
   *  @param name   tool name
   *  @param parent pointer to the parent object
   */
  HistoTool ( const std::string& type,
              const std::string& name,
              const IInterface* parent);

  // protected virtual destructor
  virtual ~HistoTool( );

public:

  // ================================= 1D Histograms ========================================

  // fill the histogram (book on demand)
  virtual AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const

  {
    return GaudiHistoTool::plot1D
      ( value , title , low , high , bins , weight ) ;
  };

  // fill the histogram with forced ID assignement (book on demand)
  virtual AIDA::IHistogram1D*  plot1D
  ( const double        value        ,
    const HistoID&      ID           ,
    const std::string&  title        ,
    const double        low          ,
    const double        high         ,
    const unsigned long bins   = 100 ,
    const double        weight = 1.0 ) const
  {
    return GaudiHistoTool::plot1D
      ( value , ID , title , low , high , bins , weight ) ;
  };

  // ================================= 2D Histograms ========================================

  // fill the histogram (book on demand)
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
    const double        weight = 1.0 ) const
  {
    return GaudiHistoTool::plot2D
      ( valueX, valueY, title, lowX, highX, lowY, highY, binsX, binsY, weight ) ;
  };

  // fill the histogram (book on demand)
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
    const double        weight = 1.0 ) const
  {
    return GaudiHistoTool::plot2D
      ( valueX, valueY, ID, title, lowX, highX, lowY, highY, binsX, binsY, weight ) ;
  };

  // ================================= 3D Histograms ========================================

  // fill the histogram (book on demand)
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
    const double        weight = 1.0 ) const
  {
    return GaudiHistoTool::plot3D
      ( valueX, valueY, valueZ, title,
        lowX, highX, lowY, highY, lowZ, highZ, binsX, binsY, binsZ, weight ) ;
  };

  // fill the histogram (book on demand)
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
    const double        weight = 1.0 ) const
  {
    return GaudiHistoTool::plot3D
      ( valueX, valueY, valueZ, ID, title,
        lowX, highX, lowY, highY, lowZ, highZ, binsX, binsY, binsZ, weight ) ;
  };

public:

  // book the 1D histogram
  virtual AIDA::IHistogram1D*  book1D
  ( const std::string&  title        ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return GaudiHistoTool::book1D ( title , low , high , bins ) ;
  };

  // book the 2D histogram
  virtual AIDA::IHistogram2D*  book2D
  ( const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const
  {
    return GaudiHistoTool::book2D ( title, lowX, highX, binsX, lowY, highY, binsY ) ;
  }

  // book the 3D histogram
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
    const unsigned long binsZ   =  10 ) const
  {
    return GaudiHistoTool::book3D ( title,
                                    lowX, highX, binsX,
                                    lowY, highY, binsY,
                                    lowZ, highZ, binsZ ) ;
  }

  // book the 1D histogram with forced ID
  virtual AIDA::IHistogram1D*  book1D
  ( const HistoID&      ID           ,
    const std::string&  title  = ""  ,
    const double        low    =   0 ,
    const double        high   = 100 ,
    const unsigned long bins   = 100 ) const
  {
    return GaudiHistoTool::book1D ( ID , title , low , high , bins ) ;
  };

  // book the 2D histogram with forced ID
  virtual AIDA::IHistogram2D*  book2D
  ( const HistoID&      ID            ,
    const std::string&  title         ,
    const double        lowX    =   0 ,
    const double        highX   = 100 ,
    const unsigned long binsX   =  50 ,
    const double        lowY    =   0 ,
    const double        highY   = 100 ,
    const unsigned long binsY   =  50 ) const
  {
    return GaudiHistoTool::book2D ( ID, title, lowX, highX, binsX, lowY, highY, binsY ) ;
  }

  // book the 3D histogram with forced ID
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
    const unsigned long binsZ   =  10 ) const
  {
    return GaudiHistoTool::book3D ( ID, title,
                                    lowX, highX, binsX,
                                    lowY, highY, binsY,
                                    lowZ, highZ, binsZ ) ;
  }

  // fill the histo with the value and weight
  virtual AIDA::IHistogram1D* fill
  ( AIDA::IHistogram1D* histo  ,
    const double        value  ,
    const double        weight ,
    const std::string&  title  = "") const
  {
    return GaudiHistoTool::fill ( histo , value , weight , title ) ;
  };

  // fill the 2D histo with the value and weight
  virtual AIDA::IHistogram2D* fill
  ( AIDA::IHistogram2D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        weight ,
    const std::string&  title  = "" ) const
  {
    return GaudiHistoTool::fill ( histo , valueX , valueY , weight , title ) ;
  };

  // fill the 3D histo with the value and weight
  virtual AIDA::IHistogram3D* fill
  ( AIDA::IHistogram3D* histo  ,
    const double        valueX ,
    const double        valueY ,
    const double        valueZ ,
    const double        weight ,
    const std::string&  title  = "" ) const
  {
    return GaudiHistoTool::fill ( histo , valueX , valueY , valueZ, weight , title ) ;
  };

  // access the EXISTING 1D histogram by title
  virtual AIDA::IHistogram1D*  histo1D ( const std::string& title  )  const
  { return GaudiHistoTool::histo1D ( title ) ; }

  // access the EXISTING 1D histogram by ID
  virtual AIDA::IHistogram1D*  histo1D ( const HistoID&     ID     )  const
  { return GaudiHistoTool::histo1D ( ID ) ; }

  // access the EXISTING 2D histogram by title
  virtual AIDA::IHistogram2D*  histo2D ( const std::string& title  )  const
  { return GaudiHistoTool::histo2D ( title ) ; }

  // access the EXISTING 2D histogram by ID
  virtual AIDA::IHistogram2D*  histo2D ( const HistoID&     ID     )  const
  { return GaudiHistoTool::histo2D ( ID ) ; }

  // access the EXISTING 3D histogram by title
  virtual AIDA::IHistogram3D*  histo3D ( const std::string& title  )  const
  { return GaudiHistoTool::histo3D ( title ) ; }

  // access the EXISTING 3D histogram by ID
  virtual AIDA::IHistogram3D*  histo3D ( const HistoID&     ID     )  const
  { return GaudiHistoTool::histo3D ( ID ) ; }

  // check the existence AND validity of the histogram with given title
  virtual bool histoExists ( const std::string& title  )  const
  { return GaudiHistoTool::histoExists ( title ) ; }

  // check the existence AND validity of the histogram with given title
  virtual bool histoExists ( const HistoID&     ID     )  const
  { return GaudiHistoTool::histoExists ( ID ) ; }

private:

  HistoTool() ;
  HistoTool           ( const HistoTool& ) ;
  HistoTool& operator=( const HistoTool& ) ;

};

#endif // GAUDIALG_HISTOTOOL_H
