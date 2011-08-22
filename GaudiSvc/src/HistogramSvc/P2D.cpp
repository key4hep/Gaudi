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
#include "Generic2D.h"
#include "TProfile2D.h"
#include "TH2D.h"

namespace Gaudi {

  /**@class Profile2D
    *
    * AIDA implementation for 2 D profiles using ROOT TProfile2D
    *
    * @author  M.Frank
    */
  class GAUDI_API Profile2D : public DataObject, public Generic2D<AIDA::IProfile2D,TProfile2D>   {
  public:
    /// Default Constructor
    Profile2D() {
      m_classType = "IProfile2D";
      m_rep = new TProfile2D();
      m_rep->SetErrorOption("s");
      m_rep->SetDirectory(0);
      m_sumEntries = 0;
    }
    /// Default Constructor with representation object
    Profile2D(TProfile2D* rep);
    /// Destructor.
    virtual ~Profile2D()  {}
    /// Fill bin content
    bool fill(double x,double y,double z,double weight)  {
      m_rep->Fill(x,y,z,weight);
      return true;
    }
    /// Retrieve reference to class defininition identifier
    virtual const CLID& clID() const { return classID(); }
    static const CLID& classID()     { return CLID_ProfileH2; }
  };
}

namespace Gaudi {
  template <>
  void* Generic2D<AIDA::IProfile2D,TProfile2D>::cast(const std::string& className) const  {
    if (className == "AIDA::IProfile2D")
      return const_cast<AIDA::IProfile2D*>((AIDA::IProfile2D*)this);
    else if (className == "AIDA::IProfile")
      return const_cast<AIDA::IProfile*>((AIDA::IProfile*)this);
    else if (className == "AIDA::IBaseHistogram")
      return const_cast<AIDA::IBaseHistogram*>((AIDA::IBaseHistogram*)this);
    return 0;
  }

  template <>
  int  Generic2D<AIDA::IProfile2D,TProfile2D>::binEntries(int idX, int idY) const  {
    int rBin = m_rep->GetBin(rIndexX(idX),rIndexY(idY));
    return int(m_rep->GetBinEntries(rBin)+0.5);
  }

  template <>
  void Generic2D<AIDA::IProfile2D,TProfile2D>::adoptRepresentation(TObject* rep) {
    TProfile2D* imp = dynamic_cast<TProfile2D*>(rep);
    if ( imp )  {
      if ( m_rep ) delete m_rep;
      m_rep = imp;
      m_xAxis.initialize(m_rep->GetXaxis(),true);
      m_yAxis.initialize(m_rep->GetYaxis(),true);
      setTitle(m_rep->GetTitle());
      return;
    }
    throw std::runtime_error("Cannot adopt native histogram representation.");
  }
}

std::pair<DataObject*,AIDA::IProfile2D*> Gaudi::createProf2D(const std::string& title, const Edges& eX, const Edges& eY, double /* zlow */ , double /* zup */) {
  // Not implemented in ROOT! Can only use TProfile2D with no z-limits
  Profile2D* p = new Profile2D(new TProfile2D(title.c_str(),title.c_str(),eX.size()-1,&eX.front(), eY.size()-1,&eY.front()/*,zlow,zup */));
  return std::pair<DataObject*,AIDA::IProfile2D*>(p,p);
}

std::pair<DataObject*,AIDA::IProfile2D*>
Gaudi::createProf2D(const std::string& title,int binsX,double xlow,double xup,int binsY,double ylow,double yup,double zlow,double zup) {
  Profile2D* p = new Profile2D(new TProfile2D(title.c_str(),title.c_str(),binsX,xlow,xup,binsY,ylow,yup,zlow,zup));
  return std::pair<DataObject*,AIDA::IProfile2D*>(p,p);
}

std::pair<DataObject*,AIDA::IProfile2D*> Gaudi::createProf2D(const AIDA::IProfile2D& hist)  {
  TProfile2D *h = getRepresentation<AIDA::IProfile2D,TProfile2D>(hist);
  Profile2D *n = h ? new Profile2D(new TProfile2D(*h)) : 0;
  return std::pair<DataObject*,AIDA::IProfile2D*>(n,n);
}

Gaudi::Profile2D::Profile2D(TProfile2D* rep)    {
  m_rep = 0;
  m_classType = "IProfile2D";
  rep->SetDirectory(0);
  adoptRepresentation(rep);
  m_sumEntries = 0;
}

typedef Gaudi::Profile2D P2D;
DECLARE_DATAOBJECT_FACTORY(P2D)
