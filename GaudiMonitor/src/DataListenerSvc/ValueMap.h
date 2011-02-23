#ifndef DATALISTENERSVC_VALUEMAP_H 
#define DATALISTENERSVC_VALUEMAP_H 1

// Include files
// from STL
#include <string>

/** @class ValueMap ValueMap.h Scouter/ValueMap.h
 *
 *  ValueMap is used in conjunction with Scouter and MLSender to carry the information declared by IMonitorSvc's declareInfo. 
 *  It converts the variable to a void* pointer and stores its type information in an enum. This way, the variable can be converted
 *  to its original form when being written or published. 
 *
 *  @author Ben King
 *  @date   2006-07-03
 */

class ValueMap  {
public: 

  enum m_type { 
    m_int, 
    m_long, 
    m_double, 
    m_bool, 
    m_string, 
    m_null 
  };
  

  /// standard Constructor
  ValueMap (void *pointer, m_type pointerType)
  {
    ptr = pointer;
    ptrType = pointerType;
  }
  
  /// default Constructor
  ValueMap() 
  {
    ptr = NULL;
    ptrType = m_null;
  }

  /// Destructor
  virtual ~ValueMap() {};
  

  // SET METHODS

  /// Sets pointer
  void set_ptr(void *pointer) { ptr = pointer;}
  /// Sets pointer type (enum)
  void set_ptrType (m_type pointerType) { ptrType = pointerType; }
  

  // ACCESS METHODS

  /// Return pointer
  void* get_ptr() { return ptr; }
  /// Return pointer type (enum)
  m_type get_ptrType() { return ptrType; }


private:

  void* ptr;
  m_type ptrType;


};
#endif // DATALISTENERSVC_VALUEMAP_H
