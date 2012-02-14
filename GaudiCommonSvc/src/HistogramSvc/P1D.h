#ifndef GAUDISVC_P1D_H
#define GAUDISVC_P1D_H

#include "GaudiKernel/DataObject.h"
#include "GaudiPI.h"
#include "Generic1D.h"
#include "TProfile.h"

/*
 *    Gaudi namespace
 */
namespace Gaudi {

  /**@class Profile1D
    *
    * AIDA implementation for 1 D profiles using ROOT TProfile
    *
    * @author  M.Frank
    */
  class GAUDI_API Profile1D : public DataObject, public Generic1D<AIDA::IProfile1D,TProfile>  {
    typedef AIDA::IAnnotation IAnnotation;
  private:
    void init(const std::string& title, bool initialize_axis=true);
  public:
    /// Default Constructor
    Profile1D();
    /// Standard constructor with initialization. The histogram representation will be adopted
    Profile1D(TProfile* rep);
    /// Destructor.
    virtual ~Profile1D()      {  }
    /// Fill the Profile1D with a value and the corresponding weight.
    virtual bool fill(double x, double y, double weight = 1.);
    virtual bool setBinContents(int i, int entries,double height,double error, double spread, double centre);
    /// Retrieve reference to class defininition identifier
    virtual const CLID& clID() const { return classID(); }
    static const CLID& classID()     { return CLID_ProfileH; }
  }; // end class IProfile1D
} // end namespace Gaudi

#endif // GAUDISVC_P1D_H
