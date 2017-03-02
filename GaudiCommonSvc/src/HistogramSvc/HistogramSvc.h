// ============================================================================
#ifndef GAUDISVC_HISTOGRAMSVC_H
#define GAUDISVC_HISTOGRAMSVC_H 1
// ============================================================================
// Include Files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/HistoProperty.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/System.h"
// ============================================================================
// AIDA
// ============================================================================
/// @FIXME: AIDA interfaces visibility
#include "AIDA/IAnnotation.h"
#include "AIDA/IHistogramFactory.h"
#include "AIDA_visibility_hack.h"
// ============================================================================
// local (+PI)
// ============================================================================
#include "Axis.h"
#include "GaudiPI.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
// ============================================================================
namespace AIDA
{
  class ICloud1D;
  class ICloud2D;
  class ICloud3D;
}

namespace detail {
  template <class T>
  static DataObject* cast(T* p)
  {
    DataObject* q = dynamic_cast<DataObject*>( p );
    if ( !q && p ) {
      throw std::runtime_error( "HistogramSvc: Unexpected object type." );
    }
    return q;
  }
}

// ============================================================================
// Forward declarations
#define BINS( x ) n##x, low##x, up##x
// ============================================================================
/** @class HistogramSvc HistogramSvc.h
 *
 *  HistogramSvc class definition
 *
 */
class HistogramSvc : public extends<DataSvc, IHistogramSvc>, virtual public AIDA::IHistogramFactory
{

private:

  void not_implemented() const
  {
    error() << "Sorry, not yet implemented..." << endmsg;
  }

protected:

  typedef AIDA::IHistogram3D H3D;
  typedef AIDA::IProfile2D P2D;
  typedef AIDA::IBaseHistogram Base;

  struct Helper {
    HistogramSvc* m_svc;
    Helper( HistogramSvc* p ) : m_svc( p ) {}
    template <class A1, class A3>
    StatusCode retrieve( A1 a1, A3*& a3 )
    {
      DataObject* pObject = nullptr;
      StatusCode sc       = m_svc->DataSvc::retrieveObject( a1, pObject );
      a3                  = dynamic_cast<A3*>( pObject );
      return sc;
    }
    template <class A1, class A2, class A3>
    StatusCode retrieve( A1 a1, A2 a2, A3*& a3 )
    {
      DataObject* pObject = nullptr;
      StatusCode sc       = m_svc->DataSvc::retrieveObject( a1, a2, pObject );
      a3                  = dynamic_cast<A3*>( pObject );
      return sc;
    }
    template <class A1, class A3>
    StatusCode find( A1 a1, A3*& a3 )
    {
      DataObject* pObject = nullptr;
      StatusCode sc       = m_svc->DataSvc::findObject( a1, pObject );
      a3                  = dynamic_cast<A3*>( pObject );
      return sc;
    }
    template <class A1, class A2, class A3>
    StatusCode find( A1 a1, A2 a2, A3*& a3 )
    {
      DataObject* pObject = nullptr;
      StatusCode sc       = m_svc->DataSvc::findObject( a1, a2, pObject );
      a3                  = dynamic_cast<A3*>( pObject );
      return sc;
    }
    template <class R, class S, class T1, class T2>
    static R* act( R* res, const S& b, void ( T1::*pmf )( const T2*, Double_t ), Double_t scale )
    {
      T1* h1 = Gaudi::getRepresentation<R, T1>( *res );
      T1* h2 = Gaudi::getRepresentation<R, T1>( b );
      if ( h1 && h2 ) {
        ( h1->*pmf )( h2, scale );
        return res;
      }
      return nullptr;
    }
    template <class R, class S, class T1, class T2>
    static R* act( R* res, const S& b, Bool_t ( T1::*pmf )( const T2*, Double_t ), Double_t scale )
    {
      T1* h1 = Gaudi::getRepresentation<R, T1>( *res );
      T1* h2 = Gaudi::getRepresentation<R, T1>( b );
      if ( h1 && h2 ) {
        ( h1->*pmf )( h2, scale );
        return res;
      }
      return nullptr;
    }
    template <class R, class S, class T1, class T2>
    static R* act( R* res, const S& b, void ( T1::*pmf )( const T2* ) )
    {
      T1* h1 = Gaudi::getRepresentation<R, T1>( *res );
      T1* h2 = Gaudi::getRepresentation<R, T1>( b );
      if ( h1 && h2 ) {
        ( h1->*pmf )( h2 );
        return res;
      }
      return nullptr;
    }
    template <class R, class S, class T1, class T2>
    static R* act( R* res, const S& b, Bool_t ( T1::*pmf )( const T2* ) )
    {
      T1* h1 = Gaudi::getRepresentation<R, T1>( *res );
      T1* h2 = Gaudi::getRepresentation<R, T1>( b );
      if ( h1 && h2 ) {
        ( h1->*pmf )( h2 );
        return res;
      }
      return nullptr;
    }
  };

public:

  using Edges = std::vector<double>;
  using DBaseEntries = std::vector<std::string>;

  /** Statndard Constructor
   *  @param name service name
   *  @param pointer to service locator interface
   */
  HistogramSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~HistogramSvc() override;

  /** Split full path into its components
   *  @param full Full path of the object
   *  @param dir  Resulting directory path
   *  @param obj  Resulting object path
   */
  std::pair<std::string, std::string> i_splitPath( const std::string& full );

  /** Connect input histogram file to the service
   *  @param ident   [IN]   Input specification
   */
  StatusCode connectInput( const std::string& ident );

  template <class T>
  inline T* i_book( DataObject* pPar, const std::string& rel, const std::string& title, const std::pair<DataObject*, T*>& o )
  {
    if ( o.first && registerObject( pPar, rel, (Base*)o.second ).isSuccess() ) return o.second;
    delete o.first;
    throw GaudiException( "Cannot book " + System::typeinfoName( typeid( T ) ) + " " + title, "HistogramSvc",
                          StatusCode::FAILURE );
  }

  /// Helper for 2D projections
  AIDA::IHistogram2D* i_project( const std::string& nameAndTitle, const AIDA::IHistogram3D& h, const std::string& dir );

public:

  /// Initialise the service
  StatusCode initialize() override;
  /// Initialise the service
  StatusCode reinitialize() override;
  /// finalize the service
  StatusCode finalize() override;

  /// Retrieve the AIDA HistogramFactory interface
  AIDA::IHistogramFactory* histogramFactory() override { return this; }

  // ==========================================================================
  // Book 1D histogram with fix binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the fullpath)
                         has to be an integer number
                         otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param nx          Number of bins on the axis X/Y
      @param lx          Lower histogram edge on the axis X/Y
      @param ux          Upper histogram edge on the axis X/Y
  */
  AIDA::IHistogram1D* book( const std::string& par, const std::string& rel, const std::string& title,
                            int nx, double lowx, double upx ) override;

  AIDA::IHistogram1D* book( const std::string& par, int hID, const std::string& title,
                            int nx, double lowx, double upx ) override;

  AIDA::IHistogram1D* book( DataObject* pPar, int hID, const std::string& title,
                            int nx, double lowx, double upx ) override;

  AIDA::IHistogram1D* book( DataObject* pPar, const std::string& rel, const std::string& title,
                            int nx, double lowx, double upx ) override;

  virtual AIDA::IHistogram1D* book( const std::pair<std::string, std::string>& loc, const std::string& title,
                                    int nx, double lowx, double upx );

  AIDA::IHistogram1D* book( const std::string& full, const std::string& title,
                            int nx, double lowx, double upx ) override;

  // ==========================================================================
  // Book 1D Profile histogram with fix binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the fullpath)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param nx          Number of bins on the axis X/Y
      @param lx          Lower histogram edge on the axis X/Y
      @param ux          Upper histogram edge on the axis X/Y
  */
  AIDA::IProfile1D* bookProf( const std::string& par, const std::string& rel, const std::string& title,
                              int nx, double lowx, double upx, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( const std::string& par, int hID, const std::string& title,
                              int nx, double lowx, double upx, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( DataObject* pPar, int hID, const std::string& title,
                              int nx, double lowx, double upx, const std::string& opt ) override;

  virtual AIDA::IProfile1D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      int nx, double lowx, double upx, const std::string& opt );

  AIDA::IProfile1D* bookProf( const std::string& full, const std::string& title,
                              int nx, double lowx, double upx, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( DataObject* pPar, const std::string& rel, const std::string& title,
                              int nx, double lowx, double upx, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( const std::string& par, const std::string& rel, const std::string& title,
                              int nx, double lowx, double upx,
                              double upper, double lower, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( const std::string& par, int hID, const std::string& title,
                              int nx, double lowx, double upx,
                              double upper, double lower, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( DataObject* pPar, int hID, const std::string& title,
                              int nx, double lowx, double upx,
                              double upper, double lower, const std::string& opt ) override;

  virtual AIDA::IProfile1D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      double upper, double lower, const std::string& opt );

  AIDA::IProfile1D* bookProf( const std::string& full, const std::string& title,
                              int nx, double lowx, double upx,
                              double upper, double lower, const std::string& opt ) override;

  AIDA::IProfile1D* bookProf( DataObject* pPar, const std::string& rel, const std::string& title,
                              int nx, double lowx, double upx,
                              double upper, double lower, const std::string& opt ) override;

  // ==========================================================================
  // Book 1D histogram with variable binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the fullpath)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param e           Bin edges for variable binned histogram.
  */
  AIDA::IHistogram1D* book( const std::string& par, int hID,
                            const std::string& title, Edges e ) override;

  AIDA::IHistogram1D* book( DataObject* pPar, int hID,
                            const std::string& title, Edges e ) override;

  AIDA::IHistogram1D* book( const std::string& par, const std::string& rel,
                            const std::string& title, Edges e ) override;

  virtual AIDA::IHistogram1D* book( const std::pair<std::string, std::string>& loc,
                                    const std::string& title, Edges e );

  AIDA::IHistogram1D* book( const std::string& full,
                            const std::string& title, Edges e ) override;

  AIDA::IHistogram1D* book( DataObject* pPar, const std::string& rel,
                            const std::string& title, Edges e ) override;

  // ==========================================================================
  // Book 1D profile histogram with variable binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param e           Bin edges for variable binned histogram.
  */
  AIDA::IProfile1D* bookProf( const std::string& full,
                              const std::string& title, Edges e ) override;

  AIDA::IProfile1D* bookProf( const std::string& par, const std::string& rel,
                              const std::string& title, Edges e ) override;

  AIDA::IProfile1D* bookProf( const std::string& par, int hID,
                              const std::string& title, Edges e ) override;

  AIDA::IProfile1D* bookProf( DataObject* pPar, int hID,
                              const std::string& title, Edges e ) override;

  virtual AIDA::IProfile1D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title, Edges e );

  AIDA::IProfile1D* bookProf( DataObject* pPar, const std::string& rel,
                              const std::string& title, Edges e ) override;

  virtual AIDA::IProfile1D* bookProf( const std::string& full, const std::string& title,
                                      Edges e, double upper, double lower );

  virtual AIDA::IProfile1D* bookProf( const std::string& par, const std::string& rel,
                                      const std::string& title, Edges e,
                                      double upper, double lower );

  virtual AIDA::IProfile1D* bookProf( const std::string& par, int hID,
                                      const std::string& title, Edges e,
                                      double upper, double lower );

  virtual AIDA::IProfile1D* bookProf( DataObject* pPar, int hID,
                                      const std::string& title, Edges e,
                                      double upper, double lower );

  virtual AIDA::IProfile1D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title, Edges e,
                                      double upper, double lower );

  virtual AIDA::IProfile1D* bookProf( DataObject* pPar, const std::string& rel,
                                      const std::string& title, Edges e,
                                      double upper, double lower );

  // ==========================================================================
  // Book 2D histogram with fixed binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param DBINS(x/y)  Macro for bin definition in X/Y. Expands to:
                         int nx:    Number of bins on the axis X/Y
                         double lx: Lower histogram edge on the axis X/Y
                         double ux: Upper histogram edge on the axis X/Y
  */
  AIDA::IHistogram2D* book( const std::string& full, const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy ) override;

  AIDA::IHistogram2D* book( const std::string& par, const std::string& rel,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy ) override;

  AIDA::IHistogram2D* book( const std::string& par, int hID, const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy ) override;

  virtual AIDA::IHistogram2D* book( const std::pair<std::string, std::string>& loc,
                                    const std::string& title,
                                    int nx, double lowx, double upx,
                                    int ny, double lowy, double upy );

  AIDA::IHistogram2D* book( DataObject* pPar, int hID, const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy ) override;

  AIDA::IHistogram2D* book( DataObject* pPar, const std::string& rel,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy ) override;


  // ==========================================================================
  // Book 2D profile histogram with fixed binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param DBINS(x/y)  Macro for bin definition in X/Y. Expands to:
                         int nx:    Number of bins on the axis X/Y
                         double lx: Lower histogram edge on the axis X/Y
                         double ux: Upper histogram edge on the axis X/Y
  */
  virtual AIDA::IProfile2D* bookProf( const std::string& full,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::string& par, const std::string& rel,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::string& par, int hID,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( DataObject* pPar, int hID, const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( DataObject* pPar, const std::string& rel,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy,
                                      double upper, double lower );

  AIDA::IProfile2D* bookProf( const std::string& full,
                              const std::string& title,
                              int nx, double lowx, double upx,
                              int ny, double lowy, double upy ) override;

  AIDA::IProfile2D* bookProf( const std::string& par, const std::string& rel,
                              const std::string& title,
                              int nx, double lowx, double upx,
                              int ny, double lowy, double upy ) override;

  virtual AIDA::IProfile2D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      int nx, double lowx, double upx,
                                      int ny, double lowy, double upy );

  AIDA::IProfile2D* bookProf( const std::string& par, int hID,
                              const std::string& title,
                              int nx, double lowx, double upx,
                              int ny, double lowy, double upy ) override;

  AIDA::IProfile2D* bookProf( DataObject* pPar, int hID,
                              const std::string& title,
                              int nx, double lowx, double upx,
                              int ny, double lowy, double upy ) override;

  AIDA::IProfile2D* bookProf( DataObject* pPar, const std::string& rel,
                              const std::string& title,
                              int nx, double lowx, double upx,
                              int ny, double lowy, double upy ) override;

  // ==========================================================================
  // Book 2D histogram with variable binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param x/y         Bin edges for variable binned histogram in X/Y.
  */
  AIDA::IHistogram2D* book( const std::string& full,
                            const std::string& title,
                            Edges x, Edges y ) override;

  AIDA::IHistogram2D* book( const std::string& par, const std::string& rel,
                            const std::string& title,
                            Edges x, Edges y ) override;

  AIDA::IHistogram2D* book( const std::string& par, int hID,
                            const std::string& title,
                            Edges x, Edges y ) override;

  virtual AIDA::IHistogram2D* book( const std::pair<std::string, std::string>& loc,
                                    const std::string& title,
                                    Edges x, Edges y );

  AIDA::IHistogram2D* book( DataObject* pPar, int hID,
                            const std::string& title,
                            Edges x, Edges y ) override;

  AIDA::IHistogram2D* book( DataObject* pPar, const std::string& rel,
                            const std::string& title,
                            Edges x, Edges y ) override;

  // ==========================================================================
  // Book 2D profile histogram with variable binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param x/y         Bin edges for variable binned histogram in X/Y.
  */
  AIDA::IProfile2D* bookProf( const std::string& full,
                              const std::string& title,
                              Edges x, Edges y ) override;

  AIDA::IProfile2D* bookProf( const std::string& par, const std::string& rel,
                              const std::string& title,
                              Edges x, Edges y ) override;

  AIDA::IProfile2D* bookProf( const std::string& par, int hID,
                              const std::string& title,
                              Edges x, Edges y ) override;

  AIDA::IProfile2D* bookProf( DataObject* pPar, int hID,
                              const std::string& title,
                              Edges x, Edges y ) override;

  virtual AIDA::IProfile2D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      Edges x, Edges y );

  AIDA::IProfile2D* bookProf( DataObject* pPar, const std::string& rel,
                              const std::string& title,
                              Edges x, Edges y ) override;

  virtual AIDA::IProfile2D* bookProf( const std::string& full,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::string& par, const std::string& rel,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::string& par, int hID,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( DataObject* pPar, int hID,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( const std::pair<std::string, std::string>& loc,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  virtual AIDA::IProfile2D* bookProf( DataObject* pPar, const std::string& rel,
                                      const std::string& title,
                                      Edges x, Edges y,
                                      double upper, double lower );

  // ==========================================================================
  // Book 3D histogram with fixed binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                         (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel         Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param n{x,y,z}    Number of bins on the axis X/Y/Z
      @param l{x,y,z}    Lower histogram edge on the axis X/Y/Z
      @param u{x,y,z}    Upper histogram edge on the axis X/Y/Z
  */
  AIDA::IHistogram3D* book( const std::string& full,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy,
                            int nz, double lowz, double upz ) override;

  AIDA::IHistogram3D* book( const std::string& par, const std::string& rel,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy,
                            int nz, double lowz, double upz ) override;

  AIDA::IHistogram3D* book( const std::string& par, int hID,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy,
                            int nz, double lowz, double upz ) override;

  AIDA::IHistogram3D* book( DataObject* pPar, int hID,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy,
                            int nz, double lowz, double upz ) override;

  virtual AIDA::IHistogram3D* book( const std::pair<std::string, std::string>& loc,
                                    const std::string& title,
                                    int nx, double lowx, double upx,
                                    int ny, double lowy, double upy,
                                    int nz, double lowz, double upz );

  AIDA::IHistogram3D* book( DataObject* pPar, const std::string& rel,
                            const std::string& title,
                            int nx, double lowx, double upx,
                            int ny, double lowy, double upy,
                            int nz, double lowz, double upz ) override;

  // ==========================================================================
  // Book 3D histogram with variable binning
  // ==========================================================================
  /** Book histogram and register it with the histogram data store
      @param full        Full path to the node of the object.
                         The basename (last part of the full)
                         has to be an integer number
                        (otherwise conversion to HBOOK is not possible)
      @param par         Path to parent node of the object,
                         the directory the histogram will be stored in.
      @param pPar        Pointer to parent node
      @param rel     Histogram identifier (std::string), the relative path
                         to the object with respect to the parent node
      @param hID         Histogram identifier (int) of the histogram
      @param title       Title property of the histogram
      @param binsX/Y/Z   Number of bins on the axis X/Y/Z
      @param lowX/Y/Z    Lower histogram edge on the axis X/Y/Z
      @param highX/Y/Z   Upper histogram edge on the axis X/Y/Z
  */
  AIDA::IHistogram3D* book( const std::string& full,
                            const std::string& title,
                            Edges x, Edges y, Edges z ) override;

  AIDA::IHistogram3D* book( const std::string& par, const std::string& rel,
                            const std::string& title,
                            Edges x, Edges y, Edges z ) override;

  AIDA::IHistogram3D* book( const std::string& par, int hID,
                            const std::string& title,
                            Edges x, Edges y, Edges z ) override;

  AIDA::IHistogram3D* book( DataObject* pPar, int hID,
                            const std::string& title,
                            Edges x, Edges y, Edges z ) override;

  virtual AIDA::IHistogram3D* book( const std::pair<std::string, std::string>& loc,
                                    const std::string& title,
                                    Edges x, Edges y, Edges z );

  AIDA::IHistogram3D* book( DataObject* pPar, const std::string& rel,
                            const std::string& title,
                            Edges x, Edges y, Edges z ) override;

  // ==========================================================================
  // Register histogram with the data store
  // ==========================================================================
  StatusCode registerObject( const std::string& parent, const std::string& rel, Base* obj ) override;

  StatusCode registerObject( const std::string& parent, int item, Base* obj ) override;

  StatusCode registerObject( Base* pPar, const std::string& rel, Base* obj ) override;

  StatusCode registerObject( DataObject* pPar, int item, Base* obj ) override;

  StatusCode registerObject( Base* pPar, int item, Base* obj ) override;

  StatusCode registerObject( const std::string& full, Base* obj ) override;

  StatusCode registerObject( DataObject* pPar, const std::string& rel, Base* obj ) override;

  // ==========================================================================
  // Unregister histogram from the data store
  // ==========================================================================
  StatusCode unregisterObject( Base* obj ) override;

  StatusCode unregisterObject( Base* obj, const std::string& objectPath ) override;

  StatusCode unregisterObject( Base* obj, int item ) override;

  // ==========================================================================
  // Retrieve histogram from data store
  // ==========================================================================
  StatusCode retrieveObject( IRegistry* pReg, const std::string& path,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( IRegistry* pReg, const std::string& path,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( IRegistry* pReg, const std::string& path,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( IRegistry* pReg, const std::string& path,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( IRegistry* pReg, const std::string& path,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( const std::string& full,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( const std::string& full,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( const std::string& full,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( const std::string& full,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( const std::string& full,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, const std::string& rel,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, const std::string& rel,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, const std::string& rel,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, const std::string& rel,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, const std::string& rel,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, int item,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, int item,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, int item,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, int item,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( const std::string& parent, int item,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, const std::string& item,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, const std::string& item,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, const std::string& item,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, const std::string& item,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, const std::string& item,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, int item,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, int item,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, int item,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, int item,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( DataObject* par, int item,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( Base* par, int item,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( Base* par, int item,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( Base* par, int item,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( Base* par, int item,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( Base* par, int item,
                             AIDA::IHistogram3D*& obj ) override;

  StatusCode retrieveObject( Base* par, const std::string& item,
                             AIDA::IProfile1D*& obj ) override;

  StatusCode retrieveObject( Base* par, const std::string& item,
                             AIDA::IProfile2D*& obj ) override;

  StatusCode retrieveObject( Base* par, const std::string& item,
                             AIDA::IHistogram1D*& obj ) override;

  StatusCode retrieveObject( Base* par, const std::string& item,
                             AIDA::IHistogram2D*& obj ) override;

  StatusCode retrieveObject( Base* par, const std::string& item,
                             AIDA::IHistogram3D*& obj ) override;

  // ==========================================================================
  // Find histogram identified by its full path in the data store
  // ==========================================================================
  StatusCode findObject( IRegistry* pReg, const std::string& path,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( IRegistry* pReg, const std::string& path,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( IRegistry* pReg, const std::string& path,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( IRegistry* pReg, const std::string& path,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( IRegistry* pReg, const std::string& path,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( const std::string& full,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( const std::string& full,
                         AIDA::IProfile2D*& obj ) override ;

  StatusCode findObject( const std::string& full,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( const std::string& full,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( const std::string& full,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( const std::string& par, const std::string& rel,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( const std::string& par, const std::string& rel,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( const std::string& par, const std::string& rel,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( const std::string& par, const std::string& rel,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( const std::string& par, const std::string& rel,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( const std::string& par, int item,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( const std::string& par, int item,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( const std::string& par, int item,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( const std::string& par, int item,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( const std::string& par, int item,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( DataObject* par, int item,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( DataObject* par, int item,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( DataObject* par, int item,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( DataObject* par, int item,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( DataObject* par, int item,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( DataObject* par, const std::string& item,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( DataObject* par, const std::string& item,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( DataObject* par, const std::string& item,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( DataObject* par, const std::string& item,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( DataObject* par, const std::string& item,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( Base* par, int item,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( Base* par, int item,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( Base* par, int item,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( Base* par, int item,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( Base* par, int item,
                         AIDA::IHistogram3D*& obj ) override;

  StatusCode findObject( Base* par, const std::string& item,
                         AIDA::IProfile1D*& obj ) override;

  StatusCode findObject( Base* par, const std::string& item,
                         AIDA::IProfile2D*& obj ) override;

  StatusCode findObject( Base* par, const std::string& item,
                         AIDA::IHistogram1D*& obj ) override;

  StatusCode findObject( Base* par, const std::string& item,
                         AIDA::IHistogram2D*& obj ) override;

  StatusCode findObject( Base* par, const std::string& item,
                         AIDA::IHistogram3D*& obj ) override;

  // ==========================================================================
  // Projections and slices.
  // ==========================================================================
  AIDA::IHistogram1D* projectionX( const std::string& name,
                                   const AIDA::IHistogram2D& h ) override;

  AIDA::IHistogram1D* projectionY( const std::string& name,
                                   const AIDA::IHistogram2D& h ) override;

  AIDA::IHistogram1D* sliceX( const std::string& name, const AIDA::IHistogram2D& h,
                              int indexY ) override ;

  AIDA::IHistogram1D* sliceY( const std::string& name, const AIDA::IHistogram2D& h,
                              int indexX ) override ;

  AIDA::IHistogram1D* sliceX( const std::string& name, const AIDA::IHistogram2D& h,
                              int indexY1, int indexY2 ) override;

  AIDA::IHistogram1D* sliceY( const std::string& name, const AIDA::IHistogram2D& h,
                              int indexX1, int indexX2 ) override;

  bool destroy( IBaseHistogram* hist ) override;

  AIDA::IHistogram1D* add( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                           const AIDA::IHistogram1D& b ) override;

  AIDA::IHistogram1D* subtract( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                const AIDA::IHistogram1D& b ) override;

  AIDA::IHistogram1D* multiply( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                                const AIDA::IHistogram1D& b ) override;

  AIDA::IHistogram1D* divide( const std::string& nameAndTitle, const AIDA::IHistogram1D& a,
                              const AIDA::IHistogram1D& b ) override;

  AIDA::IHistogram2D* add( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                           const AIDA::IHistogram2D& b ) override;

  AIDA::IHistogram2D* subtract( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                const AIDA::IHistogram2D& b ) override;

  AIDA::IHistogram2D* multiply( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                                const AIDA::IHistogram2D& b ) override;

  AIDA::IHistogram2D* divide( const std::string& nameAndTitle, const AIDA::IHistogram2D& a,
                              const AIDA::IHistogram2D& b ) override;

  AIDA::IHistogram3D* add( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                           const AIDA::IHistogram3D& b ) override;

  AIDA::IHistogram3D* subtract( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                const AIDA::IHistogram3D& b ) override;

  AIDA::IHistogram3D* multiply( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                                const AIDA::IHistogram3D& b ) override;

  AIDA::IHistogram3D* divide( const std::string& nameAndTitle, const AIDA::IHistogram3D& a,
                              const AIDA::IHistogram3D& b ) override;

  AIDA::IHistogram2D* projectionXY( const std::string& nameAndTitle,
                                    const AIDA::IHistogram3D& h ) override ;

  AIDA::IHistogram2D* projectionXZ( const std::string& nameAndTitle,
                                    const AIDA::IHistogram3D& h ) override ;

  AIDA::IHistogram2D* projectionYZ( const std::string& nameAndTitle,
                                    const AIDA::IHistogram3D& h ) override ;


  AIDA::IHistogram2D* sliceXY( const std::string& /* nameAndTitle */,
                               const AIDA::IHistogram3D& /* h */,
                               int /* low */, int /* high */ ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::IHistogram2D* sliceXZ( const std::string& /* nameAndTitle */,
                               const AIDA::IHistogram3D& /* h */,
                               int /* low */, int /* high */ ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::IHistogram2D* sliceYZ( const std::string& /* nameAndTitle */,
                               const AIDA::IHistogram3D& /* h */,
                               int /* low */, int /* high */ ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::IHistogram1D* createHistogram1D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx );

  AIDA::IHistogram1D* createHistogram1D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram1D* createHistogram1D( const std::string& name, const std::string& title,
                                         const Edges& x,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram1D* createHistogram1D( const std::string& nameAndTitle,
                                         int nx, double lowx, double upx ) override;

  AIDA::IHistogram1D* createCopy( const std::string& full,
                                  const AIDA::IHistogram1D& h ) override;

  AIDA::IHistogram1D* createCopy( const std::string& par, const std::string& rel,
                                  const AIDA::IHistogram1D& h );

  AIDA::IHistogram1D* createCopy( const std::pair<std::string, std::string>& loc,
                                  const AIDA::IHistogram1D& h );

  AIDA::IHistogram1D* createCopy( DataObject* pPar, const std::string& rel,
                                  const AIDA::IHistogram1D& h );

  AIDA::IHistogram2D* createHistogram2D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy );

  AIDA::IHistogram2D* createHistogram2D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram2D* createHistogram2D( const std::string& name, const std::string& title,
                                         const Edges& x, const Edges& y,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram2D* createHistogram2D( const std::string& nameAndTitle,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy ) override;

  AIDA::IHistogram2D* createCopy( const std::string& full,
                                  const AIDA::IHistogram2D& h ) override;

  AIDA::IHistogram2D* createCopy( const std::string& par, const std::string& rel,
                                  const AIDA::IHistogram2D& h );

  AIDA::IHistogram2D* createCopy( const std::pair<std::string, std::string>& loc,
                                  const AIDA::IHistogram2D& h );

  AIDA::IHistogram2D* createCopy( DataObject* pPar, const std::string& rel,
                                  const AIDA::IHistogram2D& h );

  AIDA::IHistogram3D* createHistogram3D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy,
                                         int nz, double lowz, double upz );

  AIDA::IHistogram3D* createHistogram3D( const std::string& name, const std::string& tit,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy,
                                         int nz, double lowz, double upz,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram3D* createHistogram3D( const std::string& name, const std::string& title,
                                         const Edges& x, const Edges& y, const Edges& z,
                                         const std::string& /*opt*/ ) override;

  AIDA::IHistogram3D* createHistogram3D( const std::string& nameAndTitle,
                                         int nx, double lowx, double upx,
                                         int ny, double lowy, double upy,
                                         int nz, double lowz, double upz ) override;

  AIDA::IHistogram3D* createCopy( const std::string& full,
                                  const AIDA::IHistogram3D& h ) override;

  AIDA::IHistogram3D* createCopy( const std::string& par, const std::string& rel,
                                  const AIDA::IHistogram3D& h );

  AIDA::IHistogram3D* createCopy( const std::pair<std::string, std::string>& loc,
                                  const AIDA::IHistogram3D& h );

  AIDA::IHistogram3D* createCopy( DataObject* pPar, const std::string& rel,
                                  const AIDA::IHistogram3D& h );

   
  AIDA::IProfile1D* createProfile1D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     const std::string& opt ) override;

  AIDA::IProfile1D* createProfile1D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     double upper, double lower,
                                     const std::string& opt ) override;

  AIDA::IProfile1D* createProfile1D( const std::string& name,
                                     const std::string& title, const Edges& x,
                                     const std::string& /* opt */ ) override;

  AIDA::IProfile1D* createProfile1D( const std::string& name,
                                     const std::string& title, const Edges& x,
                                     double upper, double lower,
                                     const std::string& /* opt */ ) override;

  AIDA::IProfile1D* createProfile1D( const std::string& nametit,
                                     int nx, double lowx, double upx ) override;

  AIDA::IProfile1D* createProfile1D( const std::string& nametit,
                                     int nx, double lowx, double upx,
                                     double upper, double lower ) override;

  AIDA::IProfile1D* createCopy( const std::string& full,
                                const AIDA::IProfile1D& h ) override;

  AIDA::IProfile1D* createCopy( const std::string& par, const std::string& rel,
                                const AIDA::IProfile1D& h );

  AIDA::IProfile1D* createCopy( const std::pair<std::string, std::string>& loc,
                                const AIDA::IProfile1D& h );

  AIDA::IProfile1D* createCopy( DataObject* pPar, const std::string& rel,
                                const AIDA::IProfile1D& h );

  AIDA::IProfile2D* createProfile2D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy );

  AIDA::IProfile2D* createProfile2D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy,
                                     const std::string& /*opt*/ ) override;

  AIDA::IProfile2D* createProfile2D( const std::string& name,
                                     const std::string& title,
                                     const Edges& x, const Edges& y,
                                     const std::string& /*opt*/ ) override;

  AIDA::IProfile2D* createProfile2D( const std::string& nameAndTitle,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy ) override;

  AIDA::IProfile2D* createProfile2D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy,
                                     double upper, double lower );

  AIDA::IProfile2D* createProfile2D( const std::string& name, const std::string& tit,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy,
                                     double upper, double lower, const std::string& /*opt*/ ) override;

  AIDA::IProfile2D* createProfile2D( const std::string& name,
                                     const std::string& title,
                                     const Edges& x, const Edges& y,
                                     double upper, double lower,
                                     const std::string& /*opt*/ ) override;

  AIDA::IProfile2D* createProfile2D( const std::string& nameAndTitle,
                                     int nx, double lowx, double upx,
                                     int ny, double lowy, double upy,
                                     double upper, double lower ) override;

  AIDA::IProfile2D* createCopy( const std::string& full,
                                const AIDA::IProfile2D& h ) override;

  AIDA::IProfile2D* createCopy( const std::string& par, const std::string& rel,
                                const AIDA::IProfile2D& h );

  AIDA::IProfile2D* createCopy( const std::pair<std::string, std::string>& loc,
                                const AIDA::IProfile2D& h );

  AIDA::IProfile2D* createCopy( DataObject* pPar, const std::string& rel,
                                const AIDA::IProfile2D& h );

  AIDA::ICloud1D* createCloud1D( const std::string&, const std::string&,
                                 int, const std::string& ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud1D* createCloud1D(const std::string&) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud1D* createCopy(const std::string&, const AIDA::ICloud1D&) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud2D* createCloud2D( const std::string&, const std::string&,
                                 int, const std::string& ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud2D* createCloud2D(const std::string&) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud2D* createCopy(const std::string&, const AIDA::ICloud2D&) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud3D* createCloud3D(const std::string&, const std::string&, int, const std::string&) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud3D* createCloud3D( const std::string& ) override
  {
    not_implemented();
    return nullptr;
  }

  AIDA::ICloud3D* createCopy( const std::string&, const AIDA::ICloud3D& ) override
  {
    not_implemented();
    return nullptr;
  }

  /// Avoids a compiler warning about hidden functions.
  using IDataProviderSvc::registerObject;
  using IDataProviderSvc::unregisterObject;
  using IDataProviderSvc::retrieveObject;
  using IDataProviderSvc::findObject;

  /// Print (ASCII) the 1D histogram into the output stream
  std::ostream& print( Base* h, std::ostream& s = std::cout ) const override;

  /// Write (ASCII) the 1D histogram table into the output stream
  std::ostream& write( Base* h, std::ostream& s = std::cout ) const override;

  /// Write (ASCII) the 1D histogram table into a file
  int write( Base* h, const char* file_name ) const override;

  /// Create all directories in a given full path
  DataObject* createPath( const std::string& newPath ) override;

  /** Create a sub-directory in a directory.
   *  @param parentDir name of the parent directory
   *  @param subDir to identify the histogram object in the store
   */
  DataObject* createDirectory( const std::string& parentDir, const std::string& subDir ) override;

  /// handler to be invoked for updating property m_defs1D
  void update1Ddefs( Gaudi::Details::PropertyBase& );

  typedef std::map<std::string, Gaudi::Histo1DDef> Histo1DMap;

private:

  Gaudi::Property<DBaseEntries> m_input{this, "Input", {}, "input streams"};
  Gaudi::Property<Histo1DMap> m_defs1D{this, "Predefined1DHistos", {}, "histograms with predefined parameters"};

  // modified histograms:
  std::set<std::string> m_mods1D;

};
#endif // GAUDISVC_HISTOGRAMSVC_H
