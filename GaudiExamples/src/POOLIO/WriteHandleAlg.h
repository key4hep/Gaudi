#ifndef HANDLEWRITE_ALG_H
#define HANDLEWRITE_ALG_H


#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/DataObjectHandle.h"

// Event Model related classes
#include "Event.h"
#include "MyTrack.h"
#include "Counter.h"

using namespace Gaudi::Examples;

//------------------------------------------------------------------------------

class WriteHandleAlg : public GaudiAlgorithm {

    friend class AlgFactory<WriteHandleAlg> ;
   
  public:

    /// the execution of the algorithm 
    virtual StatusCode execute  () ; // the execution of the algorithm 
    /// Its initialization
    virtual StatusCode initialize();
    /// the finalization of the algorithm 
    virtual StatusCode finalize () ; // the finalization of the algorithm 
    
  protected:

    WriteHandleAlg 
    ( const std::string& name , // the algorithm instance name 
      ISvcLocator*       pSvc ); // the Service Locator 

    /// virtual & protected desctrustor 
    virtual ~WriteHandleAlg() {}     // virtual & protected desctrustor

  private:

    /// the default constructor is disabled 
    WriteHandleAlg () ;                              // no default constructor
    /// the copy constructor is disabled
    WriteHandleAlg            ( const WriteHandleAlg& ) ; // no copy constructor 
    /// the assignement operator is disabled
    WriteHandleAlg& operator= ( const WriteHandleAlg& ) ; // no assignement

    DataObjectHandle<Collision>* m_output_handle;
    
    std::string m_output_name;
    
    bool m_useHandle;

  };

#endif