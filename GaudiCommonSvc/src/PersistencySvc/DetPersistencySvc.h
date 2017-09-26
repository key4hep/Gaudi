#ifndef PERSISTENCYSVC_DETPERSISTENCYSVC_H
#define PERSISTENCYSVC_DETPERSISTENCYSVC_H 1

#include "PersistencySvc.h"

class DetPersistencySvc : virtual public PersistencySvc
{
public:
  // Standard Constructor
  DetPersistencySvc( const std::string& name, ISvcLocator* svc );

  // Standard Destructor
  ~DetPersistencySvc() override = default;
};

#endif // PERSISTENCYSVC_DETPERSISTENCYSVC_H
