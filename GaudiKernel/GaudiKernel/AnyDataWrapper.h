#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

//Include files
#include "GaudiKernel/DataObject.h"

class GAUDI_API AnyDataWrapperBase : public DataObject {
 public:
   // ugly hack to circumvent the usage of boost::any yet 
   // DataSvc would need a templated register method 
   virtual ~AnyDataWrapperBase(){};
};

template<class T>
class GAUDI_API AnyDataWrapper : public AnyDataWrapperBase {
 public:

  AnyDataWrapper() : AnyDataWrapperBase(), m_data(nullptr){};
  virtual ~AnyDataWrapper(); 

  const T* getData() {return m_data;} 
  void setData(T* data) {m_data = data;}

 private:
  T* m_data; 

};

template<class T>
AnyDataWrapper<T>::~AnyDataWrapper<T>() {
  if (m_data!=nullptr) delete m_data;
}

#endif
