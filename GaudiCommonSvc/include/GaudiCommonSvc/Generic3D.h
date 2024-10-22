/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDISVC_GENERIC3D_H
#define GAUDISVC_GENERIC3D_H 1

#include "Annotation.h"
#include "Axis.h"
#include <AIDA/IHistogram3D.h>
#include <GaudiKernel/HistogramBase.h>
#include <TFile.h>
#include <memory>
#include <stdexcept>

// Hide warning message:
// warning: 'XYZ' overrides a member function but is not marked 'override'
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wsuggest-override"
#  pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined( __GNUC__ )
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

namespace Gaudi {

  /** @class Generic3D Generic3D.h GaudiPI/Generic3D.h
   *
   * Common AIDA implementation stuff for histograms and profiles
   * using ROOT implementations
   *
   * Credits: This code is the result of some stripdown implementation
   * of LCG/PI. Credits to them!
   *
   *  @author M.Frank
   */
  template <typename INTERFACE, typename IMPLEMENTATION>
  class GAUDI_API Generic3D : virtual public INTERFACE, virtual public HistogramBase {
  public:
    typedef Generic3D<INTERFACE, IMPLEMENTATION> Base;
    /// Default constructor
    Generic3D() = default;

  protected:
    /// constructor
    Generic3D( IMPLEMENTATION* p ) : m_rep( p ) {}

  public:
    /// ROOT object implementation
    TObject* representation() const override { return m_rep.get(); }
    /// Adopt ROOT histogram representation
    void adoptRepresentation( TObject* rep ) override;

    /// Get the Histogram's dimension.
    int dimension() const override { return 3; }
    /// Get the title of the object
    std::string title() const override { return m_annotation.value( "Title" ); }
    /// Set the title of the object
    bool setTitle( const std::string& title ) override;
    /// object name
    std::string name() const { return m_annotation.value( "Name" ); }
    /// Sets the name of the object
    bool setName( const std::string& newName );
    /// Access annotation object
    AIDA::IAnnotation& annotation() override { return m_annotation; }
    /// Access annotation object (cons)
    const AIDA::IAnnotation& annotation() const override { return m_annotation; }

    /// Get the number or all the entries
    int entries() const override;
    /// Get the number or all the entries, both in range and underflow/overflow bins of the IProfile.
    int allEntries() const override;
    /// Get the sum of in range bin heights in the IProfile.
    double sumBinHeights() const override;
    /// Get the sum of all the bins heights (including underflow and overflow bin).
    double sumAllBinHeights() const override;
    /// Get the sum of the underflow and overflow bin height.
    double sumExtraBinHeights() const override { return sumAllBinHeights() - sumBinHeights(); }
    /// Get the minimum height of the in-range bins.
    double minBinHeight() const override;
    /// Get the maximum height of the in-range bins.
    double maxBinHeight() const override;

    int rIndexX( int index ) const { return m_xAxis.rIndex( index ); }
    int rIndexY( int index ) const { return m_yAxis.rIndex( index ); }
    int rIndexZ( int index ) const { return m_zAxis.rIndex( index ); }

    /// The weighted mean along the x axis of a given bin.
    double binMeanX( int indexX, int, int ) const override {
      return m_rep->GetXaxis()->GetBinCenter( rIndexX( indexX ) );
    }
    /// The weighted mean along the y axis of a given bin.
    double binMeanY( int, int indexY, int ) const override {
      return m_rep->GetYaxis()->GetBinCenter( rIndexY( indexY ) );
    }
    /// The weighted mean along the z axis of a given bin.
    double binMeanZ( int, int, int indexZ ) const override {
      return m_rep->GetYaxis()->GetBinCenter( rIndexY( indexZ ) );
    }
    /// Number of entries in the corresponding bin (ie the number of times fill was calle d for this bin).
    int binEntries( int indexX, int indexY, int indexZ ) const override {
      if ( binHeight( indexX, indexY, indexZ ) <= 0 ) return 0;
      double xx = binHeight( indexX, indexY, indexZ ) / binError( indexX, indexY, indexZ );
      return int( xx * xx + 0.5 );
    }
    /// Sum of all the entries of the bins along a given x bin.
    int binEntriesX( int index ) const override {
      int n = 0;
      for ( int i = -2; i < yAxis().bins(); ++i )
        for ( int j = -2; j < zAxis().bins(); ++j ) n += binEntries( index, i, j );
      return n;
    }
    /// Sum of all the entries of the bins along a given y bin.
    int binEntriesY( int index ) const override {
      int n = 0;
      for ( int i = -2; i < xAxis().bins(); ++i )
        for ( int j = -2; j < zAxis().bins(); ++j ) n += binEntries( i, index, j );
      return n;
    }

    /// Sum of all the entries of the bins along a given z bin.
    int binEntriesZ( int index ) const override {
      int n = 0;
      for ( int i = -2; i < xAxis().bins(); ++i )
        for ( int j = -2; j < yAxis().bins(); ++j ) n += binEntries( i, j, index );
      return n;
    }

    /// Total height of the corresponding bin (ie the sum of the weights in this bin).
    double binHeight( int indexX, int indexY, int indexZ ) const {
      return m_rep->GetBinContent( rIndexX( indexX ), rIndexY( indexY ), rIndexZ( indexZ ) );
    }

    /// Sum of all the heights of the bins along a given x bin.
    double binHeightX( int index ) const override {
      double s = 0;
      for ( int i = -2; i < yAxis().bins(); ++i )
        for ( int j = -2; j < zAxis().bins(); ++j ) s += binHeight( index, i, j );
      return s;
    }
    /// Sum of all the heights of the bins along a given y bin.
    double binHeightY( int index ) const override {
      double s = 0;
      for ( int i = -2; i < xAxis().bins(); ++i )
        for ( int j = -2; j < zAxis().bins(); ++j ) s += binHeight( i, index, j );
      return s;
    }
    /// Sum of all the heights of the bins along a given z bin.
    double binHeightZ( int index ) const override {
      double s = 0;
      for ( int i = -2; i < xAxis().bins(); ++i )
        for ( int j = -2; j < yAxis().bins(); ++j ) s += binHeight( i, j, index );
      return s;
    }
    /// The error of a given bin.
    double binError( int indexX, int indexY, int indexZ ) const override {
      return m_rep->GetBinError( rIndexX( indexX ), rIndexY( indexY ), rIndexZ( indexZ ) );
    }
    /// The mean of the IHistogram3D along the x axis.
    double meanX() const override { return m_rep->GetMean( 1 ); }

    /// The mean of the IHistogram3D along the y axis.
    double meanY() const override { return m_rep->GetMean( 2 ); }
    /// The mean of the IHistogram3D along the z axis.
    double meanZ() const override { return m_rep->GetMean( 3 ); }
    /// The RMS of the IHistogram3D along the x axis.
    double rmsX() const override { return m_rep->GetRMS( 1 ); }
    /// The RMS of the IHistogram3D along the y axis.
    double rmsY() const override { return m_rep->GetRMS( 2 ); }
    /// The RMS of the IHistogram3D along the z axis.
    double rmsZ() const override { return m_rep->GetRMS( 3 ); }
    /// Get the x axis of the IHistogram3D.
    const AIDA::IAxis& xAxis() const override { return m_xAxis; }
    /// Get the y axis of the IHistogram3D.
    const AIDA::IAxis& yAxis() const override { return m_yAxis; }
    /// Get the z axis of the IHistogram3D.
    const AIDA::IAxis& zAxis() const override { return m_zAxis; }
    /// Get the bin number corresponding to a given coordinate along the x axis.
    int coordToIndexX( double coord ) const override { return xAxis().coordToIndex( coord ); }
    /// Get the bin number corresponding to a given coordinate along the y axis.
    int coordToIndexY( double coord ) const override { return yAxis().coordToIndex( coord ); }
    /// Get the bin number corresponding to a given coordinate along the z axis.
    int coordToIndexZ( double coord ) const override { return zAxis().coordToIndex( coord ); }

    /// Number of equivalent entries, i.e. <tt>SUM[ weight ] ^ 2 / SUM[ weight^2 ]</tt>
    double equivalentBinEntries() const override;
    /// Scale the weights and the errors of all the IHistogram's bins (in-range and out-of-range ones) by a given scale
    /// factor.
    bool scale( double scaleFactor ) override;
    /// Add to this Histogram3D the contents of another IHistogram3D.
    bool add( const INTERFACE& hist ) override {
      const Base* p = dynamic_cast<const Base*>( &hist );
      if ( !p ) throw std::runtime_error( "Cannot add profile histograms of different implementations." );
      m_rep->Add( p->m_rep.get() );
      return true;
    }

    // overwrite extraentries
    int extraEntries() const override {
      return binEntries( AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN ) +
             binEntries( AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN ) +
             binEntries( AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN ) +
             binEntries( AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN ) +
             binEntries( AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN ) +
             binEntries( AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::UNDERFLOW_BIN ) +
             binEntries( AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN, AIDA::IAxis::OVERFLOW_BIN );
    }
    /// Print (ASCII) the histogram into the output stream
    std::ostream& print( std::ostream& s ) const override;
    /// Write (ASCII) the histogram table into the output stream
    std::ostream& write( std::ostream& s ) const override;
    /// Write (ASCII) the histogram table into a file
    int write( const char* file_name ) const override;

  protected:
    Gaudi::Axis m_xAxis;
    Gaudi::Axis m_yAxis;
    Gaudi::Axis m_zAxis;
    /// Object annotations
    mutable AIDA::Annotation m_annotation;
    /// Reference to underlying implementation
    std::unique_ptr<IMPLEMENTATION> m_rep;
    // class type
    std::string m_classType;
    // cache sumEntries (allEntries)   when setting contents since Root can't compute by himself
    int m_sumEntries = 0;
  }; // end class IHistogram3D

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic3D<INTERFACE, IMPLEMENTATION>::setTitle( const std::string& title ) {
    m_rep->SetTitle( title.c_str() );
    if ( !annotation().addItem( "Title", title ) ) m_annotation.setValue( "Title", title );
    if ( !annotation().addItem( "title", title ) ) annotation().setValue( "title", title );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic3D<INTERFACE, IMPLEMENTATION>::setName( const std::string& newName ) {
    m_rep->SetName( newName.c_str() );
    m_annotation.setValue( "Name", newName );
    return true;
  }
  template <class INTERFACE, class IMPLEMENTATION>
  int Generic3D<INTERFACE, IMPLEMENTATION>::entries() const {
    return m_rep->GetEntries();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic3D<INTERFACE, IMPLEMENTATION>::allEntries() const {
    return int( m_rep->GetEntries() );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic3D<INTERFACE, IMPLEMENTATION>::minBinHeight() const {
    return m_rep->GetMinimum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic3D<INTERFACE, IMPLEMENTATION>::maxBinHeight() const {
    return m_rep->GetMaximum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic3D<INTERFACE, IMPLEMENTATION>::sumBinHeights() const {
    return m_rep->GetSumOfWeights();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic3D<INTERFACE, IMPLEMENTATION>::sumAllBinHeights() const {
    return m_rep->GetSum();
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic3D<INTERFACE, IMPLEMENTATION>::equivalentBinEntries() const {
    if ( sumBinHeights() <= 0 ) return 0;
    Stat_t stats[11]; // cover up to 3D...
    m_rep->GetStats( stats );
    return stats[0] * stats[0] / stats[1];
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic3D<INTERFACE, IMPLEMENTATION>::scale( double scaleFactor ) {
    m_rep->Scale( scaleFactor );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic3D<INTERFACE, IMPLEMENTATION>::print( std::ostream& s ) const {
    /// bin contents and errors are printed for all bins including under and overflows
    m_rep->Print( "all" );
    return s;
  }

  /// Write (ASCII) the histogram table into the output stream
  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic3D<INTERFACE, IMPLEMENTATION>::write( std::ostream& s ) const {
    s << "\n3D Histogram Table: " << std::endl;
    s << "BinX, BinY, BinZ, Height, Error " << std::endl;
    for ( int i = 0; i < xAxis().bins(); ++i )
      for ( int j = 0; j < yAxis().bins(); ++j )
        for ( int k = 0; k < zAxis().bins(); ++k )
          s << binMeanX( i, j, k ) << ", " << binMeanY( i, j, k ) << ", " << binMeanZ( i, j, k ) << ", "
            << binHeight( i, j, k ) << ", " << binError( i, j, k ) << std::endl;
    s << std::endl;
    return s;
  }

  /// Write (ASCII) the histogram table into a file
  template <class INTERFACE, class IMPLEMENTATION>
  int Generic3D<INTERFACE, IMPLEMENTATION>::write( const char* file_name ) const {
    TFile* f      = TFile::Open( file_name, "RECREATE" );
    Int_t  nbytes = m_rep->Write();
    f->Close();
    return nbytes;
  }
} // namespace Gaudi

#ifdef __clang__
#  pragma clang diagnostic pop
#elif defined( __GNUC__ )
#  pragma GCC diagnostic pop
#endif

#endif // GAUDIPI_GENERIC3D_H
