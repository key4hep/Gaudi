// $Header: /local/reps/Gaudi/GaudiExamples/src/Lib/Collision.h,v 1.2
// 2006/11/27 09:53:06 hmd Exp $
#ifndef GAUDIEXAMPLES_COLLISION_H
#define GAUDIEXAMPLES_COLLISION_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/DataObject.h"
#include "CLHEP/Matrix/Matrix.h"


namespace Gaudi {
  namespace Examples {

    // CLID definition
    static const CLID& CLID_Collision = 111;

    /** @class Collision
     */

    class GAUDI_API Collision : public DataObject {
    public:
      /// Constructors
      Collision() : DataObject() { }
      Collision(int num) : DataObject(), m_collision(num) { }
      /// Destructor
      virtual ~Collision() { }

      /// Retrieve reference to class definition structure
      virtual const CLID& clID() const  { return classID(); }
      static const CLID& classID() { return CLID_Collision; }

      void setCollision(int num) {
        m_collision = num;
      }
      int collision() const {
	return m_collision;
      }
      //  std::vector<double, CLHEP::HepGenMatrix::Alloc<double,25> >& matrix() {
      //    return m_matrix;
      //  }
      /// Serialize the object for writing
      virtual StreamBuffer& serialize( StreamBuffer& s ) const;
      /// Serialize the object for reading
      virtual StreamBuffer& serialize( StreamBuffer& s );

    private:
      /// Collision number
      int                m_collision;
      //  std::vector<double, CLHEP::HepGenMatrix::Alloc<double,25> >           m_matrix;
    };

    /// Serialize the object for writing
    inline StreamBuffer& Collision::serialize( StreamBuffer& s ) const {
      return s << m_collision;
    }


    /// Serialize the object for reading
    inline StreamBuffer& Collision::serialize( StreamBuffer& s ) {
      return s >> m_collision;
    }

  }
}

#endif    // GAUDIEXAMPLES_COLLISION_H
