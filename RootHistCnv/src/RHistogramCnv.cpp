// $Id: RHistogramCnv.cpp,v 1.3 2007/07/16 13:37:52 hmd Exp $
#include "RHistogramCnv.h"
#include "GaudiKernel/CnvFactory.h"

#include "TProfile.h"
#include "AIDA/IProfile1D.h"
namespace RootHistCnv {
  typedef RHistogramCnv<TProfile,TProfile,Gaudi::HistogramBase> RootHistCnvP1DCnv;
  template<> const CLID& RootHistCnvP1DCnv::classID()     { return CLID_ProfileH;    }
  DECLARE_CONVERTER_FACTORY(RootHistCnvP1DCnv)
}

#include "TProfile2D.h"
#include "AIDA/IProfile2D.h"
namespace RootHistCnv {
  typedef RHistogramCnv<TProfile2D,TProfile2D,Gaudi::HistogramBase> RootHistCnvP2DCnv;
  template<> const CLID& RootHistCnvP2DCnv::classID()     { return CLID_ProfileH2;    }
  DECLARE_CONVERTER_FACTORY(RootHistCnvP2DCnv)
}

#include "TH1D.h"
#include "AIDA/IHistogram1D.h"
namespace RootHistCnv {
  typedef RHistogramCnv<TH1D,TH1,Gaudi::HistogramBase> RootHistCnvH1DCnv;
  template<> const CLID& RootHistCnvH1DCnv::classID()     { return CLID_H1D;         }
  DECLARE_CONVERTER_FACTORY(RootHistCnvH1DCnv)
}

#include "TH2D.h"
#include "AIDA/IHistogram2D.h"
namespace RootHistCnv {
  typedef RHistogramCnv<TH2D,TH2,Gaudi::HistogramBase> RootHistCnvH2DCnv;
  template<> const CLID& RootHistCnvH2DCnv::classID()     { return CLID_H2D;         }
  DECLARE_CONVERTER_FACTORY(RootHistCnvH2DCnv)
}

#include "TH3D.h"
#include "AIDA/IHistogram3D.h"
namespace RootHistCnv {
  typedef RootHistCnv::RHistogramCnv<TH3D,TH3,Gaudi::HistogramBase> RootHistCnvH3DCnv;
  template<> const CLID& RootHistCnvH3DCnv::classID()     { return CLID_H3D;         }
  DECLARE_CONVERTER_FACTORY(RootHistCnvH3DCnv)
}
