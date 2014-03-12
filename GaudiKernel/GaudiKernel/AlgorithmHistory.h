// $Id: AlgorithmHistory.h,v 1.1 2006/11/09 10:24:04 mato Exp $

#ifndef GAUDIKERNEL_ALGORITHMHISTORY_H
#define GAUDIKERNEL_ALGORITHMHISTORY_H

// An object of this class carries the history information
// which is specific to a Gaudi algorithm.

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include <iosfwd>
#include <string>
#include <typeinfo>
#include <vector>

class Algorithm;
class Property;
class JobHistory;

/** @class AlgorithmHistory AlgorithmHistory.h
 *
 *  AlgorithmHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API AlgorithmHistory: public HistoryObj, public IVersHistoryObj {

public:  // typedefs

  // List of subalgorithm histories. This may change.
  typedef std::vector<const AlgorithmHistory*> HistoryList;

private:  // data

  // Algorithm full type.
  std::string m_algorithm_type;

  // Algorithm version.
  std::string m_algorithm_version;

  // Algorithm name.
  std::string m_algorithm_name;

  // Algorithm
  const Algorithm* m_algorithm;

  // Properties.
  PropertyList m_properties;

  // Subalgorithm histories.
  HistoryList m_subalgorithm_histories;

  // Link to jobHistory
  const JobHistory *m_jobHistory;

public:  // functions

  // Constructor from the algorithm.
  explicit AlgorithmHistory(const Algorithm& alg, const JobHistory* job);

  // All-fields Constructor for persistency
  explicit AlgorithmHistory(const std::string& algVersion,
			    const std::string& algName,
			    const std::string& algType,
			    const PropertyList& props,
			    const HistoryList& subHists);
  // Destructor.
  virtual ~AlgorithmHistory();

  // Class IDs
  virtual const CLID& clID() const { return classID(); }
  static const CLID& classID();

  // Return the algorithm type.
  const std::string& algorithm_type() const { return m_algorithm_type; }

  // Return the algorithm version.
  const std::string& algorithm_version() const { return m_algorithm_version; }

  // Return the algorithm name.
  const std::string& algorithm_name() const { return m_algorithm_name; }

  // The actual algorithm
  const Algorithm* algorithm() const { return m_algorithm; }

  // Return the algorithm properties.
  const PropertyList& properties() const { return m_properties; }

  // Return the subalgorithm histories.
  const HistoryList& subalgorithm_histories() const
    { return m_subalgorithm_histories; }

  // Return the jobHistory
  const JobHistory* jobHistory() const { return m_jobHistory; }


  void dump(std::ostream &, const bool isXML=false, int indent=0) const;

  const std::string& name() const { return algorithm_name(); }
  const std::string& type() const { return algorithm_type(); }
  const std::string& version() const { return algorithm_version(); }

};

// Output stream.
GAUDI_API std::ostream& operator<<(std::ostream& lhs, const AlgorithmHistory& rhs);

#endif
