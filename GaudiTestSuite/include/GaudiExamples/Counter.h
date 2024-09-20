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
#ifndef GAUDIEXAMPLES_COUNTER_H
#define GAUDIEXAMPLES_COUNTER_H 1

// Include files
#include <GaudiKernel/DataObject.h>
#include <iostream>

namespace Gaudi {
  namespace Examples {

    class Collision;

    // CLID definition
    static const CLID& CLID_Counter = 109;

    /** @class Counter
        Simple counter object

        @author Markus Frank
    */

    class GAUDI_API Counter : public DataObject {

    public:
      /// Constructors
      Counter();

      /// Retrieve reference to class definition structure
      const CLID&        clID() const override { return classID(); }
      static const CLID& classID() { return CLID_Counter; }

      /// Retrieve counter value
      int value() const { return m_counter; }
      /// Increment counter
      int increment() { return ++m_counter; }
      /// Decrement counter
      int decrement() { return --m_counter; }
      /// Set value
      void set( int val ) { m_counter = val; }

    private:
      /// Counter number
      int m_counter;
    };
  } // namespace Examples
} // namespace Gaudi
#endif // GAUDIEXAMPLES_COUNTER_H
