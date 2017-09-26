#ifndef GAUDICOMMONSVC_P2D_H
#define GAUDICOMMONSVC_P2D_H

#include "Generic2D.h"
#include <AIDA/IProfile1D.h>
#include <GaudiKernel/DataObject.h>
#include <TProfile2D.h>

namespace Gaudi
{

  /**@class Profile2D
    *
    * AIDA implementation for 2 D profiles using ROOT TProfile2D
    *
    * @author  M.Frank
    */
  class GAUDI_API Profile2D : public DataObject, public Generic2D<AIDA::IProfile2D, TProfile2D>
  {
  public:
    /// Default Constructor
    Profile2D() : Base( new TProfile2D() )
    {
      m_classType = "IProfile2D";
      m_rep->SetErrorOption( "s" );
      m_rep->SetDirectory( nullptr );
    }
    /// Default Constructor with representation object
    Profile2D( TProfile2D* rep );

    /// Fill bin content
    bool fill( double x, double y, double z, double weight ) override
    {
      m_rep->Fill( x, y, z, weight );
      return true;
    }
    /// Retrieve reference to class defininition identifier
    const CLID& clID() const override { return classID(); }
    static const CLID& classID() { return CLID_ProfileH2; }
  };
}

#endif
