/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/KeyedContainer.h>
#include <GaudiKernel/KeyedObject.h>
#include <GaudiKernel/SmartRefVector.h>
#include <GaudiTestSuite/Event.h>
#include <GaudiTestSuite/MyTrack.h>

namespace Gaudi {
  namespace TestSuite {
    // Forward declarations
    //    class MyTrack;

    // CLID definition
    static const CLID& CLID_MyVertex = 1356;

    /** @class MyVertex
     *
     *  Simple class that represents a vertex for testing purposes
     *
     *  @author Markus Frank
     */
    class GAUDI_API MyVertex : public KeyedObject<int> {
    protected:
      /// The vertex location
      float m_x, m_y, m_z;
      /// Link to Top level event
      SmartRef<Event> m_event;
      /// Link to mother track
      SmartRef<MyTrack> m_motherParticle;
      /// Links to all daughter particles
      SmartRefVector<MyTrack> m_decayParticles;
      /// Vector of collisions this object belongs to
      SmartRefVector<Collision> m_collisions;

    public:
      /// Standard constructor
      MyVertex();
      /// Standard constructor
      MyVertex( float x, float y, float z );
      /// Standard Destructor
      ~MyVertex() override;
      /// Retrieve pointer to class definition structure
      const CLID&        clID() const override { return classID(); }
      static const CLID& classID() { return CLID_MyVertex; }
      /// Accessors: Retrieve x-component of the track momentum
      float x() const { return m_x; }
      /// Accessors: Retrieve y-component of the track momentum
      float y() const { return m_y; }
      /// Accessors: Retrieve z-component of the track momentum
      float z() const { return m_z; }
      /// Accessors: Update x-component of the track momentum
      void setX( float x ) { m_x = x; }
      /// Accessors: Update y-component of the track momentum
      void setY( float y ) { m_y = y; }
      /// Accessors: Update z-component of the track momentum
      void setZ( float z ) { m_z = z; }

      /// Access to the source track object (constant case)
      const Event* event() const { return m_event; }
      /// Access to event object
      void setEvent( Event* evt ) { m_event = evt; }
      /// Mother track
      const MyTrack* motherParticle() const;

      /// Set mother track
      void setMotherParticle( MyTrack* mother );

      /// Access to decay particles
      const SmartRefVector<MyTrack>& decayParticles() const;

      /// Add decay particle
      void addDecayParticle( MyTrack* track );

      /// Remove decay vertex
      void removeDecayParticle( MyTrack* vtx );

      /// Access to collisions
      const SmartRefVector<Collision>& collisions() const;

      /// Add collision
      void addCollision( Collision* vtx );

      /// Remove collision
      void removeCollision( Collision* vtx );

      /// Serialize the object for writing
      StreamBuffer& serialize( StreamBuffer& s ) const override;
      /// Serialize the object for reading
      StreamBuffer& serialize( StreamBuffer& s ) override;
    };

    // Definition of all container types of MCParticle
    typedef KeyedContainer<MyVertex> MyVertexVector;

    /// Mother particle
    inline const MyTrack* MyVertex::motherParticle() const { return m_motherParticle; }

    /// Set origin vertex
    inline void MyVertex::setMotherParticle( MyTrack* mother ) { m_motherParticle = mother; }

    /// Access to decay vertices
    inline const SmartRefVector<MyTrack>& MyVertex::decayParticles() const { return m_decayParticles; }

    /// Add decay vertex
    inline void MyVertex::addDecayParticle( MyTrack* p ) { m_decayParticles.push_back( SmartRef<MyTrack>( p ) ); }

    /// Remove decay vertex
    inline void MyVertex::removeDecayParticle( MyTrack* p ) {
      SmartRefVector<MyTrack>::iterator i;
      for ( i = m_decayParticles.begin(); i != m_decayParticles.end(); ++i ) {
        if ( i->target() == p ) {
          m_decayParticles.erase( i );
          return;
        }
      }
    }

    /// Access to decay vertices
    inline const SmartRefVector<Collision>& MyVertex::collisions() const { return m_collisions; }

    /// Add decay vertex
    inline void MyVertex::addCollision( Collision* c ) { m_collisions.push_back( SmartRef<Collision>( c ) ); }

    /// Remove decay vertex
    inline void MyVertex::removeCollision( Collision* c ) {
      SmartRefVector<Collision>::iterator i;
      for ( i = m_collisions.begin(); i != m_collisions.end(); ++i ) {
        if ( i->target() == c ) {
          m_collisions.erase( i );
          return;
        }
      }
    }
  } // namespace TestSuite
} // namespace Gaudi
