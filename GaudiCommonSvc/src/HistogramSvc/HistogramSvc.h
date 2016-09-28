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
// ============================================================================
// Forward declarations
#define DBINS( x ) int n##x, double low##x, double up##x
#define BINS( x ) n##x, low##x, up##x
#define NOT_IMPLEMENTED                                                                                                \
  {                                                                                                                    \
    not_implemented();                                                                                                 \
    return 0;                                                                                                          \
  }
// ============================================================================
/** @class HistogramSvc HistogramSvc.h
 *
 *  HistogramSvc class definition
 *
 */
class HistogramSvc : public extends<DataSvc, IHistogramSvc>, virtual public AIDA::IHistogramFactory
{

private:
  typedef const std::pair<std::string, std::string>& STRPAIR;
  void not_implemented() const { error() << "Sorry, not yet implemented..." << endmsg; }
protected:
  typedef const std::string& CSTR;
  typedef std::vector<double> Edges;
  typedef std::vector<std::string> DBaseEntries;
  typedef AIDA::IHistogram1D H1D;
  typedef AIDA::IHistogram2D H2D;
  typedef AIDA::IHistogram3D H3D;
  typedef AIDA::IProfile1D P1D;
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
  std::pair<std::string, std::string> i_splitPath( CSTR full );

  /** Connect input histogram file to the service
   *  @param ident   [IN]   Input specification
   */
  StatusCode connectInput( CSTR ident );

  template <class T>
  inline T* i_book( DataObject* pPar, CSTR rel, CSTR title, const std::pair<DataObject*, T*>& o )
  {
    if ( o.first && registerObject( pPar, rel, (Base*)o.second ).isSuccess() ) return o.second;
    delete o.first;
    throw GaudiException( "Cannot book " + System::typeinfoName( typeid( T ) ) + " " + title, "HistogramSvc",
                          StatusCode::FAILURE );
  }
  template <class T>
  static DataObject* __cast( T* p )
  {
    DataObject* q = dynamic_cast<DataObject*>( p );
    if ( !q && p ) {
      throw std::runtime_error( "HistogramSvc: Unexpected object type." );
    }
    return q;
  }
  /// Helper for 2D projections
  H2D* i_project( CSTR nameAndTitle, const H3D& h, CSTR dir );

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
      @param DBINS(x)    Macro for bin definition. Expands to:
                         int nx:    Number of bins on the axis X/Y
                         double lx: Lower histogram edge on the axis X/Y
                         double ux: Upper histogram edge on the axis X/Y
  */
  H1D* book( CSTR par, CSTR rel, CSTR title, DBINS( x ) ) override
  {
    return book( createPath( par ), rel, title, BINS( x ) );
  }
  H1D* book( CSTR par, int hID, CSTR title, DBINS( x ) ) override
  {
    return book( par, std::to_string( hID ), title, BINS( x ) );
  }
  H1D* book( DataObject* pPar, int hID, CSTR title, DBINS( x ) ) override
  {
    return book( pPar, std::to_string( hID ), title, BINS( x ) );
  }
  H1D* book( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ) ) override;
  virtual H1D* book( STRPAIR loc, CSTR title, DBINS( x ) ) { return book( loc.first, loc.second, title, BINS( x ) ); }
  H1D* book( CSTR full, CSTR title, DBINS( x ) ) override { return book( i_splitPath( full ), title, BINS( x ) ); }
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
      @param DBINS(x)    Macro for bin definition. Expands to:
                         int nx:    Number of bins on the axis X/Y
                         double lx: Lower histogram edge on the axis X/Y
                         double ux: Upper histogram edge on the axis X/Y
  */
  P1D* bookProf( CSTR par, CSTR rel, CSTR title, DBINS( x ), CSTR opt ) override
  {
    return bookProf( createPath( par ), rel, title, BINS( x ), opt );
  }
  P1D* bookProf( CSTR par, int hID, CSTR title, DBINS( x ), CSTR opt ) override
  {
    return bookProf( par, std::to_string( hID ), title, BINS( x ), opt );
  }
  P1D* bookProf( DataObject* pPar, int hID, CSTR title, DBINS( x ), CSTR opt ) override
  {
    return bookProf( pPar, std::to_string( hID ), title, BINS( x ), opt );
  }
  virtual P1D* bookProf( STRPAIR loc, CSTR title, DBINS( x ), CSTR opt )
  {
    return bookProf( loc.first, loc.second, title, BINS( x ), opt );
  }
  P1D* bookProf( CSTR full, CSTR title, DBINS( x ), CSTR opt ) override
  {
    return bookProf( i_splitPath( full ), title, BINS( x ), opt );
  }
  P1D* bookProf( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), CSTR opt ) override
  {
    return i_book( pPar, rel, title, Gaudi::createProf1D( title, BINS( x ), 0, 0, opt ) );
  }

  P1D* bookProf( CSTR par, CSTR rel, CSTR title, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return bookProf( createPath( par ), rel, title, BINS( x ), upper, lower, opt );
  }
  P1D* bookProf( CSTR par, int hID, CSTR title, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return bookProf( par, std::to_string( hID ), title, BINS( x ), upper, lower, opt );
  }
  P1D* bookProf( DataObject* pPar, int hID, CSTR title, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return bookProf( pPar, std::to_string( hID ), title, BINS( x ), upper, lower, opt );
  }
  virtual P1D* bookProf( STRPAIR loc, CSTR title, DBINS( x ), double upper, double lower, CSTR opt )
  {
    return bookProf( loc.first, loc.second, title, BINS( x ), upper, lower, opt );
  }
  P1D* bookProf( CSTR full, CSTR title, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return bookProf( i_splitPath( full ), title, BINS( x ), upper, lower, opt );
  }
  P1D* bookProf( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return i_book( pPar, rel, title, Gaudi::createProf1D( title, BINS( x ), upper, lower, opt ) );
  }
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
  H1D* book( CSTR par, int hID, CSTR title, Edges e ) override { return book( par, std::to_string( hID ), title, e ); }
  H1D* book( DataObject* pPar, int hID, CSTR title, Edges e ) override
  {
    return book( pPar, std::to_string( hID ), title, e );
  }
  H1D* book( CSTR par, CSTR rel, CSTR title, Edges e ) override { return book( createPath( par ), rel, title, e ); }
  virtual H1D* book( STRPAIR loc, CSTR title, Edges e ) { return book( loc.first, loc.second, title, e ); }
  H1D* book( CSTR full, CSTR title, Edges e ) override { return book( i_splitPath( full ), title, e ); }
  H1D* book( DataObject* pPar, CSTR rel, CSTR title, Edges e ) override
  {
    return i_book( pPar, rel, title, Gaudi::createH1D( title, e ) );
  }

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
  P1D* bookProf( CSTR full, CSTR title, Edges e ) override { return bookProf( i_splitPath( full ), title, e ); }
  P1D* bookProf( CSTR par, CSTR rel, CSTR title, Edges e ) override
  {
    return bookProf( createPath( par ), rel, title, e );
  }
  P1D* bookProf( CSTR par, int hID, CSTR title, Edges e ) override
  {
    return bookProf( par, std::to_string( hID ), title, e );
  }
  P1D* bookProf( DataObject* pPar, int hID, CSTR title, Edges e ) override
  {
    return bookProf( pPar, std::to_string( hID ), title, e );
  }
  virtual P1D* bookProf( STRPAIR loc, CSTR title, Edges e ) { return bookProf( loc.first, loc.second, title, e ); }
  P1D* bookProf( DataObject* pPar, CSTR rel, CSTR title, Edges e ) override
  {
    return i_book( pPar, rel, title, Gaudi::createProf1D( title, e, 0, 0 ) );
  }

  virtual P1D* bookProf( CSTR full, CSTR title, Edges e, double upper, double lower )
  {
    return bookProf( i_splitPath( full ), title, e, upper, lower );
  }
  virtual P1D* bookProf( CSTR par, CSTR rel, CSTR title, Edges e, double upper, double lower )
  {
    return bookProf( createPath( par ), rel, title, e, upper, lower );
  }
  virtual P1D* bookProf( CSTR par, int hID, CSTR title, Edges e, double upper, double lower )
  {
    return bookProf( par, std::to_string( hID ), title, e, upper, lower );
  }
  virtual P1D* bookProf( DataObject* pPar, int hID, CSTR title, Edges e, double upper, double lower )
  {
    return bookProf( pPar, std::to_string( hID ), title, e, upper, lower );
  }
  virtual P1D* bookProf( STRPAIR loc, CSTR title, Edges e, double upper, double lower )
  {
    return bookProf( loc.first, loc.second, title, e, upper, lower );
  }
  virtual P1D* bookProf( DataObject* pPar, CSTR rel, CSTR title, Edges e, double upper, double lower )
  {
    return i_book( pPar, rel, title, Gaudi::createProf1D( title, e, upper, lower ) );
  }
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
  H2D* book( CSTR full, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return book( i_splitPath( full ), title, BINS( x ), BINS( y ) );
  }
  H2D* book( CSTR par, CSTR rel, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return book( createPath( par ), rel, title, BINS( x ), BINS( y ) );
  }
  H2D* book( CSTR par, int hID, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return book( par, std::to_string( hID ), title, BINS( x ), BINS( y ) );
  }
  virtual H2D* book( STRPAIR loc, CSTR title, DBINS( x ), DBINS( y ) )
  {
    return book( loc.first, loc.second, title, BINS( x ), BINS( y ) );
  }
  H2D* book( DataObject* pPar, int hID, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return book( pPar, std::to_string( hID ), title, BINS( x ), BINS( y ) );
  }
  H2D* book( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return i_book( pPar, rel, title, Gaudi::createH2D( title, BINS( x ), BINS( y ) ) );
  }
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
  virtual P2D* bookProf( CSTR full, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( i_splitPath( full ), title, BINS( x ), BINS( y ), upper, lower );
  }
  virtual P2D* bookProf( CSTR par, CSTR rel, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( createPath( par ), rel, title, BINS( x ), BINS( y ), upper, lower );
  }
  virtual P2D* bookProf( STRPAIR loc, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( loc.first, loc.second, title, BINS( x ), BINS( y ), upper, lower );
  }
  virtual P2D* bookProf( CSTR par, int hID, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( par, std::to_string( hID ), title, BINS( x ), BINS( y ), upper, lower );
  }
  virtual P2D* bookProf( DataObject* pPar, int hID, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( pPar, std::to_string( hID ), title, BINS( x ), BINS( y ), upper, lower );
  }
  virtual P2D* bookProf( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return i_book( pPar, rel, title, Gaudi::createProf2D( title, BINS( x ), BINS( y ), upper, lower ) );
  }

  P2D* bookProf( CSTR full, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return bookProf( i_splitPath( full ), title, BINS( x ), BINS( y ) );
  }
  P2D* bookProf( CSTR par, CSTR rel, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return bookProf( createPath( par ), rel, title, BINS( x ), BINS( y ) );
  }
  virtual P2D* bookProf( STRPAIR loc, CSTR title, DBINS( x ), DBINS( y ) )
  {
    return bookProf( loc.first, loc.second, title, BINS( x ), BINS( y ) );
  }
  P2D* bookProf( CSTR par, int hID, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return bookProf( par, std::to_string( hID ), title, BINS( x ), BINS( y ) );
  }
  P2D* bookProf( DataObject* pPar, int hID, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return bookProf( pPar, std::to_string( hID ), title, BINS( x ), BINS( y ) );
  }
  P2D* bookProf( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), DBINS( y ) ) override
  {
    return i_book( pPar, rel, title, Gaudi::createProf2D( title, BINS( x ), BINS( y ), 0, 0 ) );
  }
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
  H2D* book( CSTR full, CSTR title, Edges x, Edges y ) override { return book( i_splitPath( full ), title, x, y ); }
  H2D* book( CSTR par, CSTR rel, CSTR title, Edges x, Edges y ) override
  {
    return book( createPath( par ), rel, title, x, y );
  }
  H2D* book( CSTR par, int hID, CSTR title, Edges x, Edges y ) override
  {
    return book( par, std::to_string( hID ), title, x, y );
  }
  virtual H2D* book( STRPAIR loc, CSTR title, Edges x, Edges y ) { return book( loc.first, loc.second, title, x, y ); }
  H2D* book( DataObject* pPar, int hID, CSTR title, Edges x, Edges y ) override
  {
    return book( pPar, std::to_string( hID ), title, x, y );
  }
  H2D* book( DataObject* pPar, CSTR rel, CSTR title, Edges x, Edges y ) override
  {
    return i_book( pPar, rel, title, Gaudi::createH2D( title, x, y ) );
  }
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
  P2D* bookProf( CSTR full, CSTR title, Edges x, Edges y ) override
  {
    return bookProf( i_splitPath( full ), title, x, y );
  }
  P2D* bookProf( CSTR par, CSTR rel, CSTR title, Edges x, Edges y ) override
  {
    return bookProf( createPath( par ), rel, title, x, y );
  }
  P2D* bookProf( CSTR par, int hID, CSTR title, Edges x, Edges y ) override
  {
    return bookProf( par, std::to_string( hID ), title, x, y );
  }
  P2D* bookProf( DataObject* pPar, int hID, CSTR title, Edges x, Edges y ) override
  {
    return bookProf( pPar, std::to_string( hID ), title, x, y );
  }
  virtual P2D* bookProf( STRPAIR loc, CSTR title, Edges x, Edges y )
  {
    return bookProf( loc.first, loc.second, title, x, y );
  }
  P2D* bookProf( DataObject* pPar, CSTR rel, CSTR title, Edges x, Edges y ) override
  {
    return i_book( pPar, rel, title, Gaudi::createProf2D( title, x, y, 0, 0 ) );
  }

  virtual P2D* bookProf( CSTR full, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return bookProf( i_splitPath( full ), title, x, y, upper, lower );
  }
  virtual P2D* bookProf( CSTR par, CSTR rel, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return bookProf( createPath( par ), rel, title, x, y, upper, lower );
  }
  virtual P2D* bookProf( CSTR par, int hID, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return bookProf( par, std::to_string( hID ), title, x, y, upper, lower );
  }
  virtual P2D* bookProf( DataObject* pPar, int hID, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return bookProf( pPar, std::to_string( hID ), title, x, y, upper, lower );
  }
  virtual P2D* bookProf( STRPAIR loc, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return bookProf( loc.first, loc.second, title, x, y, upper, lower );
  }
  virtual P2D* bookProf( DataObject* pPar, CSTR rel, CSTR title, Edges x, Edges y, double upper, double lower )
  {
    return i_book( pPar, rel, title, Gaudi::createProf2D( title, x, y, upper, lower ) );
  }
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
      @param DBINS(x/y/z) Macro for bin definition in X/Y/Z. Expands to:
                         int nx:    Number of bins on the axis X/Y/Z
                         double lx: Lower histogram edge on the axis X/Y/Z
                         double ux: Upper histogram edge on the axis X/Y/Z
  */
  H3D* book( CSTR full, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return book( i_splitPath( full ), title, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* book( CSTR par, CSTR rel, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return book( createPath( par ), rel, title, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* book( CSTR par, int hID, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return book( par, std::to_string( hID ), title, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* book( DataObject* pPar, int hID, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return book( pPar, std::to_string( hID ), title, BINS( x ), BINS( y ), BINS( z ) );
  }
  virtual H3D* book( STRPAIR loc, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) )
  {
    return book( loc.first, loc.second, title, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* book( DataObject* pPar, CSTR rel, CSTR title, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return i_book( pPar, rel, title, Gaudi::createH3D( title, BINS( x ), BINS( y ), BINS( z ) ) );
  }
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
  H3D* book( CSTR full, CSTR title, Edges x, Edges y, Edges z ) override
  {
    return book( i_splitPath( full ), title, x, y, z );
  }
  H3D* book( CSTR par, CSTR rel, CSTR title, Edges x, Edges y, Edges z ) override
  {
    return book( createPath( par ), rel, title, x, y, z );
  }
  H3D* book( CSTR par, int hID, CSTR title, Edges x, Edges y, Edges z ) override
  {
    return book( par, std::to_string( hID ), title, x, y, z );
  }
  H3D* book( DataObject* pPar, int hID, CSTR title, Edges x, Edges y, Edges z ) override
  {
    return book( pPar, std::to_string( hID ), title, x, y, z );
  }
  virtual H3D* book( STRPAIR loc, CSTR title, Edges x, Edges y, Edges z )
  {
    return book( loc.first, loc.second, title, x, y, z );
  }
  H3D* book( DataObject* pPar, CSTR rel, CSTR title, Edges x, Edges y, Edges z ) override
  {
    return i_book( pPar, rel, title, Gaudi::createH3D( title, x, y, z ) );
  }

  // ==========================================================================
  // Register histogram with the data store
  // ==========================================================================
  //------------------------------------------------------------------------------
  // Register object with the data store
  //------------------------------------------------------------------------------
  StatusCode registerObject( CSTR parent, CSTR rel, Base* obj ) override
  {
    return registerObject( createPath( parent ), rel, obj );
  }
  StatusCode registerObject( CSTR parent, int item, Base* obj ) override
  {
    return registerObject( parent, std::to_string( item ), obj );
  }
  StatusCode registerObject( Base* pPar, CSTR rel, Base* obj ) override
  {
    return registerObject( __cast( pPar ), rel, obj );
  }
  StatusCode registerObject( DataObject* pPar, int item, Base* obj ) override
  {
    return registerObject( pPar, std::to_string( item ), obj );
  }
  StatusCode registerObject( Base* pPar, int item, Base* obj ) override
  {
    return registerObject( __cast( pPar ), item, obj );
  }
  StatusCode registerObject( CSTR full, Base* obj ) override;
  StatusCode registerObject( DataObject* pPar, CSTR rel, Base* obj ) override;
  // ==========================================================================
  // Unregister histogram from the data store
  // ==========================================================================
  StatusCode unregisterObject( Base* obj ) override { return DataSvc::unregisterObject( __cast( obj ) ); }
  StatusCode unregisterObject( Base* obj, CSTR objectPath ) override
  {
    return DataSvc::unregisterObject( __cast( obj ), objectPath );
  }
  StatusCode unregisterObject( Base* obj, int item ) override
  {
    return DataSvc::unregisterObject( __cast( obj ), item );
  }
  // ==========================================================================
  // Retrieve histogram from data store
  // ==========================================================================
  StatusCode retrieveObject( IRegistry* pReg, CSTR path, H1D*& obj ) override
  {
    return Helper( this ).retrieve( pReg, path, obj );
  }
  StatusCode retrieveObject( IRegistry* pReg, CSTR path, P1D*& obj ) override
  {
    return Helper( this ).retrieve( pReg, path, obj );
  }
  StatusCode retrieveObject( IRegistry* pReg, CSTR path, H2D*& obj ) override
  {
    return Helper( this ).retrieve( pReg, path, obj );
  }
  StatusCode retrieveObject( IRegistry* pReg, CSTR path, P2D*& obj ) override
  {
    return Helper( this ).retrieve( pReg, path, obj );
  }
  StatusCode retrieveObject( IRegistry* pReg, CSTR path, H3D*& obj ) override
  {
    return Helper( this ).retrieve( pReg, path, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( CSTR full, P1D*& obj ) override { return Helper( this ).retrieve( full, obj ); }
  StatusCode retrieveObject( CSTR full, P2D*& obj ) override { return Helper( this ).retrieve( full, obj ); }
  StatusCode retrieveObject( CSTR full, H1D*& obj ) override { return Helper( this ).retrieve( full, obj ); }
  StatusCode retrieveObject( CSTR full, H2D*& obj ) override { return Helper( this ).retrieve( full, obj ); }
  StatusCode retrieveObject( CSTR full, H3D*& obj ) override { return Helper( this ).retrieve( full, obj ); }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( CSTR parent, CSTR rel, P1D*& obj ) override
  {
    return Helper( this ).retrieve( parent, rel, obj );
  }
  StatusCode retrieveObject( CSTR parent, CSTR rel, P2D*& obj ) override
  {
    return Helper( this ).retrieve( parent, rel, obj );
  }
  StatusCode retrieveObject( CSTR parent, CSTR rel, H1D*& obj ) override
  {
    return Helper( this ).retrieve( parent, rel, obj );
  }
  StatusCode retrieveObject( CSTR parent, CSTR rel, H2D*& obj ) override
  {
    return Helper( this ).retrieve( parent, rel, obj );
  }
  StatusCode retrieveObject( CSTR parent, CSTR rel, H3D*& obj ) override
  {
    return Helper( this ).retrieve( parent, rel, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( CSTR parent, int item, P1D*& obj ) override
  {
    return Helper( this ).retrieve( parent, item, obj );
  }
  StatusCode retrieveObject( CSTR parent, int item, P2D*& obj ) override
  {
    return Helper( this ).retrieve( parent, item, obj );
  }
  StatusCode retrieveObject( CSTR parent, int item, H1D*& obj ) override
  {
    return Helper( this ).retrieve( parent, item, obj );
  }
  StatusCode retrieveObject( CSTR parent, int item, H2D*& obj ) override
  {
    return Helper( this ).retrieve( parent, item, obj );
  }
  StatusCode retrieveObject( CSTR parent, int item, H3D*& obj ) override
  {
    return Helper( this ).retrieve( parent, item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( DataObject* par, CSTR item, P1D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, CSTR item, P2D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, CSTR item, H1D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, CSTR item, H2D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, CSTR item, H3D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( DataObject* par, int item, P1D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, int item, P2D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, int item, H1D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, int item, H2D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  StatusCode retrieveObject( DataObject* par, int item, H3D*& obj ) override
  {
    return Helper( this ).retrieve( par, item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( Base* par, int item, P1D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, int item, P2D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, int item, H1D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, int item, H2D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, int item, H3D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode retrieveObject( Base* par, CSTR item, P1D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, CSTR item, P2D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, CSTR item, H1D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, CSTR item, H2D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }
  StatusCode retrieveObject( Base* par, CSTR item, H3D*& obj ) override
  {
    return Helper( this ).retrieve( __cast( par ), item, obj );
  }

  // ==========================================================================
  // Find histogram identified by its full path in the data store
  // ==========================================================================

  //------------------------------------------------------------------------------
  // Find object identified by its full path in the data store
  //------------------------------------------------------------------------------
  StatusCode findObject( IRegistry* pReg, CSTR path, P1D*& obj ) override
  {
    return Helper( this ).find( pReg, path, obj );
  }
  StatusCode findObject( IRegistry* pReg, CSTR path, P2D*& obj ) override
  {
    return Helper( this ).find( pReg, path, obj );
  }
  StatusCode findObject( IRegistry* pReg, CSTR path, H1D*& obj ) override
  {
    return Helper( this ).find( pReg, path, obj );
  }
  StatusCode findObject( IRegistry* pReg, CSTR path, H2D*& obj ) override
  {
    return Helper( this ).find( pReg, path, obj );
  }
  StatusCode findObject( IRegistry* pReg, CSTR path, H3D*& obj ) override
  {
    return Helper( this ).find( pReg, path, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode findObject( CSTR full, P1D*& obj ) override { return Helper( this ).find( full, obj ); }
  StatusCode findObject( CSTR full, P2D*& obj ) override { return Helper( this ).find( full, obj ); }
  StatusCode findObject( CSTR full, H1D*& obj ) override { return Helper( this ).find( full, obj ); }
  StatusCode findObject( CSTR full, H2D*& obj ) override { return Helper( this ).find( full, obj ); }
  StatusCode findObject( CSTR full, H3D*& obj ) override { return Helper( this ).find( full, obj ); }
  //------------------------------------------------------------------------------
  StatusCode findObject( CSTR par, CSTR rel, P1D*& obj ) override { return Helper( this ).find( par, rel, obj ); }
  StatusCode findObject( CSTR par, CSTR rel, P2D*& obj ) override { return Helper( this ).find( par, rel, obj ); }
  StatusCode findObject( CSTR par, CSTR rel, H1D*& obj ) override { return Helper( this ).find( par, rel, obj ); }
  StatusCode findObject( CSTR par, CSTR rel, H2D*& obj ) override { return Helper( this ).find( par, rel, obj ); }
  StatusCode findObject( CSTR par, CSTR rel, H3D*& obj ) override { return Helper( this ).find( par, rel, obj ); }
  //------------------------------------------------------------------------------
  StatusCode findObject( CSTR par, int item, P1D*& obj ) override { return Helper( this ).find( par, item, obj ); }
  StatusCode findObject( CSTR par, int item, P2D*& obj ) override { return Helper( this ).find( par, item, obj ); }
  StatusCode findObject( CSTR par, int item, H1D*& obj ) override { return Helper( this ).find( par, item, obj ); }
  StatusCode findObject( CSTR par, int item, H2D*& obj ) override { return Helper( this ).find( par, item, obj ); }
  StatusCode findObject( CSTR par, int item, H3D*& obj ) override { return Helper( this ).find( par, item, obj ); }
  //------------------------------------------------------------------------------
  StatusCode findObject( DataObject* par, int item, P1D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, int item, P2D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, int item, H1D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, int item, H2D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, int item, H3D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode findObject( DataObject* par, CSTR item, P1D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, CSTR item, P2D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, CSTR item, H1D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, CSTR item, H2D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  StatusCode findObject( DataObject* par, CSTR item, H3D*& obj ) override
  {
    return Helper( this ).find( par, item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode findObject( Base* par, int item, P1D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, int item, P2D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, int item, H1D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, int item, H2D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, int item, H3D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  //------------------------------------------------------------------------------
  StatusCode findObject( Base* par, CSTR item, P1D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, CSTR item, P2D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, CSTR item, H1D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, CSTR item, H2D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }
  StatusCode findObject( Base* par, CSTR item, H3D*& obj ) override
  {
    return Helper( this ).find( __cast( par ), item, obj );
  }

  // ==========================================================================
  // Projections and slices.
  // ==========================================================================
  H1D* projectionX( CSTR name, const H2D& h ) override
  {
    return sliceX( name, h, IAxis::UNDERFLOW_BIN, IAxis::OVERFLOW_BIN );
  }
  H1D* projectionY( CSTR name, const H2D& h ) override
  {
    return sliceY( name, h, IAxis::UNDERFLOW_BIN, IAxis::OVERFLOW_BIN );
  }
  H1D* sliceX( CSTR name, const H2D& h, int indexY ) override { return sliceX( name, h, indexY, indexY ); }
  H1D* sliceY( CSTR name, const H2D& h, int indexX ) override { return sliceY( name, h, indexX, indexX ); }
  H1D* sliceX( CSTR name, const H2D& h, int indexY1, int indexY2 ) override;
  H1D* sliceY( CSTR name, const H2D& h, int indexX1, int indexX2 ) override;
  //------------------------------------------------------------------------------
  bool destroy( IBaseHistogram* hist ) override;

  H1D* add( CSTR nameAndTitle, const H1D& a, const H1D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Add, 1. );
  }
  H1D* subtract( CSTR nameAndTitle, const H1D& a, const H1D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Add, -1. );
  }
  H1D* multiply( CSTR nameAndTitle, const H1D& a, const H1D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Multiply );
  }
  H1D* divide( CSTR nameAndTitle, const H1D& a, const H1D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH1::Divide );
  }

  H2D* add( CSTR nameAndTitle, const H2D& a, const H2D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Add, 1. );
  }
  H2D* subtract( CSTR nameAndTitle, const H2D& a, const H2D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Add, -1. );
  }
  H2D* multiply( CSTR nameAndTitle, const H2D& a, const H2D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Multiply );
  }
  H2D* divide( CSTR nameAndTitle, const H2D& a, const H2D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH2D::Divide );
  }

  H3D* add( CSTR nameAndTitle, const H3D& a, const H3D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Add, 1. );
  }
  H3D* subtract( CSTR nameAndTitle, const H3D& a, const H3D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Add, -1. );
  }
  H3D* multiply( CSTR nameAndTitle, const H3D& a, const H3D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Multiply );
  }
  H3D* divide( CSTR nameAndTitle, const H3D& a, const H3D& b ) override
  {
    return Helper::act( createCopy( nameAndTitle, a ), b, &TH3D::Divide );
  }

  H2D* projectionXY( CSTR nameAndTitle, const H3D& h ) override { return i_project( nameAndTitle, h, "xy" ); }
  H2D* projectionXZ( CSTR nameAndTitle, const H3D& h ) override { return i_project( nameAndTitle, h, "xz" ); }
  H2D* projectionYZ( CSTR nameAndTitle, const H3D& h ) override { return i_project( nameAndTitle, h, "yz" ); }
  H2D* sliceXY( CSTR /* nameAndTitle */, const H3D& /* h */, int /* low */,
                int /* high */ ) override NOT_IMPLEMENTED H2D* sliceXZ( CSTR /* nameAndTitle */, const H3D& /* h */,
                                                                        int /* low */, int /* high */ ) override
      NOT_IMPLEMENTED H2D* sliceYZ( CSTR /* nameAndTitle */, const H3D& /* h */, int /* low */,
                                    int /* high */ ) override NOT_IMPLEMENTED

      //------------------------------------------------------------------------------
      H1D* createHistogram1D( CSTR name, CSTR tit, DBINS( x ) )
  {
    return book( name, tit, BINS( x ) );
  }
  H1D* createHistogram1D( CSTR name, CSTR tit, DBINS( x ), CSTR /*opt*/ ) override
  {
    return book( name, tit, BINS( x ) );
  }
  H1D* createHistogram1D( CSTR name, CSTR title, const Edges& x, CSTR /*opt*/ ) override
  {
    return book( name, title, x );
  }
  H1D* createHistogram1D( CSTR nameAndTitle, DBINS( x ) ) override
  {
    return book( nameAndTitle, nameAndTitle, BINS( x ) );
  }
  H1D* createCopy( CSTR full, const H1D& h ) override { return createCopy( i_splitPath( full ), h ); }
  H1D* createCopy( CSTR par, CSTR rel, const H1D& h ) { return createCopy( createPath( par ), rel, h ); }
  H1D* createCopy( STRPAIR loc, const H1D& h ) { return createCopy( loc.first, loc.second, h ); }
  H1D* createCopy( DataObject* pPar, CSTR rel, const H1D& h )
  {
    return i_book( pPar, rel, h.title(), Gaudi::createH1D( h ) );
  }
  //------------------------------------------------------------------------------
  H2D* createHistogram2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ) )
  {
    return book( name, tit, BINS( x ), BINS( y ) );
  }
  H2D* createHistogram2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), CSTR /*opt*/ ) override
  {
    return book( name, tit, BINS( x ), BINS( y ) );
  }
  H2D* createHistogram2D( CSTR name, CSTR title, const Edges& x, const Edges& y, CSTR /*opt*/ ) override
  {
    return book( name, title, x, y );
  }
  H2D* createHistogram2D( CSTR nameAndTitle, DBINS( x ), DBINS( y ) ) override
  {
    return book( nameAndTitle, nameAndTitle, BINS( x ), BINS( y ) );
  }
  H2D* createCopy( CSTR full, const H2D& h ) override { return createCopy( i_splitPath( full ), h ); }
  H2D* createCopy( CSTR par, CSTR rel, const H2D& h ) { return createCopy( createPath( par ), rel, h ); }
  H2D* createCopy( STRPAIR loc, const H2D& h ) { return createCopy( loc.first, loc.second, h ); }
  H2D* createCopy( DataObject* pPar, CSTR rel, const H2D& h )
  {
    return i_book( pPar, rel, h.title(), Gaudi::createH2D( h ) );
  }
  //------------------------------------------------------------------------------
  H3D* createHistogram3D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), DBINS( z ) )
  {
    return book( name, tit, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* createHistogram3D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), DBINS( z ), CSTR /*opt*/ ) override
  {
    return book( name, tit, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* createHistogram3D( CSTR name, CSTR title, const Edges& x, const Edges& y, const Edges& z, CSTR /*opt*/ ) override
  {
    return book( name, title, x, y, z );
  }
  H3D* createHistogram3D( CSTR nameAndTitle, DBINS( x ), DBINS( y ), DBINS( z ) ) override
  {
    return book( nameAndTitle, nameAndTitle, BINS( x ), BINS( y ), BINS( z ) );
  }
  H3D* createCopy( CSTR full, const H3D& h ) override { return createCopy( i_splitPath( full ), h ); }
  H3D* createCopy( CSTR par, CSTR rel, const H3D& h ) { return createCopy( createPath( par ), rel, h ); }
  H3D* createCopy( STRPAIR loc, const H3D& h ) { return createCopy( loc.first, loc.second, h ); }
  H3D* createCopy( DataObject* pPar, CSTR rel, const H3D& h )
  {
    return i_book( pPar, rel, h.title(), Gaudi::createH3D( h ) );
  }
  //------------------------------------------------------------------------------

  P1D* createProfile1D( CSTR name, CSTR tit, DBINS( x ), CSTR opt ) override
  {
    return bookProf( name, tit, BINS( x ), opt );
  }
  P1D* createProfile1D( CSTR name, CSTR tit, DBINS( x ), double upper, double lower, CSTR opt ) override
  {
    return bookProf( name, tit, BINS( x ), upper, lower, opt );
  }
  P1D* createProfile1D( CSTR name, CSTR title, const Edges& x, CSTR /* opt */ ) override
  {
    return bookProf( name, title, x );
  }
  P1D* createProfile1D( CSTR name, CSTR title, const Edges& x, double upper, double lower, CSTR /* opt */ ) override
  {
    return bookProf( name, title, x, upper, lower );
  }

  P1D* createProfile1D( CSTR nametit, DBINS( x ) ) override { return bookProf( nametit, nametit, BINS( x ), "s" ); }
  P1D* createProfile1D( CSTR nametit, DBINS( x ), double upper, double lower ) override
  {
    return bookProf( nametit, nametit, BINS( x ), upper, lower, "s" );
  }

  P1D* createCopy( CSTR full, const P1D& h ) override { return createCopy( i_splitPath( full ), h ); }
  P1D* createCopy( CSTR par, CSTR rel, const P1D& h ) { return createCopy( createPath( par ), rel, h ); }
  P1D* createCopy( STRPAIR loc, const P1D& h ) { return createCopy( loc.first, loc.second, h ); }
  P1D* createCopy( DataObject* pPar, CSTR rel, const P1D& h )
  {
    return i_book( pPar, rel, h.title(), Gaudi::createProf1D( h ) );
  }
  //------------------------------------------------------------------------------
  P2D* createProfile2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ) )
  {
    return bookProf( name, tit, BINS( x ), BINS( y ) );
  }
  P2D* createProfile2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), CSTR /*opt*/ ) override
  {
    return bookProf( name, tit, BINS( x ), BINS( y ) );
  }
  P2D* createProfile2D( CSTR name, CSTR title, const Edges& x, const Edges& y, CSTR /*opt*/ ) override
  {
    return bookProf( name, title, x, y );
  }
  P2D* createProfile2D( CSTR nameAndTitle, DBINS( x ), DBINS( y ) ) override
  {
    return bookProf( nameAndTitle, nameAndTitle, BINS( x ), BINS( y ) );
  }

  P2D* createProfile2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), double upper, double lower )
  {
    return bookProf( name, tit, BINS( x ), BINS( y ), upper, lower );
  }
  P2D* createProfile2D( CSTR name, CSTR tit, DBINS( x ), DBINS( y ), double upper, double lower, CSTR /*opt*/ ) override
  {
    return bookProf( name, tit, BINS( x ), BINS( y ), upper, lower );
  }
  P2D* createProfile2D( CSTR name, CSTR title, const Edges& x, const Edges& y, double upper, double lower,
                        CSTR /*opt*/ ) override
  {
    return bookProf( name, title, x, y, upper, lower );
  }
  P2D* createProfile2D( CSTR nameAndTitle, DBINS( x ), DBINS( y ), double upper, double lower ) override
  {
    return bookProf( nameAndTitle, nameAndTitle, BINS( x ), BINS( y ), upper, lower );
  }

  P2D* createCopy( CSTR full, const P2D& h ) override { return createCopy( i_splitPath( full ), h ); }
  P2D* createCopy( CSTR par, CSTR rel, const P2D& h ) { return createCopy( createPath( par ), rel, h ); }
  P2D* createCopy( STRPAIR loc, const P2D& h ) { return createCopy( loc.first, loc.second, h ); }
  P2D* createCopy( DataObject* pPar, CSTR rel, const P2D& h )
  {
    return i_book( pPar, rel, h.title(), Gaudi::createProf2D( h ) );
  }
  //------------------------------------------------------------------------------
  AIDA::ICloud1D* createCloud1D( CSTR, CSTR, int, CSTR ) override NOT_IMPLEMENTED AIDA::ICloud1D* createCloud1D(
      CSTR ) override NOT_IMPLEMENTED AIDA::ICloud1D* createCopy( CSTR, const AIDA::ICloud1D& ) override
      NOT_IMPLEMENTED AIDA::ICloud2D* createCloud2D( CSTR, CSTR, int, CSTR ) override
      NOT_IMPLEMENTED AIDA::ICloud2D* createCloud2D( CSTR ) override NOT_IMPLEMENTED AIDA::ICloud2D* createCopy(
          CSTR, const AIDA::ICloud2D& ) override NOT_IMPLEMENTED AIDA::ICloud3D* createCloud3D( CSTR, CSTR, int, CSTR )
          override NOT_IMPLEMENTED AIDA::ICloud3D* createCloud3D( CSTR ) override
      NOT_IMPLEMENTED AIDA::ICloud3D* createCopy( CSTR, const AIDA::ICloud3D& ) override NOT_IMPLEMENTED

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
  DataObject* createPath( CSTR newPath ) override;

  /** Create a sub-directory in a directory.
   *  @param parentDir name of the parent directory
   *  @param subDir to identify the histogram object in the store
   */
  DataObject* createDirectory( CSTR parentDir, CSTR subDir ) override;

public:
  // ==========================================================================
  /// handler to be invoked for updating property m_defs1D
  void update1Ddefs( Gaudi::Details::PropertyBase& );
  // ==========================================================================
  typedef std::map<std::string, Gaudi::Histo1DDef> Histo1DMap;
  // ==========================================================================
private:
  Gaudi::Property<DBaseEntries> m_input{this, "Input", {}, "input streams"};
  Gaudi::Property<Histo1DMap> m_defs1D{this, "Predefined1DHistos", {}, "histograms with predefined parameters"};

  // ==========================================================================
  // modified histograms:
  std::set<std::string> m_mods1D;
  // ==========================================================================
};
// ===========================================================================
// The END
// ===========================================================================
#endif // GAUDISVC_HISTOGRAMSVC_H
// ===========================================================================
