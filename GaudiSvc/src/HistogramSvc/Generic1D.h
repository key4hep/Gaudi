#ifndef GAUDISVC_GENERIC1D_H
#define GAUDISVC_GENERIC1D_H 1
/// @FIXME: AIDA interfaces visibility
#include "AIDA_visibility_hack.h"

#include <stdexcept>
#include "Axis.h"
#include "Annotation.h"
#include "GaudiKernel/HistogramBase.h"
#include "AIDA/IProfile1D.h"
#include "TFile.h"

/*
 *    Gaudi namespace
 */
namespace Gaudi {

  /** @class Generic1D Generic1D.h GaudiPI/Generic1D.h
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
  class GAUDI_API Generic1D : virtual public INTERFACE, virtual public HistogramBase {
  public:
    typedef Generic1D<INTERFACE,IMPLEMENTATION> Base;
    /// Default constructor
    Generic1D() : m_rep(0) {}
    /// Default destructor
    virtual ~Generic1D()                                 { delete m_rep;                       }
    /// The AIDA user-level unterface leaf class type
    virtual const std::string& userLevelClassType() const{ return m_classType;                 }
    /// Manual cast by class name
    virtual void* cast(const std::string& cl) const;
    /// ROOT object implementation
    TObject* representation() const                      { return m_rep;                       }
    /// Adopt ROOT histogram representation
    virtual void adoptRepresentation(TObject*rep);
    /// Get the title of the object
    virtual std::string title() const                    { return m_annotation.value("Title"); }
    /// Set the title of the object
    virtual bool setTitle(const std::string & title);
    /// object name
    std::string name() const                             { return m_annotation.value("Name");  }
    /// Set the name of the object
    bool setName( const std::string& newName );
    /// Access annotation object
    virtual AIDA::IAnnotation & annotation()             { return m_annotation;                }
    /// Access annotation object (cons)
    virtual const AIDA::IAnnotation & annotation() const { return m_annotation;                }
    /// Access to axis object
    Axis & axis ()                                       { return m_axis;                      }
    /// Get the x axis of the IHistogram1D.
    const Axis & axis () const                           { return m_axis;                      }

    /// Get the number or all the entries
    virtual int entries() const                   { return (int)m_rep->GetEntries();           }
    /// Get the number or all the entries, both in range and underflow/overflow bins of the IProfile.
    virtual int allEntries() const                { return int(m_rep->GetEntries());           }
    /// Get the number of entries in the underflow and overflow bins.
    virtual int extraEntries() const;
    /// Number of entries in the corresponding bin (ie the number of times fill was called for this bin).
    virtual int  binEntries ( int index ) const;
    // spread
    virtual double binRms(int index) const;
    /// Get the sum of in range bin heights in the IProfile.
    virtual double sumBinHeights() const         { return m_rep->GetSumOfWeights();            }
    /// Get the sum of all the bins heights (including underflow and overflow bin).
    virtual double sumAllBinHeights() const      { return m_rep->GetSum();                     }
    /// Get the sum of the underflow and overflow bin height.
    virtual double  sumExtraBinHeights () const  { return  sumAllBinHeights()-sumBinHeights(); }
    /// Get the minimum height of the in-range bins.
    virtual double minBinHeight() const          { return m_rep->GetMinimum();                 }
    /// Get the maximum height of the in-range bins.
    virtual double maxBinHeight() const          { return m_rep->GetMaximum();                 }

    /// Number of equivalent entries, i.e. <tt>SUM[ weight ] ^ 2 / SUM[ weight^2 ]</tt>
    virtual double equivalentBinEntries (  ) const;
    /// Scale the weights and the errors of all the IHistogram's bins (in-range and out-of-range ones) by a given scale factor.
    virtual bool scale( double scaleFactor );
    /// Reset the Histogram; as if just created.
    virtual bool reset();
    /// Modifies this IProfile1D by adding the contents of profile to it.
    virtual bool add(const INTERFACE & profile);
    /// operator methods
    virtual int rIndex(int index) const          { return m_axis.rIndex(index);}
    /// The weighted mean of a bin.
    virtual double  binMean(int index) const;
    /// Total height of the corresponding bin (ie the sum of the weights in this bin).
    virtual double  binHeight(int index) const;
    /// The error of a given bin.
    virtual double  binError(int index) const;
    /// The mean of the whole IHistogram1D.
    virtual double  mean() const                 { return m_rep->GetMean();                    }
    /// The RMS of the whole IHistogram1D.
    virtual double  rms () const                 {  return m_rep->GetRMS();                    }
    /// Get the bin number corresponding to a given coordinate along the x axis.
    virtual int  coordToIndex ( double coord ) const { return axis().coordToIndex(coord);}
    /// Get the Histogram's dimension.
    virtual int  dimension (  ) const  { return 1; }
    /// Print (ASCII) the histogram into the output stream
    virtual std::ostream& print( std::ostream& s ) const;
    /// Write (ASCII) the histogram table into the output stream
    virtual std::ostream& write( std::ostream& s ) const;
    /// Write (ASCII) the histogram table into a file
    virtual int write( const char* file_name ) const;

  protected:
    /// Axis member
    Axis                     m_axis;
    /// Object annotations
    mutable AIDA::Annotation m_annotation;
    /// Reference to underlying implementation
    IMPLEMENTATION*          m_rep;
    // class type
    std::string              m_classType;
    // cache sumEntries (allEntries)   when setting contents since Root can't compute by himself
    int                      m_sumEntries;
  }; // end class Generic1D

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic1D<INTERFACE,IMPLEMENTATION>::setTitle(const std::string & title)  {
    m_rep->SetTitle(title.c_str());
    if ( !annotation().addItem( "Title", title ) )
      m_annotation.setValue( "Title" , title );
    if ( !annotation().addItem( "title", title ) )
      annotation().setValue( "title", title );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic1D<INTERFACE,IMPLEMENTATION>::setName( const std::string& newName ) {
    m_rep->SetName(newName.c_str());
    m_annotation.setValue( "Name", newName );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic1D<INTERFACE,IMPLEMENTATION>::binRms(int index) const {
    return m_rep->GetBinError ( rIndex(index) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic1D<INTERFACE,IMPLEMENTATION>::binMean ( int index ) const  {
    return m_rep->GetBinCenter ( rIndex(index) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic1D<INTERFACE,IMPLEMENTATION>::binHeight ( int index ) const  {
    return m_rep->GetBinContent ( rIndex(index) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic1D<INTERFACE,IMPLEMENTATION>::binError ( int index ) const  {
    return m_rep->GetBinError ( rIndex(index) );
  }

  template <class INTERFACE, class IMPLEMENTATION>
  int Generic1D<INTERFACE,IMPLEMENTATION>::extraEntries() const {
    return binEntries(AIDA::IAxis::UNDERFLOW_BIN) +
          binEntries(AIDA::IAxis::OVERFLOW_BIN);
  }
  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic1D<INTERFACE,IMPLEMENTATION>::reset()   {
    m_sumEntries = 0;
    m_rep->Reset();
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  double Generic1D<INTERFACE,IMPLEMENTATION>::equivalentBinEntries() const  {
    if (sumBinHeights() <= 0) return 0;
    Stat_t stats[11];   // cover up to 3D...
    m_rep->GetStats(stats);
    return stats[0]*stats[0]/stats[1];
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic1D<INTERFACE,IMPLEMENTATION>::scale(double scaleFactor)   {
    m_rep->Scale ( scaleFactor );
    return true;
  }

  template <class INTERFACE, class IMPLEMENTATION>
  bool Generic1D<INTERFACE,IMPLEMENTATION>::add(const INTERFACE & h)  {
    const Generic1D<INTERFACE,IMPLEMENTATION>* p =
      dynamic_cast<const Generic1D<INTERFACE,IMPLEMENTATION>*>(&h);
    if ( p )  {
      m_rep->Add(p->m_rep);
      return true;
    }
    throw std::runtime_error("Cannot add profile histograms of different implementations.");
  }

  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic1D<INTERFACE,IMPLEMENTATION>::print( std::ostream& s ) const    {
    /// bin contents and errors are printed for all bins including under and overflows
    m_rep->Print("all");
    return s;
  }

  /// Write (ASCII) the histogram table into the output stream
  template <class INTERFACE, class IMPLEMENTATION>
  std::ostream& Generic1D<INTERFACE,IMPLEMENTATION>::write( std::ostream& s ) const  {
    s << "\n1D Histogram Table: " << std::endl;
    s << "Bin, Height, Error "  << std::endl;
    for( int i = 0; i < axis().bins(); ++i )
      s << binMean( i ) << ", "
        << binHeight( i ) << ", "
        << binError ( i ) << std::endl;
    s << std::endl;
    return s;
  }

  /// Write (ASCII) the histogram table into a file
  template <class INTERFACE, class IMPLEMENTATION>
  int Generic1D<INTERFACE,IMPLEMENTATION>::write( const char* file_name ) const
  {
    TFile *f = TFile::Open(file_name,"RECREATE");
    Int_t nbytes = m_rep->Write();
    f->Close();
    return nbytes;
  }
} // end namespace AIDA

#endif // AIDAROOT_GENERIC1D_H
