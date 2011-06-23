#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning(disable:2259)
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning(disable:4996)
#endif

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiPI.h"
#include "Generic3D.h"
#include "TH3D.h"

namespace Gaudi {

  /**@class Histogram3D
    *
    * AIDA implementation for 2 D histograms using ROOT THD2
    *
    * @author  M.Frank
    */
  class GAUDI_API Histogram3D : public DataObject, public Generic3D<AIDA::IHistogram3D,TH3D>   {
  public:
    /// Standard Constructor
    Histogram3D();
    /// Standard Constructor
    Histogram3D(TH3D* rep);
    /// Destructor.
    virtual ~Histogram3D() {}
    /// Fill bin content
    bool fill ( double x, double y, double z, double weight);
    /// Fast filling method for a given bin. It can be also the over/underflow bin
    virtual bool setBinContents( int i, int j, int k, int entries,double height,double error,double centreX, double centreY, double centreZ );
    /// Sets the rms of the histogram.
    virtual bool setRms(double rmsX, double rmsY, double rmsZ);
    // overwrite reset
    virtual bool reset();
    /// Introspection method
    void* cast(const std::string & className) const;
    /// Create new histogram from any AIDA based histogram
    void  copyFromAida(const AIDA::IHistogram3D & h);
    /// Retrieve reference to class defininition identifier
    virtual const CLID& clID() const { return classID(); }
    static const CLID& classID()     { return CLID_H3D; }

  protected:
    // cache sumwx and sumwy  when setting contents since I don't have bin mean
    double m_sumwx;
    double m_sumwy;
    double m_sumwz;
  };
}

namespace Gaudi {
  template <>
  void Generic3D<AIDA::IHistogram3D,TH3D>::adoptRepresentation(TObject* rep)  {
    TH3D* imp = dynamic_cast<TH3D*>(rep);
    if ( imp )  {
      if ( m_rep ) delete m_rep;
      m_rep = imp;
      m_xAxis.initialize(m_rep->GetXaxis(),true);
      m_yAxis.initialize(m_rep->GetYaxis(),true);
      m_zAxis.initialize(m_rep->GetZaxis(),true);
      const TArrayD* a = m_rep->GetSumw2();
      if ( 0 == a || (a && a->GetSize()==0) ) m_rep->Sumw2();
      setTitle(m_rep->GetTitle());
      return;
    }
    throw std::runtime_error("Cannot adopt native histogram representation.");
  }
}

/// Create 3D histogram with fixed bins
std::pair<DataObject*,AIDA::IHistogram3D*>
Gaudi::createH3D(const std::string& title,int nBinsX,double xlow,double xup,int nBinsY,double ylow,double yup,int nBinsZ,double zlow,double zup) {
  Histogram3D* p = new Histogram3D(new TH3D(title.c_str(),title.c_str(),nBinsX,xlow,xup,nBinsY,ylow,yup,nBinsZ,zlow,zup));
  return std::pair<DataObject*,AIDA::IHistogram3D*>(p,p);
}

/// Create 3D histogram with variable bins
std::pair<DataObject*,AIDA::IHistogram3D*>
Gaudi::createH3D(const std::string& title, const Edges& eX,const Edges& eY,const Edges& eZ) {
  Histogram3D* p = new Histogram3D(new TH3D(title.c_str(),title.c_str(),eX.size()-1,&eX.front(), eY.size()-1,&eY.front(), eZ.size()-1,&eZ.front()));
  return std::pair<DataObject*,AIDA::IHistogram3D*>(p,p);
}

std::pair<DataObject*,AIDA::IHistogram3D*> Gaudi::createH3D(const AIDA::IHistogram3D& hist)  {
  TH3D *h = getRepresentation<AIDA::IHistogram3D,TH3D>(hist);
  Histogram3D *n = h ? new Histogram3D(new TH3D(*h)) : 0;
  return std::pair<DataObject*,AIDA::IHistogram3D*>(n,n);
}

Gaudi::Histogram3D::Histogram3D() {
  m_rep = new TH3D();
  setTitle("");
  m_rep->Sumw2();
  m_sumEntries = 0;
  m_sumwx = 0;
  m_sumwy = 0;
  m_sumwz = 0;
  m_rep->SetDirectory(0);
}

Gaudi::Histogram3D::Histogram3D(TH3D* rep) {
  m_rep = 0;
  adoptRepresentation(rep);
  m_sumEntries = 0;
  m_sumwx = 0;
  m_sumwy = 0;
  m_sumwz = 0;
  m_rep->SetDirectory(0);
}

// set bin content (entries and centre are not used )
bool Gaudi::Histogram3D::setBinContents(int i,int j,int k,int entries,double height,double error,double centreX, double centreY, double centreZ ) {
  m_rep->SetBinContent(rIndexX(i), rIndexY(j), rIndexZ(k), height);
  m_rep->SetBinError(rIndexX(i), rIndexY(j), rIndexZ(k), error);
  // accumulate sum bin centers
  if (i >=0 && j >= 0 && k >= 0) {
    m_sumwx += centreX*height;
    m_sumwy += centreY*height;
    m_sumwz += centreZ*height;
  }
  m_sumEntries += entries;
  return true;
}

bool Gaudi::Histogram3D::reset (  )   {
  m_sumwx = 0;
  m_sumwy = 0;
  m_sumwz = 0;
  m_sumEntries = 0;
  m_rep->Reset ( );
  return true;
}

bool Gaudi::Histogram3D::fill ( double x, double y, double z, double weight)  {
  m_rep->Fill ( x , y, z, weight );
  return true;
}

void* Gaudi::Histogram3D::cast(const std::string & className) const   {
  if (className == "AIDA::IHistogram3D")   {
    return (AIDA::IHistogram3D*)this;
  }
  else if (className == "AIDA::IHistogram")   {
    return (AIDA::IHistogram*)this;
  }
  return 0;
}

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison are meant
#pragma warning(push)
#pragma warning(disable:1572)
#endif
bool Gaudi::Histogram3D::setRms(double rmsX, double rmsY, double rmsZ   ) {
  m_rep->SetEntries(m_sumEntries);
  std::vector<double> stat(11);
  // sum weights
  stat[0] =  sumBinHeights();
  stat[1] = 0;
  if (equivalentBinEntries() != 0)
    stat[1] = (  sumBinHeights() * sumBinHeights() ) / equivalentBinEntries();
  stat[2] = m_sumwx;
  double meanX = 0;
  if ( sumBinHeights() != 0 ) meanX =  m_sumwx/ sumBinHeights();
  stat[3] = ( meanX*meanX + rmsX*rmsX )* sumBinHeights();
  stat[4] = m_sumwy;
  double meanY = 0;
  if ( sumBinHeights() != 0 ) meanY =  m_sumwy/ sumBinHeights();
  stat[5] = ( meanY*meanY + rmsY*rmsY )* sumBinHeights();
  stat[6] = 0;
  stat[7] = m_sumwz;
  double meanZ = 0;
  if ( sumBinHeights() != 0 ) meanZ =  m_sumwz/ sumBinHeights();
  stat[8] = ( meanZ*meanZ + rmsZ*rmsZ )* sumBinHeights();
  // do not need to use sumwxy sumwxz and sumwyz

  m_rep->PutStats(&stat.front());
  return true;
}

void Gaudi::Histogram3D::copyFromAida(const AIDA::IHistogram3D & h) {
  delete m_rep;
  // implement here the copy
  const char* tit = h.title().c_str();
  if (h.xAxis().isFixedBinning() && h.yAxis().isFixedBinning() &&  h.zAxis().isFixedBinning() )  {
    m_rep = new TH3D(tit,tit,
      h.xAxis().bins(), h.xAxis().lowerEdge(), h.xAxis().upperEdge(),
      h.yAxis().bins(), h.yAxis().lowerEdge(), h.yAxis().upperEdge(),
      h.zAxis().bins(), h.zAxis().lowerEdge(), h.zAxis().upperEdge() );
  }
  else {
    Edges eX, eY, eZ;
    for (int i =0; i < h.xAxis().bins(); ++i)
      eX.push_back(h.xAxis().binLowerEdge(i));
    // add also upperedges at the end
    eX.push_back(h.xAxis().upperEdge() );
    for (int i =0; i < h.yAxis().bins(); ++i)
      eY.push_back(h.yAxis().binLowerEdge(i));
    // add also upperedges at the end
    eY.push_back(h.yAxis().upperEdge() );
    for (int i =0; i < h.zAxis().bins(); ++i)
      eZ.push_back(h.zAxis().binLowerEdge(i));
    // add also upperedges at the end
    eZ.push_back(h.zAxis().upperEdge() );
    m_rep = new TH3D(tit,tit,eX.size()-1,&eX.front(),eY.size()-1,&eY.front(),eZ.size()-1,&eZ.front());
  }
  m_xAxis.initialize(m_rep->GetXaxis(),true);
  m_yAxis.initialize(m_rep->GetYaxis(),true);
  m_zAxis.initialize(m_rep->GetZaxis(),true);
  const TArrayD* a = m_rep->GetSumw2();
  if ( 0 == a || (a && a->GetSize()==0) ) m_rep->Sumw2();
  m_sumEntries = 0;
  m_sumwx = 0;
  m_sumwy = 0;
  m_sumwz = 0;

  // statistics
  double sumw   = h.sumBinHeights();
  double sumw2  = 0;
  if (h.equivalentBinEntries() != 0)
    sumw2 = ( sumw * sumw ) /h.equivalentBinEntries();
  double sumwx  = h.meanX()*h.sumBinHeights();
  double sumwx2 = (h.meanX()*h.meanX() + h.rmsX()*h.rmsX() )*h.sumBinHeights();
  double sumwy  = h.meanY()*h.sumBinHeights();
  double sumwy2 = (h.meanY()*h.meanY() + h.rmsY()*h.rmsY() )*h.sumBinHeights();
  double sumwz  = h.meanZ()*h.sumBinHeights();
  double sumwz2 = (h.meanZ()*h.meanZ() + h.rmsZ()*h.rmsZ() )*h.sumBinHeights();
  double sumwxy = 0;
  double sumwxz = 0;
  double sumwyz = 0;

  // copy the contents in  (AIDA underflow/overflow are -2,-1)
  for (int i=-2; i < xAxis().bins(); ++i) {
    for (int j=-2; j < yAxis().bins(); ++j) {
      for (int k=-2; k < zAxis().bins(); ++k) {
        m_rep->SetBinContent(rIndexX(i), rIndexY(j), rIndexZ(k), h.binHeight(i,j,k) );
        m_rep->SetBinError(rIndexX(i), rIndexY(j), rIndexZ(k), h.binError(i,j,k) );
        // calculate statistics
        if ( i >= 0 && j >= 0 && k >= 0) {
          sumwxy += h.binHeight(i,j,k)*h.binMeanX(i,j,k)*h.binMeanY(i,j,k);
          sumwxz += h.binHeight(i,j,k)*h.binMeanX(i,j,k)*h.binMeanZ(i,j,k);
          sumwyz += h.binHeight(i,j,k)*h.binMeanY(i,j,k)*h.binMeanZ(i,j,k);
        }
      }
    }
  }
  // need to do set entries after setting contents otherwise root will recalulate them
  // taking into account how many time  SetBinContents() has been called
  m_rep->SetEntries(h.allEntries());

  // fill stat vector
  std::vector<double> stat(11);
  stat[0] = sumw;
  stat[1] = sumw2;
  stat[2] = sumwx;
  stat[3] = sumwx2;
  stat[4] = sumwy;
  stat[5] = sumwy2;
  stat[6] = sumwxy;
  stat[7] = sumwz;
  stat[8] = sumwz2;
  stat[9] = sumwxz;
  stat[10] = sumwyz;
  m_rep->PutStats(&stat.front());
}

#ifdef __ICC
// re-enable icc remark #1572
#pragma warning(pop)
#endif

typedef Gaudi::Histogram3D H3D;
DECLARE_DATAOBJECT_FACTORY(H3D)
