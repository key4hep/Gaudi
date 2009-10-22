#include <cmath>
#include "P1D.h"
#include "GaudiKernel/ObjectFactory.h"
typedef Gaudi::Profile1D P1D;
DECLARE_DATAOBJECT_FACTORY(P1D)

std::pair<DataObject*,AIDA::IProfile1D*> Gaudi::createProf1D
( const std::string& title , 
  int nBins  , double xlow, double xup, 
  double ylow, double yup, const std::string& opt )  
{
  TProfile* _p = new TProfile(title.c_str(),title.c_str(),nBins,xlow,xup,ylow,yup,opt.c_str() ) ;
  Profile1D* p = new Profile1D(_p);
  return std::pair<DataObject*,AIDA::IProfile1D*>(p,p);
}

std::pair<DataObject*,AIDA::IProfile1D*> Gaudi::createProf1D
( const std::string& title, 
  const Edges& e, double ylow, double yup , 
  const std::string& opt )  
{  
  Profile1D* p = new Profile1D(new TProfile(title.c_str(),title.c_str(),e.size()-1,&e.front(),ylow,yup,opt.c_str()));
  return std::pair<DataObject*,AIDA::IProfile1D*>(p,p);
}

std::pair<DataObject*,AIDA::IProfile1D*> Gaudi::createProf1D(const AIDA::IProfile1D& hist)  {
  TProfile *h = getRepresentation<AIDA::IProfile1D,TProfile>(hist);
  Profile1D *n = h ? new Profile1D(new TProfile(*h)) : 0;
  return std::pair<DataObject*,AIDA::IProfile1D*>(n,n);
}

namespace Gaudi {
  template <> 
  int Generic1D<AIDA::IProfile1D,TProfile>::binEntries (int index) const  { 
    return int(m_rep->GetBinEntries( rIndex(index) )+0.5);
  }

  template <> void* Generic1D<AIDA::IProfile1D,TProfile>::cast(const std::string& className) const {  
    if (className == "AIDA::IProfile1D") 
      return const_cast<AIDA::IProfile1D*>((AIDA::IProfile1D*)this); 
    else if (className == "AIDA::IProfile") 
      return const_cast<AIDA::IProfile*>((AIDA::IProfile*)this); 
    else if (className == "AIDA::IBaseHistogram") 
      return const_cast<AIDA::IBaseHistogram*>((AIDA::IBaseHistogram*)this); 
    return 0;
  }

  template <> 
  void Generic1D<AIDA::IProfile1D,TProfile>::adoptRepresentation(TObject* rep)  {
    TProfile* imp = dynamic_cast<TProfile*>(rep);
    if ( imp )  {
    if ( m_rep ) delete m_rep;
      m_rep = imp;
      m_axis.initialize(m_rep->GetXaxis(),true);
      const TArrayD* a = m_rep->GetSumw2();
      if ( 0 == a || (a && a->GetSize()==0) ) m_rep->Sumw2();
      setTitle(m_rep->GetTitle());
      return;
    }
    throw std::runtime_error("Cannot adopt native histogram representation.");
  }
}

Gaudi::Profile1D::Profile1D() {
  m_rep = new TProfile();
  init("",false);
}

Gaudi::Profile1D::Profile1D(TProfile* rep)  {
  m_rep = rep;
  init(m_rep->GetTitle());
}

void Gaudi::Profile1D::init(const std::string& title, bool initialize_axis) {
  m_classType = "IProfile1D";
  setTitle(title);
  setName(title);
  if ( initialize_axis )  {
    axis().initialize(m_rep->GetXaxis(),false);
  }
  //m_rep->SetErrorOption("s"); 
  m_rep->SetDirectory(0);
  m_sumEntries = 0;
}

bool Gaudi::Profile1D::setBinContents(int i, int entries,double height,double /*error*/, double spread, double /* centre */ ) { 
  m_rep->SetBinEntries(rIndex(i), entries ); 
  // set content takes in root height * entries 
  m_rep->SetBinContent(rIndex(i), height*entries );
  // set error takes sqrt of bin sum(w*y**2) 
  double sumwy2Bin = ( spread*spread + height*height )*entries; 
  m_rep->SetBinError(rIndex(i), sqrt(sumwy2Bin) ); 
  m_sumEntries += entries;
  // not very efficient (but do evey bin since root cannot figure out by himself)
  m_rep->SetEntries(m_sumEntries);
  return true;
}

bool Gaudi::Profile1D::fill ( double x, double y, double weight )  {
  (weight == 1.) ? m_rep->Fill(x,y) : m_rep->Fill(x,y,weight);
  return true;
}
