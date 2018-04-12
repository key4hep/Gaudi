#ifndef ALGORITHM_SEQUENCER_H
#define ALGORITHM_SEQUENCER_H

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"

class MsgStream;

/**
 ** ClassName: Sequencer
 **
 ** Description: A Sequencer is essentially a list of Algorithms and is responsible
 **              for their management. Note that Sequences may themselves contain other
 **              Sequences. The default execute( ) implementation loops over the
 **              members of the sequence, calling their execute( ) methods. However, this
 **              can be modified if a member is disabled, has already been executed, or a
 **              member indicates that it's filter fails. The the former two cases the
 **             execution of the member is bypassed. In the latter case, the loop is
 **             terminated and the Sequencer assumes the same filtered state as the
 **             last member.
 **/
class GAUDI_API Sequencer : public Algorithm
{
public:
  /**
   ** Constructor(s)
   **/
  Sequencer( const std::string& name,  // The path object's name
             ISvcLocator*       svcloc // A pointer to a service location service
             );

  /*****************************
   ** Public Function Members **
   *****************************/

  /**
   ** Initialization of a sequencer. Typically things like histogram creation,
   ** setting up of data structures etc, should be done here. If a sequence
   ** has properties specified in the job options file, they will be set to
   ** the requested values BEFORE the initialize() method is invoked.
   **/
  StatusCode initialize() override;

  /**
   ** Sequencer Reinitialization.
   **/
  StatusCode reinitialize() override;

  /**
   ** Sequencer finalization.
   **/
  StatusCode start() override;

  /**
   ** The actions to be performed by the sequencer on an event. This method
   ** is invoked once per event.
   **/
  StatusCode execute() override;

  /**
   ** Sequencer finalization.
   **/
  StatusCode stop() override;

  /**
   ** Sequencer finalization.
   **/
  StatusCode finalize() override;

  /**
   ** Reset the Sequencer executed state for the current event.
   **/
  void resetExecuted() override;

  /**
   ** additional interface methods
   **/

  /**
   ** Identify as a Sequence
   **/
  bool isSequence() const override final { return true; }

  /**
   ** Was the branch filter passed for the last event?
   **/
  virtual bool branchFilterPassed() const;

  /**
   ** Set the branch filter passed flag for the last event
   **/
  virtual StatusCode setBranchFilterPassed( bool state );

  /**
   ** Has the StopOverride mode been set?
   **/
  virtual bool isStopOverride() const;

  /**
   ** Append an algorithm to the sequencer.
   **/
  StatusCode append( Algorithm* pAlgorithm );

  /**
   ** Append an algorithm to the sequencer branch
   **/
  StatusCode appendToBranch( Algorithm* pAlgorithm );

  /**
   ** Create a algorithm and append it to the sequencer. A call to this method
   ** creates a child algorithm object. Note that the returned pointer is
   ** to Algorithm (as opposed to IAlgorithm), and thus the methods of
   ** IProperty are also available for the direct setting of the algorithm's
   ** properties. Using this mechanism instead of creating algorithms
   ** directly via the new operator is preferred since then the framework
   ** may take care of all of the necessary book-keeping.
   **/
  StatusCode createAndAppend( const std::string& type,      // The concrete algorithm class of the algorithm
                              const std::string& name,      // The name to be given to the algorithm
                              Algorithm*&        pAlgorithm // Set to point to the newly created algorithm object
                              );

  /**
   ** Create a algorithm and append it to the sequencer branch. A call to this method
   ** creates a child algorithm object. Note that the returned pointer is
   ** to Algorithm (as opposed to IAlgorithm), and thus the methods of
   ** IProperty are also available for the direct setting of the algorithm's
   ** properties. Using this mechanism instead of creating algorithms
   ** directly via the new operator is preferred since then the framework
   ** may take care of all of the necessary book-keeping.
   **/
  StatusCode createAndAppendToBranch( const std::string& type, // The concrete algorithm class of the algorithm
                                      const std::string& name, // The name to be given to the algorithm
                                      Algorithm*& pAlgorithm   // Set to point to the newly created algorithm object
                                      );

  /**
   ** Remove the specified algorithm from the sequencer
   **/
  StatusCode remove( Algorithm* pAlgorithm );
  StatusCode remove( const std::string& name );
  StatusCode removeFromBranch( Algorithm* pAlgorithm );
  StatusCode removeFromBranch( const std::string& name );

  /**
   ** List of branch algorithms. These are the algorithms
   ** that would get executed if a filter algorithm indicated
   ** a failure. The branch is located within the main sequence
   ** by the first element, which is the filter algorithm.
   **/
  const std::vector<Algorithm*>& branchAlgorithms() const;
  std::vector<Algorithm*>&       branchAlgorithms();

  /// Decode Member Name list
  StatusCode decodeMemberNames();

  /// Decode branch member name list
  StatusCode decodeBranchMemberNames();

protected:
  /**
   ** Append an algorithm to the sequencer.
   **/
  StatusCode append( Algorithm* pAlgorithm, std::vector<Algorithm*>& theAlgs );

  /**
   ** Create a algorithm and append it to the sequencer. A call to this method
   ** creates a child algorithm object. Note that the returned pointer is
   ** to Algorithm (as opposed to IAlgorithm), and thus the methods of
   ** IProperty are also available for the direct setting of the algorithm's
   ** properties. Using this mechanism instead of creating algorithms
   ** directly via the new operator is preferred since then the framework
   ** may take care of all of the necessary book-keeping.
   **/
  StatusCode createAndAppend( const std::string&       type,       // The concrete algorithm class of the algorithm
                              const std::string&       name,       // The name to be given to the algorithm
                              Algorithm*&              pAlgorithm, // Set to point to the newly created algorithm object
                              std::vector<Algorithm*>& theAlgs );

  /**
   ** Decode algorithm names, creating or appending algorithms as appropriate
   **/
  StatusCode decodeNames( Gaudi::Property<std::vector<std::string>>& theNames, std::vector<Algorithm*>& theAlgs,
                          std::vector<bool>& theLogic );

  /**
   ** Execute the members in the specified list
   **/
  StatusCode execute( const std::vector<Algorithm*>& theAlgs, std::vector<bool>& theLogic, Algorithm*& lastAlgorithm,
                      unsigned int first = 0 );

  /**
   ** Execute member algorithm
   **/
  StatusCode executeMember( Algorithm* theAlgorithm );

  /**
   ** Remove the specified algorithm from the sequencer
   **/

  StatusCode remove( const std::string& algname, std::vector<Algorithm*>& theAlgs );

  // NO COPY / ASSIGNMENT  ALLOWED
  Sequencer( const Sequencer& a ) = delete;
  Sequencer& operator=( const Sequencer& rhs ) = delete;

public:
  /// Produce string represention of the control flow expression.
  std::ostream& toControlFlowExpression( std::ostream& os ) const override;

private:
  /**************************
   ** Private Data Members **
   **************************/

  Gaudi::Property<std::vector<std::string>> m_names{this, "Members", {}, "member names"};
  Gaudi::Property<std::vector<std::string>> m_branchNames{this, "BranchMembers", {}, "branch member names"};
  Gaudi::Property<bool> m_stopOverride{this, "StopOverride", false, "stop on filter failure override"};

  std::vector<bool>       m_isInverted;       // Member logic inverted list
  std::vector<Algorithm*> m_branchAlgs;       // Branch algorithms
  std::vector<bool>       m_isBranchInverted; // Branch Member logic inverted list

  bool m_branchFilterPassed = false; // Branch filter passed flag
};

#endif // ALGORITHM_SEQUENCER_H
