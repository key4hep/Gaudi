#ifndef HISTOGRAMSVC_H1D_H
#define HISTOGRAMSVC_H1D_H 1

#include "GaudiKernel/DataObject.h"
#include "Generic1D.h"
#include "GaudiPI.h"
#include "TH1D.h"

namespace Gaudi {

  /**@class Histogram1D
    *
    * AIDA implementation for 1 D histograms using ROOT THD1
    *
    * @author  M.Frank
    */
  class GAUDI_API Histogram1D : public DataObject, public Gaudi::Generic1D<AIDA::IHistogram1D,TH1D>   {
  private:
    void init(const std::string& title, bool initialize_axis=true);
    void initSums();
  protected:
    /// cache sumwx  when setting contents since I don't have bin mean
    double m_sumwx;
  public:
    /// Standard constructor
    Histogram1D();
    /// Standard constructor with initialization. The histogram representation will be adopted
    Histogram1D(TH1D* rep);
    /// Destructor.
    virtual ~Histogram1D() {}
    /// Adopt ROOT histogram representation
    virtual void adoptRepresentation(TObject*rep);
    /// set bin content (entries and centre are not used )
    virtual bool setBinContents(int i,int entries ,double height,double error,double centre);
    /// need to overwrite reset to reset the sums
    virtual bool reset();
    /// set histogram statistics
    virtual bool setStatistics(int allEntries,double eqBinEntries,double mean,double rms);
    /// Fill the Profile1D with a value and the corresponding weight.
    virtual bool fill(double x,double weight);
    /// Update histogram RMS
    bool setRms(double rms);
    /// Create new histogram from any AIDA based histogram
    void copyFromAida(const AIDA::IHistogram1D & h);
    /// Retrieve reference to class defininition identifier
    virtual const CLID& clID() const { return classID(); }
    static const CLID& classID()     { return CLID_H1D; }
    /** Serialization mechanism, Serialize the object for reading.
      * @param s the StreamBuffer containing the data to be read
      * @return the resulting StreamBuffer, after reading
      */
    StreamBuffer& serialize(StreamBuffer& s);

    /** Serialization mechanism, Serialize the object for writing.
      * @param s the StreamBuffer where to write the data
      * @return the resulting StreamBuffer, after wrinting
      */
    StreamBuffer& serialize(StreamBuffer& s) const;

  }; // end class IHistogram1D
} // end namespace Gaudi
#endif // HISTOGRAMSVC_H1D_H
