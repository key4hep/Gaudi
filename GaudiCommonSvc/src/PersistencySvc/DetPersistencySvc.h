#ifndef PERSISTENCYSVC_DETPERSISTENCYSVC_H
#define PERSISTENCYSVC_DETPERSISTENCYSVC_H 1

#include "PersistencySvc.h"

class DetPersistencySvc  : virtual public PersistencySvc
{
public:

    // Initialize the service.
    virtual StatusCode initialize();

    // Finalize the service.
    virtual StatusCode finalize();

    // Standard Constructor
    DetPersistencySvc(const std::string& name, ISvcLocator* svc);

    // Standard Destructor
    virtual ~DetPersistencySvc();

};

#endif // PERSISTENCYSVC_DETPERSISTENCYSVC_H

