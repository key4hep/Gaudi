#ifndef GAUDISVC_GENERIC2D_H
#define GAUDISVC_GENERIC2D_H 1
/// @FIXME: AIDA interfaces visibility
#include "AIDA_visibility_hack.h"

#include <stdexcept>
#include "AIDA/IProfile2D.h"
#include "GaudiKernel/HistogramBase.h"
#include "Annotation.h"
#include "Axis.h"
#include "TFile.h"


/*
 *    Gaudi namespace
 */
namespace Gaudi {

  /** @class Generic2D Generic2D.h GaudiPI/Generic2D.h
    *
    * Common AIDA implementation stuff for histograms and profiles
    * using ROOT implementations
    *
    * Credits: This code is the result of some stripdown implementation
    * of LCG/PI. Credits to them!
    *
    *  @author M.Frank
    */
  template <class INTERFACE, class IMPLEMENTATION>
  class GAUDI_API Generic2D : virtual public INTERFACE, virtual public HistogramBase  {
  public:
    typedef Generic2D<INTERFACE,IMPLEMENTATION> Base;

    Generic2D() : m_rep(0) {}

    /// Destructor.
    virtual ~Generic2D()   {  delete m_rep;    }

    /// ROOT object implementation
    TObject* representation() const                      { return m_rep;                       }
    /// Adopt ROOT histogram representation
    virtual void adoptRepresentation(TObject* rep);
    /// Get the title of the object
    virtual std::string title() const                    {  return m_annotation.value( "Title" );    }
    /// Set the title of the object
    virtual bool setTitle(const std::string & title);
    /// object name
    std::string name() const                             { return m_annotation.value("Name"); }
    /// Set the name of the object
    bool setName( const std::string& newName );
    /// Access annotation object
    virtual AIDA::IAnnotation & annotation()             { return m_annotation;           }
    /// Access annotation object (cons)
    virtual const AIDA::IAnnotation & annotation() const { return m_annotation;           }

    /// Return the X axis.
    virtual const AIDA::IAxis & xAxis() const            { return m_xAxis;                }
    /// Return the Y axis.
    virtual const AIDA::IAxis & yAxis() const            { return m_yAxis;                }
    /// operator methods
    virtual int rIndexX(int index) const                 { return m_xAxis.rIndex(index);  }
    /// operator methods
    virtual int rIndexY(int index) const                 { return m_yAxis.rIndex(index);  }

    /// Get the number or all the entries
    virtual int entries() const;
    /// Get the number or all the entries, both in range and underflow/overflow bins of the IProfile.
    virtual int allEntries() const;
    /// Get the number of entries in the underflow and overflow bins.
    virtual int extraEntries() const;
    /// Get the sum of in range bin heights in the IProfile.
    virtual double sumBinHeights() const;
    /// Get the sum of all the bins heights (including underflow and overflow bin).
    virtual double sumAllBinHeights() const;
    /// Get the sum of the underflow and overflow bin height.
    virtual double  sumExtraBinHeights (  ) const  { return  sumAllBinHeights()-sumBinHeights(); }
    /// Get the minimum height of the in-range bins.
    virtual double minBinHeight() const;
    /// Get the maximum height of the in-range bins.
    virtual double maxBinHeight() const;

    /// The weighted mean along x of a given bin.
    virtual double binMeanX(int indexX,int indexY) const;
    /// The weighted mean along y of a given bin.
    virtual double binMeanY(int indexX,int indexY) const;
    /// The number of entries (ie the number of times fill was called for this bin).
    virtual int  binEntries ( int indexX,int indexY ) const;
    /// Equivalent to <tt>projectionX().binEntries(indexX)</tt>.
    virtual int binEntriesX(int indexX) const;
    /// Equivalent to <tt>projectionY().binEntries(indexY)</tt>.
    virtual int binEntriesY(int indexY) const;
    /// Total height of the corresponding bin (ie the sum of the weights in this bin).
    virtual double binHeight(int indexX,int indexY) const;
    /// Equivalent to <tt>projectionX().binHeight(indexX)</tt>.
    virtual double binHeightX(int indexX) const;
    /// Equivalent to <tt>projectionY().binHeight(indexY)</tt>.
    virtual double binHeightY(int indexY) const;
    /// The error on this bin.
    virtual double binError(int indexX,int indexY) const;
    /// The spread (RMS) of this bin.
    virtual double binRms(int indexX,int indexY) const;
    /// Returns the mean of the profile, as calculated on filling-time projected on the X axis.
    virtual double meanX() const;
    /// Returns the mean of the profile, as calculated on filling-time projected on the Y axis.
    virtual double meanY() const;
    /// Returns the rms of the profile as calculated on filling-time projected on the X axis.
    virtual double rmsX() const;
    /// Returns the rms of the profile as calculated on filling-time projected on the Y axis.
    virtual double rmsY() const;
    /// Convenience method, equivalent to <tt>xAxis().coordToIndex(coord)</tt>.
    virtual int coordToIndexX(double coordX) const;
    /// Convenience method, equivalent to <tt>yAxis().coordToIndex(coord)</tt>.
    virtual int coordToIndexY(double coordY) const;
    /// Number of equivalent entries, i.e. <tt>SUM[ weight ] ^ 2 / SUM[ weight^2 ]</tt>
    virtual double equivalentBinEntries (  ) const;
    /// Scale the weights and the errors of all the IHistogram's bins (in-range and out-of-range ones) by a given scale factor.
    virtual bool scale( double scaleFactor );
    /// Modifies this profile by adding the contents of profile to it.
    virtual bool add(const INTERFACE & h);
    // overwrite reset
    bool  reset (  );
    /// Introspection method
    void * cast(const std::string & className) const;
    /// The AIDA user-level unterface leaf class type
    const std::string& userLevelClassType() const { return m_classType; }
    /// Get the Histogram's dimension.
    virtual int  dimension() const  { return 2; }
    /// Print (ASCII) the histogram into the output stream
    virtual std::ostream& print( std::ostream& s ) const;
    /// Write (ASCII) the histogram table into the output stream
    virtual std::ostream& write( std::ostream& s ) const;
    /// Write (ASCII) the histogram table into a file
    virtual int write( const char* file_name ) const;

  protected:
    /// X axis member
    Axis                     m_xAxis;
    /// Y axis member
    Axis                     m_yAxis;
    /// Object annotations
    mutable AIDA::Annotation m_annotation;
    /// Reference to underlying implementation
    IMPLEMENTATION*          m_rep;
    /// class type
    std::string              m_classType;
    /// cache sumEntries (allEntries)   when setting contents since Root can't compute by himself
    int                      m_sumEntries;
  };

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic2D<INTERFACE,IMPLEMENTATION>::setTitle(const std::string & title)  {
    m_rep->SetTitle(title.c_str());
    if ( !annotation().addItem( "Title", title ) )
      m_annotation.setValue( "Title" , title );
    if ( !annotation().addItem( "title", title ) )
      annotation().setValue( "title", title );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic2D<INTERFACE,IMPLEMENTATION>::setName( const std::string& newName ) {
    m_rep->SetName(newName.c_str());
    m_annotation.setValue( "Name", newName );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::entries() const                       {
    return (int)m_rep->GetEntries();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::allEntries (  ) const  {
    return int(m_rep->GetEntries());
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double  Generic2D<INTERFACE,IMPLEMENTATION>::minBinHeight() const  {
    return m_rep->GetMinimum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double  Generic2D<INTERFACE,IMPLEMENTATION>::maxBinHeight() const  {
    return m_rep->GetMaximum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::sumBinHeights () const  {
    return m_rep->GetSumOfWeights();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::sumAllBinHeights () const  {
    return m_rep->GetSum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binRms(int indexX,int indexY) const {
    return m_rep->GetBinError ( rIndexX(indexX), rIndexY(indexY) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binMeanX(int indexX,int ) const {
    return (m_rep->GetXaxis())->GetBinCenter( rIndexX(indexX) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binMeanY(int,int indexY) const  {
    return (m_rep->GetYaxis())->GetBinCenter( rIndexY(indexY) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::binEntriesX(int index) const   {
    int n = 0;
    for (int iY = -2; iY < yAxis().bins(); ++iY)
      n += binEntries(index,iY);
    return n;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::binEntriesY(int index) const    {
    int n = 0;
    for (int iX = -2; iX < xAxis().bins(); ++iX)
      n += binEntries(iX,index);
    return n;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binHeight ( int indexX,int indexY ) const  {
    return m_rep->GetBinContent ( rIndexX(indexX), rIndexY(indexY) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binHeightX(int index) const  {
    double s = 0;
    for (int iY = -2; iY < yAxis().bins(); ++iY) {
      s += binHeight(index,iY);
    }
    return s;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binHeightY(int index) const  {
    double s = 0;
    for (int iX = -2; iX < xAxis().bins(); ++iX)
      s += binHeight(iX,index);
    return s;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::binError(int indexX,int indexY) const  {
    return m_rep->GetBinError ( rIndexX(indexX), rIndexY(indexY ) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::meanX() const  {
    return m_rep->GetMean(1);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::meanY() const  {
    return m_rep->GetMean(2);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::rmsX() const  {
    return m_rep->GetRMS(1);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::rmsY() const  {
    return m_rep->GetRMS(2);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::coordToIndexX ( double coord ) const {
    return xAxis().coordToIndex(coord);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::coordToIndexY ( double coord ) const {
    return yAxis().coordToIndex(coord);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic2D<INTERFACE,IMPLEMENTATION>::add ( const INTERFACE & hist ) {
    const Base* p = dynamic_cast<const Base*>(&hist);
    if ( p )  {
      m_rep->Add(p->m_rep);
      return true;
    }
    throw std::runtime_error("Cannot add profile histograms of different implementations.");
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::extraEntries() const {
    return
      binEntries(AIDA::IAxis::UNDERFLOW_BIN,AIDA::IAxis::UNDERFLOW_BIN) +
      binEntries(AIDA::IAxis::UNDERFLOW_BIN,AIDA::IAxis::OVERFLOW_BIN)  +
      binEntries(AIDA::IAxis::OVERFLOW_BIN,AIDA::IAxis::UNDERFLOW_BIN)  +
      binEntries(AIDA::IAxis::OVERFLOW_BIN,AIDA::IAxis::OVERFLOW_BIN);
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic2D<INTERFACE,IMPLEMENTATION>::equivalentBinEntries() const  {
    if (sumBinHeights() <= 0) return 0;
    Stat_t stats[11];   // cover up to 3D...
    m_rep->GetStats(stats);
    return stats[0]*stats[0]/stats[1];
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic2D<INTERFACE,IMPLEMENTATION>::scale(double scaleFactor)   {
    m_rep->Scale ( scaleFactor );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic2D<INTERFACE,IMPLEMENTATION>::reset (  )   {
    m_sumEntries = 0;
    m_rep->Reset ( );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic2D<INTERFACE,IMPLEMENTATION>::print( std::ostream& s ) const
  {
    /// bin contents and errors are printed for all bins including under and overflows
    m_rep->Print("all");
    return s;
  }

  /// Write (ASCII) the histogram table into the output stream
  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic2D<INTERFACE,IMPLEMENTATION>::write( std::ostream& s ) const
  {
    s << std::endl << "2D Histogram Table: " << std::endl;
    s << "BinX, BinY, Height, Error " << std::endl;
    for ( int i = 0; i < xAxis().bins(); ++i )  {
      for ( int j = 0; j < yAxis().bins(); ++j )  {
        s << binMeanX( i, j )    << ", "
	        << binMeanY( i, j )    << ", "
	        << binHeight( i, j ) << ", "
	        << binError ( i, j ) << std::endl;
      }
    }
    s << std::endl;
    return s;
  }

  /// Write (ASCII) the histogram table into a file
  template <class INTERFACE, class IMPLEMENTATION>
  int Generic2D<INTERFACE,IMPLEMENTATION>::write( const char* file_name ) const
  {
    TFile *f = TFile::Open(file_name,"RECREATE");
    Int_t nbytes = m_rep->Write();
    f->Close();
    return nbytes;
  }
} // end namespace AIDA
#endif // GAUDIPI_GENERIC2D_H
