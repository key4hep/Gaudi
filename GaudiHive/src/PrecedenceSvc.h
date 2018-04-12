#ifndef GAUDIHIVE_PRECEDENCESVC_H_
#define GAUDIHIVE_PRECEDENCESVC_H_

#include "IPrecedenceSvc.h"
#include "PrecedenceRulesGraph.h"

// Framework include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/Service.h"

#include <boost/filesystem.hpp>

/** @class PrecedenceSvc PrecedenceSvc.h GaudiHive/PrecedenceSvc.h
  *
  * @brief A service to resolve the task execution precedence.
  *
  * @author Illya Shapoval
  * @created Aug 7, 2017
  */

class PrecedenceSvc : public extends<Service, IPrecedenceSvc>
{

public:
  /// Constructor
  PrecedenceSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~PrecedenceSvc() = default;

  /// Initialize
  StatusCode initialize() override;

  /// Finalize
  StatusCode finalize() override;

  /// Infer the precedence effect caused by an execution flow event
  StatusCode iterate( EventSlot&, const Cause& ) override;

  /// Simulate execution flow
  StatusCode simulate( EventSlot& ) const override;

  /// Check if the root CF decision is resolved
  bool CFRulesResolved( EventSlot& ) const override;

  /// Get priority of an algorithm
  uint getPriority( const std::string& name ) const override
  {
    return (int)m_PRGraph.getAlgorithmNode( name )->getRank();
  }

  /// Check if a task is CPU-blocking
  bool isBlocking( const std::string& name ) const override { return m_PRGraph.getAlgorithmNode( name )->isIOBound(); }

  /// Dump precedence rules
  void              dumpControlFlow() const override;
  void              dumpDataFlow() const override;
  const std::string printState( EventSlot& ) const override;

  /// Dump precedence rules (available only in DEBUG mode, and must be enabled
  /// with the corresponding service property)
  void dumpPrecedenceRules( EventSlot& ) override;
  /// Dump precedence trace (available only in DEBUG mode, and must be enabled
  /// with the corresponding service property)
  void dumpPrecedenceTrace( EventSlot& ) override;

  /// Precedence rules accessor
  const concurrency::PrecedenceRulesGraph* getRules() const { return &m_PRGraph; };

private:
  StatusCode assembleCFRules( Algorithm*, const std::string&, unsigned int recursionDepth = 0 );

private:
  /// A shortcut to the algorithm resource pool
  SmartIF<IAlgResourcePool> m_algResourcePool;
  /// Graph of precedence rules
  concurrency::PrecedenceRulesGraph m_PRGraph{"PrecedenceRulesGraph", serviceLocator()};
  /// Scheduling strategy
  Gaudi::Property<std::string> m_mode{this, "TaskPriorityRule", "", "Task avalanche induction strategy."};
  /// Scheduling strategy
  Gaudi::Property<bool> m_ignoreDFRules{this, "IgnoreDFRules", false, "Ignore the data flow rules."};
  /// Precedence analysis facilities
  boost::filesystem::path m_dumpDirName{
      boost::filesystem::unique_path( boost::filesystem::path( "precedence.analysis.%%%%" ) )};
  Gaudi::Property<bool> m_dumpPrecTrace{this, "DumpPrecedenceTrace", false,
                                        "Dump task precedence traces for each event."
                                        "The service must be in DEBUG mode for this switch "
                                        "to have effect."};
  Gaudi::Property<std::string> m_dumpPrecTraceFile{
      this, "PrecedenceTraceFile", "", "Override default name of the GRAPHML trace file. NOTE: if more than "
                                       "1 event is processed, the setting forces creation of a single file "
                                       "with cumulative precedence trace."};
  Gaudi::Property<bool> m_dumpPrecRules{this, "DumpPrecedenceRules", false, "Dump task precedence rules. The service "
                                                                            "must be in DEBUG mode for this switch "
                                                                            "to have effect."};
  Gaudi::Property<std::string> m_dumpPrecRulesFile{this, "PrecedenceRulesFile", "",
                                                   "Override default name of the GRAPHML precedence rules file."};
};

#endif /* GAUDIHIVE_PRECEDENCESVC_H_ */
