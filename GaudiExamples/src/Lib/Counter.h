// $Header: /local/reps/Gaudi/GaudiExamples/src/Lib/Counter.h,v 1.4 2007/09/28 11:48:17 marcocle Exp $
#ifndef GAUDIEXAMPLES_COUNTER_H
#define GAUDIEXAMPLES_COUNTER_H 1

// Include files
#include "GaudiKernel/DataObject.h"
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
      /// Destructor
      virtual ~Counter() { }

      /// Retrieve reference to class definition structure
      virtual const CLID& clID() const  { return classID(); }
      static const CLID& classID() { return CLID_Counter; }

      /// Retrieve counter value
      int value() const { return m_counter; }
      /// Increment counter
      int increment() { return ++m_counter; }
      /// Decrement counter
      int decrement() { return --m_counter; }
      /// Set value
      void set(int val) {  m_counter = val; }
    private:
      /// Counter number
      int                 m_counter;
    };

  }
}
#endif    // GAUDIEXAMPLES_COUNTER_H
