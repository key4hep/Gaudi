// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiAlg/GaudiAlg/Sequencer.h,v 1.4 2008/06/02 14:22:04 marcocle Exp $
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
class GAUDI_API Sequencer: public Algorithm {
 public:

    /**
     ** Constructor(s)
     **/
    Sequencer( const std::string& name, // The path object's name
	           ISvcLocator* svcloc      // A pointer to a service location service
              );

    /**
     ** Destructor
     **/
    virtual ~Sequencer( );

    /*****************************
     ** Public Function Members **
     *****************************/

    /**
     ** Initialization of a sequencer. Typically things like histogram creation,
     ** setting up of data structures etc, should be done here. If a sequence
     ** has properties specified in the job options file, they will be set to
     ** the requested values BEFORE the initialize() method is invoked.
     **/
    virtual StatusCode initialize( );

    /**
     ** Sequencer Reinitialization.
     **/
    virtual StatusCode reinitialize( );

    /**
     ** Sequencer finalization.
     **/
    virtual StatusCode start( );

    /**
     ** The actions to be performed by the sequencer on an event. This method
     ** is invoked once per event.
     **/
    virtual StatusCode execute( );

    /**
     ** Sequencer finalization.
     **/
    virtual StatusCode stop( );

    /**
     ** Sequencer finalization.
     **/
    virtual StatusCode finalize( );

    /**
     ** Sequencer beginRun.
     **/
    virtual StatusCode beginRun( );

    /**
     ** Sequencer endRun.
     **/
    virtual StatusCode endRun( );

    /**
     ** Reset the Sequencer executed state for the current event.
     **/
    void resetExecuted( );

    /**
     ** Was the branch filter passed for the last event?
     **/
    virtual bool branchFilterPassed( ) const;

    /**
     ** Set the branch filter passed flag for the last event
     **/
    virtual StatusCode setBranchFilterPassed( bool state );

    /**
     ** Has the StopOverride mode been set?
     **/
    virtual bool isStopOverride( ) const;

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
     StatusCode createAndAppend(
	    const std::string& type,  // The concrete algorithm class of the algorithm
	    const std::string& name,  // The name to be given to the algorithm
	    Algorithm*& pAlgorithm    // Set to point to the newly created algorithm object
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
     StatusCode createAndAppendToBranch(
	    const std::string& type,  // The concrete algorithm class of the algorithm
	    const std::string& name,  // The name to be given to the algorithm
	    Algorithm*& pAlgorithm    // Set to point to the newly created algorithm object
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
    std::vector<Algorithm*>* branchAlgorithms( ) const;

     /// Decode Member Name list
     StatusCode decodeMemberNames( );

     /// "Members" property handler
     void       membershipHandler( Property& theProp );

     /// Decode branch member name list
     StatusCode decodeBranchMemberNames( );

     /// "BranchMembers" property handler
     void       branchMembershipHandler( Property& theProp );

protected:

    /**
     ** Append an algorithm to the sequencer.
     **/
    StatusCode append( Algorithm* pAlgorithm,
                       std::vector<Algorithm*>* theAlgs );

    /**
     ** Create a algorithm and append it to the sequencer. A call to this method
     ** creates a child algorithm object. Note that the returned pointer is
     ** to Algorithm (as opposed to IAlgorithm), and thus the methods of
     ** IProperty are also available for the direct setting of the algorithm's
     ** properties. Using this mechanism instead of creating algorithms
     ** directly via the new operator is preferred since then the framework
     ** may take care of all of the necessary book-keeping.
     **/
     StatusCode createAndAppend(
	    const std::string& type,  // The concrete algorithm class of the algorithm
	    const std::string& name,  // The name to be given to the algorithm
	    Algorithm*& pAlgorithm,    // Set to point to the newly created algorithm object
	    std::vector<Algorithm*>* theAlgs
        );

    /**
     ** Decode algorithm names, creating or appending algorithms as appropriate
     **/
    StatusCode decodeNames( StringArrayProperty& theNames,
                            std::vector<Algorithm*>* theAlgs,
                            std::vector<bool>& theLogic );

    /**
     ** Execute the members in the specified list
     **/
    StatusCode execute( std::vector<Algorithm*>* theAlgs,
                        std::vector<bool>& theLogic,
                        Algorithm*& lastAlgorithm, unsigned int first = 0 );

    /**
     ** Execute member algorithm
     **/
    StatusCode executeMember( Algorithm* theAlgorithm );

    /**
     ** Remove the specified algorithm from the sequencer
     **/

    StatusCode remove( const std::string& algname, std::vector<Algorithm*>* theAlgs );

private:

    /******************************
     ** Private Function Members **
     ******************************/

    /**
     ** Private Copy constructor: NO COPY ALLOWED
     **/
    Sequencer( const Sequencer& a );

    /**
     ** Private assignment operator: NO ASSIGNMENT ALLOWED
     **/
    Sequencer& operator=( const Sequencer& rhs );

    /**************************
     ** Private Data Members **
     **************************/

    StringArrayProperty m_names;             // Member names
    std::vector<bool> m_isInverted;          // Member logic inverted list
    StringArrayProperty m_branchNames;       // Branch Member names
    std::vector<Algorithm*>* m_branchAlgs;   // Branch algorithms
    std::vector<bool> m_isBranchInverted;    // Branch Member logic inverted list
    BooleanProperty m_stopOverride;          // Stop on filter failure Override flag
    bool m_branchFilterPassed;               // Branch filter passed flag
};

#endif //ALGORITHM_SEQUENCER_H

