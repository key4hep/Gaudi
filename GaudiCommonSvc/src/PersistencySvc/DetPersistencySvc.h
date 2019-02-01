#ifndef PERSISTENCYSVC_DETPERSISTENCYSVC_H
#define PERSISTENCYSVC_DETPERSISTENCYSVC_H 1

#include "PersistencySvc.h"

class DetPersistencySvc : virtual public PersistencySvc {
public:
  // Standard Constructor
  DetPersistencySvc( const std::string& name, ISvcLocator* svc );
};

#endif // PERSISTENCYSVC_DETPERSISTENCYSVC_H
