// $Id: AlgToolHistory.h,v 1.1 2006/11/09 10:24:04 mato Exp $

#ifndef GAUDIKERNEL_ALGTOOLHISTORY_H
#define GAUDIKERNEL_ALGTOOLHISTORY_H

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

class Property;
class AlgTool;
class JobHistory;

#include <string>
#include <vector>

/** @class AlgToolHistory AlgToolHistory.h
 *
 *  AlgToolHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API AlgToolHistory: public HistoryObj, virtual public IVersHistoryObj {

private:  // data

  // Algtool full type.
  std::string m_type;

  // Algtool version.
  std::string m_version;

  // Algtool name.
  std::string m_name;

  // Pointer to the algtool
  const AlgTool *m_tool;

  // Properties.
  PropertyList m_properties;

  // Link to jobHistory
  const JobHistory *m_jobHistory;

public:

  AlgToolHistory(const AlgTool& alg, const JobHistory* job);

  AlgToolHistory(const std::string& algVersion,
		 const std::string& algName,
		 const std::string& algType,
		 const AlgTool* tool,
		 const PropertyList& props,
		 const JobHistory* job);

  // Destructor.
  virtual ~AlgToolHistory() {};

  // Class IDs
  virtual const CLID& clID() const { return classID(); }
  static const CLID& classID();

 // Return the algtool type.
  const std::string& algtool_type() const { return m_type; }

  // Return the algtool version.
  const std::string& algtool_version() const { return m_version; }

  // Return the algtool name.
  const std::string& algtool_name() const { return m_name; }

  // Pointer to the algtool
  const AlgTool* algtool_instance() const { return m_tool; }

  // Return the algorithm properties.
  const PropertyList& properties() const { return m_properties; }

  void dump(std::ostream &, const bool isXML=false, int indent=0) const;

  const std::string& name() const { return algtool_name(); }
  const std::string& type() const { return algtool_type(); }
  const std::string& version() const { return algtool_version(); }


};

// Output stream.
GAUDI_API std::ostream& operator<<(std::ostream& lhs, const AlgToolHistory& rhs);

#endif

