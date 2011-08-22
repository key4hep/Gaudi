#ifndef HISTOGRAMSVC_H2D_H
#define HISTOGRAMSVC_H2D_H 1

#include "GaudiKernel/DataObject.h"
#include "Generic2D.h"
#include "GaudiPI.h"
#include "TH2D.h"

using namespace AIDA;

namespace Gaudi {
  class Histogram1D;

  /**@class Histogram2D
    *
    * AIDA implementation for 2 D histograms using ROOT THD2
    *
    * @author  M.Frank
    */
  class GAUDI_API Histogram2D : public DataObject, public Generic2D<IHistogram2D,TH2D>   {
  public:
    /// Standard Constructor
    Histogram2D();
    /// Standard initializing Constructor with TH2D representation to be adopted
    Histogram2D(TH2D* rep);
    /// Destructor.
    virtual ~Histogram2D() {}
    /// Fill the Histogram2D with a value and the
    bool  fill(double x,double y,double weight = 1.);
    /// Fast filling method for a given bin. It can be also the over/underflow bin
    virtual bool setBinContents(int binIndexX,int binIndexY,int entries,
                                double height,double error,double centreX,
                                double centreY );
    /// Sets the rms of the histogram.
    bool setRms( double rmsX,double rmsY );
    // overwrite reset
    bool reset();
    /// Create new histogram from any AIDA based histogram
    void copyFromAida(const IHistogram2D& h);
    /// Retrieve reference to class defininition identifier
    virtual const CLID& clID() const { return classID(); }
    static const CLID& classID()     { return CLID_H2D; }
  protected:
    // cache sumwx and sumwy  when setting contents since I don't have bin mean
    double m_sumwx;
    double m_sumwy;
  };
}
#endif // HISTOGRAMSVC_H2D_H
