#ifndef GAUDIKERNEL_DATAHANDLEHOLDERVISTOR
#define GAUDIKERNEL_DATAHANDLEHOLDERVISTOR 1

#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/GaudiException.h"
#include <set>
#include <typeinfo>

class DHHVisitor : public IDataHandleVisitor {
public:

   DHHVisitor(DataObjIDColl& ido, DataObjIDColl& odo);
   
   const DataObjIDColl& ignoredInpKeys() const { return m_ign_i; }
   const DataObjIDColl& ignoredOutKeys() const { return m_ign_o; }
   
   void visit(const IDataHandleHolder* visitee) override;
   
private:

   DataObjIDColl &m_ido, &m_odo;
   DataObjIDColl m_ign_i, m_ign_o;

};

#endif
