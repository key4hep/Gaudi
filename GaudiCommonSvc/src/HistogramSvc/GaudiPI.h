#ifndef GAUDISVC_GAUDIPI_H
#define GAUDISVC_GAUDIPI_H
/// @FIXME: AIDA interfaces visibility
#include "AIDA_visibility_hack.h"

#include <vector>
#include <utility>
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "GaudiKernel/HistogramBase.h"

class DataObject;
class TH2D;

namespace Gaudi {
  typedef std::vector<double> Edges;

  template <class Q, class T> T* getRepresentation(const Q& hist)  {
    T* result = 0;
    const HistogramBase *p = dynamic_cast<const HistogramBase*>(&hist);
    if ( p )  {
      result = dynamic_cast<T*>(p->representation());
    }
    return result;
  }
  /// Copy constructor
  std::pair<DataObject*,AIDA::IHistogram1D*> createH1D(const AIDA::IHistogram1D& hist);
  /// Creator for 1D histogram with fixed bins
  std::pair<DataObject*,AIDA::IHistogram1D*> createH1D(const std::string& title, int nBins=10, double lowerEdge=0., double upperEdge=1.);
  /// Creator for 1D histogram with variable bins
  std::pair<DataObject*,AIDA::IHistogram1D*> createH1D(const std::string& title, const Edges& e);

  /// Copy constructor
  std::pair<DataObject*,AIDA::IHistogram2D*> createH2D(const AIDA::IHistogram2D& hist);
  /// "Adopt" constructor
  std::pair<DataObject*,AIDA::IHistogram2D*> createH2D(TH2D* rep);
  /// Creator for 2 D histograms with fixed bins
  std::pair<DataObject*,AIDA::IHistogram2D*> createH2D(const std::string& title, int binsX, double iminX, double imaxX, int binsY, double iminY, double imaxY);
  /// Creator for 2 D histograms with variable bins
  std::pair<DataObject*,AIDA::IHistogram2D*> createH2D(const std::string& title, const Edges& eX, const Edges& eY);
  /// Create 1D slice from 2D histogram
  std::pair<DataObject*,AIDA::IHistogram1D*> slice1DX(const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);
  /// Create 1D profile in X from 2D histogram
  std::pair<DataObject*,AIDA::IProfile1D*>   profile1DX(const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);
  /// Create 1D projection in X from 2D histogram
  std::pair<DataObject*,AIDA::IHistogram1D*> project1DX(const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);
  /// Create 1D slice from 2D histogram
  std::pair<DataObject*,AIDA::IHistogram1D*> slice1DY  (const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);
  /// Create 1D profile in Y from 2D histogram
  std::pair<DataObject*,AIDA::IProfile1D*>   profile1DY(const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);
  /// Create 1D projection in Y from 2D histogram
  std::pair<DataObject*,AIDA::IHistogram1D*> project1DY(const std::string& name, const AIDA::IHistogram2D& h, int firstbin, int lastbin);

  /// Copy constructor
  std::pair<DataObject*,AIDA::IHistogram3D*> createH3D(const AIDA::IHistogram3D& hist);
  /// Create 3D histogram with fixed bins
  std::pair<DataObject*,AIDA::IHistogram3D*> createH3D(const std::string& title, int nBinsX, double xlow, double xup, int nBinsY, double ylow, double yup, int nBinsZ, double zlow, double zup);
  /// Create 3D histogram with variable bins
  std::pair<DataObject*,AIDA::IHistogram3D*> createH3D(const std::string& title, const Edges  & eX,const Edges& eY, const Edges& eZ);

  /// Copy constructor
  std::pair<DataObject*,AIDA::IProfile1D*> createProf1D(const AIDA::IProfile1D& hist);
  /// Creator of 1D profile with fixed bins
  std::pair<DataObject*,AIDA::IProfile1D*> createProf1D
  (const std::string& title, int nBins, double xlow, double xup, double ylow, double yup, const std::string& opt="" );
  /// Creator of 1D profile with variable bins
  std::pair<DataObject*,AIDA::IProfile1D*> createProf1D
  (const std::string& title, const Edges& e, double ylow, double yup, const std::string& opt="");

  /// Copy constructor
  std::pair<DataObject*,AIDA::IProfile2D*> createProf2D(const AIDA::IProfile2D& hist);
  /// Creator for 2 D profile with fixed bins
  std::pair<DataObject*,AIDA::IProfile2D*> createProf2D(const std::string& title, int binsX, double iminX, double imaxX, int binsY, double iminY, double imaxY, double lowerValue, double upperValue);
  /// Creator for 2 D profile with variable bins
  std::pair<DataObject*,AIDA::IProfile2D*> createProf2D(const std::string& title, const Edges& eX, const Edges& eY, double lowerValue, double upperValue);
}
#endif // GAUDIPI_GAUDIPI_H
