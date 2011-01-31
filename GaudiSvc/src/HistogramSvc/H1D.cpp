// $Id: H1D.cpp,v 1.14 2007/07/16 13:36:17 hmd Exp $
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
#include "H1D.h"
#include "GaudiPI.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ObjectFactory.h"

std::pair<DataObject*,AIDA::IHistogram1D*> Gaudi::createH1D(const std::string& title,int nBins,double xlow, double xup)  {
  Histogram1D* p = new Histogram1D(new TH1D(title.c_str(),title.c_str(),nBins,xlow,xup));
  return std::pair<DataObject*,AIDA::IHistogram1D*>(p,p);
}

std::pair<DataObject*,AIDA::IHistogram1D*> Gaudi::createH1D(const std::string& title, const Edges& e)  {
  Histogram1D* p = new Histogram1D(new TH1D(title.c_str(),title.c_str(),e.size()-1,&e.front()));
  return std::pair<DataObject*,AIDA::IHistogram1D*>(p,p);
}

std::pair<DataObject*,AIDA::IHistogram1D*> Gaudi::createH1D(const AIDA::IHistogram1D& hist)  {
  TH1D *h = getRepresentation<AIDA::IHistogram1D,TH1D>(hist);
  Histogram1D *n = h ? new Histogram1D(new TH1D(*h)) : 0;
  return std::pair<DataObject*,AIDA::IHistogram1D*>(n,n);
}
namespace Gaudi {
  template<> void *Generic1D<AIDA::IHistogram1D,TH1D>::cast(const std::string & className) const  {
    if (className == "AIDA::IHistogram1D")
      return const_cast<AIDA::IHistogram1D*>((AIDA::IHistogram1D*)this);
    if (className == "AIDA::IHistogram")
      return const_cast<AIDA::IHistogram*>((AIDA::IHistogram*)this);
    return 0;
  }

  template<> int Generic1D<AIDA::IHistogram1D,TH1D>::binEntries (int index) const  {
    if (binHeight(index)<=0) return 0;
    double xx =  binHeight(index)/binError(index);
    return int(xx*xx+0.5);
  }

  template <>
  void Generic1D<AIDA::IHistogram1D,TH1D>::adoptRepresentation(TObject* rep)  {
    TH1D* imp = dynamic_cast<TH1D*>(rep);
    if ( imp )  {
      if ( m_rep ) delete m_rep;
      m_rep = imp;
      return;
    }
    throw std::runtime_error("Cannot adopt native histogram representation.");
  }
}

Gaudi::Histogram1D::Histogram1D()  {
  m_rep = new TH1D();
  init("",false);
}

Gaudi::Histogram1D::Histogram1D(TH1D* rep)  {
  m_rep = rep;
  init(m_rep->GetTitle());
  initSums();
}

void Gaudi::Histogram1D::init(const std::string& title, bool initialize_axis)  {
  m_classType = "IHistogram1D";
  if ( initialize_axis )  {
    m_axis.initialize(m_rep->GetXaxis(),false);
  }
  const TArrayD* a = m_rep->GetSumw2();
  if ( 0 == a || (a && a->GetSize()==0) ) m_rep->Sumw2();
  setTitle(title);
  m_rep->SetDirectory(0);
  m_sumEntries = 0;
  m_sumwx = 0;
}

void Gaudi::Histogram1D::initSums()  {
  m_sumwx = 0;
  m_sumEntries = 0;
  for(int i=1, n=m_rep->GetNbinsX(); i<=n; ++i)    {
    m_sumwx += m_rep->GetBinContent(i)*m_rep->GetBinCenter(i);
    m_sumEntries += (int)m_rep->GetBinContent(i);
  }
}

bool Gaudi::Histogram1D::reset()   {
  m_sumwx = 0;
  m_sumEntries = 0;
  return Base::reset();
}

/// Adopt ROOT histogram representation
void Gaudi::Histogram1D::adoptRepresentation(TObject* rep)  {
  Gaudi::Generic1D<AIDA::IHistogram1D,TH1D>::adoptRepresentation(rep);
  if ( m_rep )  {
    init(m_rep->GetTitle());
    initSums();
  }
}

bool Gaudi::Histogram1D::setBinContents(int i,int entries ,double height,double error,double centre) {
  m_rep->SetBinContent(rIndex(i),height);
  m_rep->SetBinError(rIndex(i),error);
  // accumulate sumwx for in range bins
  if (i != AIDA::IAxis::UNDERFLOW_BIN && i !=  AIDA::IAxis::OVERFLOW_BIN )
    m_sumwx  += centre*height;
  m_sumEntries += entries;
  return true;
}

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//   The comparison are meant
#pragma warning(push)
#pragma warning(disable:1572)
#endif
bool Gaudi::Histogram1D::setRms(double rms) {
  m_rep->SetEntries(m_sumEntries);
  std::vector<double> stat(11);
  // sum weights
  stat[0] =  sumBinHeights();
  stat[1] = 0;
  if (equivalentBinEntries() != 0)
    stat[1] = (  sumBinHeights() * sumBinHeights() ) / equivalentBinEntries();
  stat[2] = m_sumwx;
  double mean = 0;
  if ( sumBinHeights() != 0 ) mean =  m_sumwx/ sumBinHeights();
  stat[3] = ( mean*mean + rms*rms )* sumBinHeights();
  m_rep->PutStats(&stat.front());
  return true;
}

// set histogram statistics
bool Gaudi::Histogram1D::setStatistics(int allEntries,double eqBinEntries,double mean,double rms) {
  m_rep->SetEntries(allEntries);
  // fill statistcal vector for Root
  std::vector<double> stat(11);
  // sum weights
  stat[0] =  sumBinHeights();
  // sum weights **2
  stat[1] = 0;
  if (eqBinEntries != 0)
    stat[1] = (  sumBinHeights() * sumBinHeights() ) / eqBinEntries;
  // sum weights * x
  stat[2] = mean*sumBinHeights();
  // sum weight * x **2
  stat[3] = ( mean*mean + rms*rms )* sumBinHeights();
  m_rep->PutStats(&stat.front());
  return true;
}

bool Gaudi::Histogram1D::fill ( double x,double weight )  {
  (weight == 1.) ? m_rep->Fill(x) : m_rep->Fill(x,weight);
  return true;
}

void Gaudi::Histogram1D::copyFromAida(const AIDA::IHistogram1D & h) {
 // implement here the copy
  std::string tit = h.title()+"Copy";
  delete m_rep;
  if (h.axis().isFixedBinning() )  {
    m_rep = new TH1D(tit.c_str(),tit.c_str(),h.axis().bins(),h.axis().lowerEdge(),h.axis().upperEdge());
  }
  else {
    Edges e;
    for (int i =0; i < h.axis().bins(); ++i)  {
      e.push_back(h.axis().binLowerEdge(i));
    }
    // add also upperedges at the end
    e.push_back(h.axis().upperEdge() );
    m_rep = new TH1D(tit.c_str(),tit.c_str(),e.size()-1,&e.front());
  }
  m_axis.initialize(m_rep->GetXaxis(),false);
  m_rep->Sumw2();
  m_sumEntries = 0;
  m_sumwx = 0;
  // sumw
  double sumw = h.sumBinHeights();
  // sumw2
  double sumw2 = 0;
  if (h.equivalentBinEntries() != 0)
    sumw2 = ( sumw * sumw ) /h.equivalentBinEntries();

  double sumwx = h.mean()*h.sumBinHeights();
  double sumwx2 = (h.mean()*h.mean() + h.rms()*h.rms() )*h.sumBinHeights();

  // copy the contents in
  for (int i=-2; i < axis().bins(); ++i) {
    // root binning starts from one !
    m_rep->SetBinContent(rIndex(i),h.binHeight(i) );
    m_rep->SetBinError(rIndex(i),h.binError(i) );
  }
  // need to do set entries after setting contents otherwise root will recalulate them
  // taking into account how many time  SetBinContents() has been called
  m_rep->SetEntries(h.allEntries());
    // stat vector
  std::vector<double> stat(11);
  stat[0] = sumw;
  stat[1] = sumw2;
  stat[2] = sumwx;
  stat[3] = sumwx2;
  m_rep->PutStats(&stat.front());
}

#ifdef __ICC
// re-enable icc remark #1572
#pragma warning(pop)
#endif

StreamBuffer& Gaudi::Histogram1D::serialize(StreamBuffer& s) {
  //DataObject::serialize(s);
  std::string title;
  int size;
  s >> size;
  for (int j = 0; j < size; j++) {
    std::string key, value;
    s >> key >> value;
    annotation().addItem (key, value);
    if ("Title" == key) {
      title = value;
    }
  }
  double lowerEdge, upperEdge, binHeight, binError;
  int    isFixedBinning, bins;
  s >> isFixedBinning >> bins;

  if ( m_rep ) delete m_rep;
  if ( isFixedBinning ) {
    s >> lowerEdge >> upperEdge;
    m_rep = new TH1D(title.c_str(),title.c_str(),bins,lowerEdge,upperEdge);
  } else {
    Edges edges;
    edges.resize(bins);
    for ( int i = 0; i <= bins; ++i )
      s >> *(double*)&edges[i];
    m_rep = new TH1D(title.c_str(),title.c_str(),edges.size()-1,&edges.front());
  }
  m_axis.initialize(m_rep->GetXaxis(),true);
  m_rep->Sumw2();
  m_sumEntries = 0;
  m_sumwx = 0;

  for ( int i = 0; i <= bins + 1; ++i ) {
    s >> binHeight >> binError;
    m_rep->SetBinContent( i, binHeight );
    m_rep->SetBinError( i, binError );
  }
  Stat_t allEntries;
  s >> allEntries;
  m_rep->SetEntries( allEntries );
  Stat_t stats[4];                                    // stats array
  s >> stats[0] >> stats[1] >> stats[2] >> stats[3];
  m_rep->PutStats( stats );
  return s;
}

StreamBuffer& Gaudi::Histogram1D::serialize(StreamBuffer& s) const {
  //DataObject::serialize(s);
  s << static_cast<int>( annotation().size() );
  for (int i = 0; i < annotation().size(); i++) {
    s << annotation().key(i);
    s << annotation().value(i);
  }
  const AIDA::IAxis & axis( this->axis() );
  const int isFixedBinning = axis.isFixedBinning();
  const int bins = axis.bins();
  s << isFixedBinning << bins;
  if ( isFixedBinning ) {
    s << axis.lowerEdge();
  }
  else {
    for ( int i = 0; i < bins; ++i )
      s << axis.binLowerEdge(i);
  }
  s << axis.upperEdge();
  for ( int i = 0; i <= bins + 1; ++i )
    s << m_rep->GetBinContent(i) << m_rep->GetBinError( i );

  s << m_rep->GetEntries();
  Stat_t stats[4];  // stats array
  m_rep->GetStats( stats );
  s << stats[0] << stats[1] << stats[2] << stats[3];
  return s;
}

typedef Gaudi::Histogram1D H1D;
DECLARE_DATAOBJECT_FACTORY(H1D)
