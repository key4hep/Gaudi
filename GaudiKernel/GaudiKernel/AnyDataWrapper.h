#ifndef GAUDIKERNEL_ANYDATAWRAPPER_H
#define GAUDIKERNEL_ANYDATAWRAPPER_H

//Include files
#include "GaudiKernel/DataObject.h"

/*
 * Initial version from Benedikt
 */
class GAUDI_API AnyDataWrapperBase : public DataObject {
 public:
   // ugly hack to circumvent the usage of boost::any yet 
   // DataSvc would need a templated register method 
   virtual ~AnyDataWrapperBase(){};
};

template<class T>
class GAUDI_API AnyDataWrapperBasic : public AnyDataWrapperBase {
 public:

  AnyDataWrapperBasic() : AnyDataWrapperBase(), m_data(nullptr){};
  virtual ~AnyDataWrapperBasic(); 

  const T* getData() {return m_data;} 
  void setData(T* data) {m_data = data;}

 private:
  T* m_data; 

};

template<class T>
AnyDataWrapperBasic<T>::~AnyDataWrapperBasic<T>() {
  if (m_data!=nullptr) delete m_data;
}

/*
 * New version of the code
 */
template<class T>
class GAUDI_API AnyDataWrapper: public AnyDataWrapperBase {
 public:

 AnyDataWrapper(T&& data) : AnyDataWrapperBase(),
    m_data(std::move(data)){};

 AnyDataWrapper(AnyDataWrapper&& other) : AnyDataWrapperBase(),
    m_data(std::move(other.m_data)){};

  
  virtual ~AnyDataWrapper() {};

  const T& getData() const {return m_data;} 

 private:
  T m_data; 

};



#endif
