/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
// ============================================================================
// Include files
// ============================================================================
namespace {
  // ==========================================================================
  /// the actual type to represent the bin content
  typedef std::pair<double, double> Bin;
  /// vector of bins
  typedef std::vector<Bin> Bins;
  // ==========================================================================
  /** @struct Edges
   *  helper structure to represent the edges
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-21
   */
  struct Edges final {
    //
    void setHighEdge( double value ) { high = value; }
    void setLowEdge( double value ) { low = value; }
    void setNBins( unsigned int value ) { nbins = value; }
    void setEdges( std::vector<double> value ) { edges = std::move( value ); }
    //
    Edges& operator-=( double value ) {
      setLowEdge( value );
      return *this;
    }
    Edges& operator+=( double value ) {
      setHighEdge( value );
      return *this;
    }
    Edges& operator*=( std::vector<double> value ) {
      setEdges( std::move( value ) );
      return *this;
    }
    Edges& operator/=( unsigned int value ) {
      setNBins( value );
      return *this;
    }
    //
    bool         ok() const { return edges.empty() ? low < high && 0 < nbins : 2 < edges.size(); }
    unsigned int nBins() const { return edges.empty() ? nbins : edges.size() - 1; }
    //
    double              low;
    double              high;
    unsigned int        nbins;
    std::vector<double> edges;
  };
  // ==========================================================================
  /** @struct H1
   *  the trivial representation of the 1D-histogram
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-21
   */
  struct H1 final {
    //
    void setName( std::string value ) { m_name = std::move( value ); }
    void setTitle( std::string value ) { m_title = std::move( value ); }

    void setEdges( Edges value ) { m_edges = std::move( value ); }
    void setHighEdge( const double value ) { m_edges.setHighEdge( value ); }
    void setLowEdge( const double value ) { m_edges.setLowEdge( value ); }
    void setNBins( const unsigned int value ) { m_edges.setNBins( value ); }

    void setBins( Bins value ) { m_bins = std::move( value ); }
    //
    H1& operator*=( std::string value ) {
      setName( std::move( value ) );
      return *this;
    }
    H1& operator/=( std::string value ) {
      setTitle( std::move( value ) );
      return *this;
    }
    H1& operator^=( const double value ) {
      setHighEdge( value );
      return *this;
    }
    H1& operator-=( const double value ) {
      setLowEdge( value );
      return *this;
    }
    H1& operator|=( const unsigned int value ) {
      setNBins( value );
      return *this;
    }

    H1& operator&=( Edges value ) {
      setEdges( std::move( value ) );
      return *this;
    }
    H1& operator+=( Bins value ) {
      setBins( std::move( value ) );
      return *this;
    }
    //
    bool ok() const {
      if ( m_bins.empty() ) { return false; }
      if ( !m_edges.ok() ) { return false; }
      if ( m_bins.size() != m_edges.nBins() + 2 ) { return false; }
      return true;
    }
    //
    std::string m_name;
    std::string m_title;
    Edges       m_edges;
    Bins        m_bins;
  };
  // ==========================================================================
  /** @struct H2
   *  the trivial representation of the 2D-histogram
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-21
   */
  struct H2 final {
    //
    void setName( std::string value ) { m_name = std::move( value ); }
    void setTitle( std::string value ) { m_title = std::move( value ); }
    void setXEdges( Edges value ) { m_xedges = std::move( value ); }
    void setYEdges( Edges value ) { m_yedges = std::move( value ); }
    void setBins( Bins value ) { m_bins = std::move( value ); }
    //
    H2& operator*=( std::string value ) {
      setName( std::move( value ) );
      return *this;
    }
    H2& operator/=( std::string value ) {
      setTitle( std::move( value ) );
      return *this;
    }
    H2& operator&=( Edges value ) {
      setXEdges( std::move( value ) );
      return *this;
    }
    H2& operator|=( Edges value ) {
      setYEdges( std::move( value ) );
      return *this;
    }
    H2& operator+=( Bins value ) {
      setBins( std::move( value ) );
      return *this;
    }
    //
    bool ok() const {
      if ( m_bins.empty() ) { return false; }
      if ( !m_xedges.ok() ) { return false; }
      if ( !m_yedges.ok() ) { return false; }
      if ( m_bins.size() != ( m_xedges.nBins() + 2 ) * ( m_yedges.nBins() + 2 ) ) { return false; }
      return true;
    }
    //
    //
    std::string m_name;
    std::string m_title;
    Edges       m_xedges;
    Edges       m_yedges;
    Bins        m_bins;
  };
  // ==========================================================================
  /** @struct H3
   *  the trivial representation of the 3D-histogram
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date 2009-10-21
   */
  struct H3 final {
    //
    void setName( std::string value ) { m_name = std::move( value ); }
    void setTitle( std::string value ) { m_title = std::move( value ); }
    void setXEdges( Edges value ) { m_xedges = std::move( value ); }
    void setYEdges( Edges value ) { m_yedges = std::move( value ); }
    void setZEdges( Edges value ) { m_zedges = std::move( value ); }
    void setBins( Bins value ) { m_bins = std::move( value ); }
    //
    H3& operator*=( std::string value ) {
      setName( std::move( value ) );
      return *this;
    }
    H3& operator/=( std::string value ) {
      setTitle( std::move( value ) );
      return *this;
    }
    H3& operator&=( Edges value ) {
      setXEdges( std::move( value ) );
      return *this;
    }
    H3& operator|=( Edges value ) {
      setYEdges( std::move( value ) );
      return *this;
    }
    H3& operator-=( Edges value ) {
      setZEdges( std::move( value ) );
      return *this;
    }
    H3& operator+=( Bins value ) {
      setBins( std::move( value ) );
      return *this;
    }
    //
    bool ok() const {
      if ( m_bins.empty() ) { return false; }
      if ( !m_xedges.ok() ) { return false; }
      if ( !m_yedges.ok() ) { return false; }
      if ( !m_zedges.ok() ) { return false; }
      if ( m_bins.size() != ( m_xedges.nBins() + 2 ) * ( m_yedges.nBins() + 2 ) * ( m_zedges.nBins() + 2 ) ) {
        return false;
      }
      return true;
    }
    //
    //
    std::string m_name;
    std::string m_title;
    Edges       m_xedges;
    Edges       m_yedges;
    Edges       m_zedges;
    Bins        m_bins;
  };

  // ==========================================================================
} // end of anonymous namespace
// ============================================================================
// The END
// ============================================================================
