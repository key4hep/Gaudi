/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// An object of this class carries the history information
// which is specific to a Gaudi algorithm.

#include <GaudiKernel/HistoryObj.h>
#include <GaudiKernel/IVersHistoryObj.h>

#include <iosfwd>
#include <string>
#include <typeinfo>
#include <vector>

namespace Gaudi {
  class Algorithm;
}
class JobHistory;

/** @class AlgorithmHistory AlgorithmHistory.h
 *
 *  AlgorithmHistory class definition
 *
 *  @author: Charles Leggett
 *
 */

class GAUDI_API AlgorithmHistory : public HistoryObj, public IVersHistoryObj {

public: // typedefs
  // List of subalgorithm histories. This may change.
  typedef std::vector<const AlgorithmHistory*> HistoryList;

private: // data
  // Algorithm full type.
  std::string m_algorithm_type;

  // Algorithm version.
  std::string m_algorithm_version;

  // Algorithm name.
  std::string m_algorithm_name;

  // Algorithm
  const Gaudi::Algorithm* m_algorithm;

  // Properties.
  PropertyList m_properties;

  // Subalgorithm histories.
  HistoryList m_subalgorithm_histories;

  // Link to jobHistory
  const JobHistory* m_jobHistory;

public: // functions
  // Constructor from the algorithm.
  explicit AlgorithmHistory( const Gaudi::Algorithm& alg, const JobHistory* job );

  // All-fields Constructor for persistency
  explicit AlgorithmHistory( std::string algVersion, std::string algName, std::string algType,
                             const PropertyList& props, const HistoryList& subHists );
  // Destructor.
  virtual ~AlgorithmHistory();

  // Class IDs
  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();

  // Return the algorithm type.
  const std::string& algorithm_type() const { return m_algorithm_type; }

  // Return the algorithm version.
  const std::string& algorithm_version() const { return m_algorithm_version; }

  // Return the algorithm name.
  const std::string& algorithm_name() const { return m_algorithm_name; }

  // The actual algorithm
  const Gaudi::Algorithm* algorithm() const { return m_algorithm; }

  // Return the algorithm properties.
  const PropertyList& properties() const override { return m_properties; }

  // Return the subalgorithm histories.
  const HistoryList& subalgorithm_histories() const { return m_subalgorithm_histories; }

  // Return the jobHistory
  const JobHistory* jobHistory() const { return m_jobHistory; }

  std::ostream& dump( std::ostream&, bool isXML, int indent ) const override;

  const std::string& name() const override { return algorithm_name(); }
  const std::string& type() const override { return algorithm_type(); }
  const std::string& version() const override { return algorithm_version(); }

  // Output stream.
  friend std::ostream& operator<<( std::ostream& lhs, const AlgorithmHistory& rhs ) {
    return rhs.dump( lhs, false, 0 );
  }
};
