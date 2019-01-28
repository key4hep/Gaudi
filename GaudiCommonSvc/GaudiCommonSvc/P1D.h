#ifndef GAUDISVC_P1D_H
#define GAUDISVC_P1D_H

#include "Generic1D.h"
#include <AIDA/IProfile1D.h>
#include <GaudiKernel/DataObject.h>
#include <TProfile.h>

/*
 *    Gaudi namespace
 */
namespace Gaudi
{

  /**@class Profile1D
    *
    * AIDA implementation for 1 D profiles using ROOT TProfile
    *
    * @author  M.Frank
    */
  class GAUDI_API Profile1D : public DataObject, public Generic1D<AIDA::IProfile1D, TProfile>
  {
    typedef AIDA::IAnnotation IAnnotation;

  private:
    void init( const std::string& title, bool initialize_axis = true );

  public:
    /// Default Constructor
    Profile1D();
    /// Standard constructor with initialization. The histogram representation will be adopted
    Profile1D( TProfile* rep );

    /// Fill the Profile1D with a value and the corresponding weight.
    bool fill( double x, double y, double weight = 1. ) override;
    virtual bool setBinContents( int i, int entries, double height, double error, double spread, double centre );
    /// Retrieve reference to class defininition identifier
    const CLID&        clID() const override { return classID(); }
    static const CLID& classID() { return CLID_ProfileH; }

  private:
    std::mutex m_fillSerialization;

  }; // end class IProfile1D
} // end namespace Gaudi

#endif // GAUDISVC_P1D_H
