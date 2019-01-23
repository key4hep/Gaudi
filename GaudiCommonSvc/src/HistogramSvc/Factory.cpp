#include <GaudiKernel/ObjectFactory.h>

#include <GaudiCommonSvc/H1D.h>
#include <GaudiCommonSvc/H2D.h>
#include <GaudiCommonSvc/H3D.h>
#include <GaudiCommonSvc/HistogramSvc.h>
#include <GaudiCommonSvc/P1D.h>
#include <GaudiCommonSvc/P2D.h>

namespace
{
  typedef Gaudi::Histogram1D H1D;
  typedef Gaudi::Profile1D   P1D;
  typedef Gaudi::Histogram2D H2D;
  typedef Gaudi::Profile2D   P2D;
  typedef Gaudi::Histogram3D H3D;
}

DECLARE_COMPONENT( HistogramSvc )

DECLARE_DATAOBJECT_FACTORY( H1D )
DECLARE_DATAOBJECT_FACTORY( P1D )
DECLARE_DATAOBJECT_FACTORY( H2D )
DECLARE_DATAOBJECT_FACTORY( P2D )
DECLARE_DATAOBJECT_FACTORY( H3D )
