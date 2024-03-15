/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLES_COLLISION_H
#define GAUDIEXAMPLES_COLLISION_H 1

// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"

namespace Gaudi {
  namespace Examples {

    // CLID definition
    static const CLID& CLID_Collision = 111;

    /** @class Collision
     */

    class GAUDI_API Collision : public DataObject {
    public:
      /// Constructors
      Collision() = default;
      Collision( int num ) : DataObject(), m_collision( num ) {}

      /// Retrieve reference to class definition structure
      const CLID&        clID() const override { return classID(); }
      static const CLID& classID() { return CLID_Collision; }

      void setCollision( int num ) { m_collision = num; }
      int  collision() const { return m_collision; }
      //  std::vector<double, CLHEP::HepGenMatrix::Alloc<double,25> >& matrix() {
      //    return m_matrix;
      //  }
      /// Serialize the object for writing
      StreamBuffer& serialize( StreamBuffer& s ) const;
      /// Serialize the object for reading
      StreamBuffer& serialize( StreamBuffer& s );

    private:
      /// Collision number
      int m_collision{ 0 };
      //  std::vector<double, CLHEP::HepGenMatrix::Alloc<double,25> >           m_matrix;
    };

    /// Serialize the object for writing
    inline StreamBuffer& Collision::serialize( StreamBuffer& s ) const { return s << m_collision; }

    /// Serialize the object for reading
    inline StreamBuffer& Collision::serialize( StreamBuffer& s ) { return s >> m_collision; }
  } // namespace Examples
} // namespace Gaudi

#endif // GAUDIEXAMPLES_COLLISION_H
