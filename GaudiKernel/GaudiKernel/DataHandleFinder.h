#ifndef GAUDIKERNEL_DATAHANDLEFINDER
#define GAUDIKERNEL_DATAHANDLEFINDER 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IDataHandleHolder.h"

/** @class DataHandleFinder DataHandleFinder.h GaudiKernel/DataHandleFinder.h
 *
 *  Implements the IDataHandleVisitor interface
 *  Class used to explore heirarchy of nested IDataHandleHolders
 *  <UL>
 *  <LI> Instantiated with a specific target data object to find
 *  <LI> Descends heirarchy of tools, examining all data handles
 *  <LI> Returns a vector of tool/algorithm names that own handles
 *  to the target data
 *  </UL>
 *
 *  @author Ben Wynne
 */

class DataHandleFinder : public IDataHandleVisitor {
public:
  DataHandleFinder( DataObjID const& target );

  void                      visit( const IDataHandleHolder* visitee ) override;
  std::vector<std::string>& holderNames() { return m_holderNames; }

private:
  DataObjID const&         m_target;
  std::vector<std::string> m_holderNames;
};

#endif
