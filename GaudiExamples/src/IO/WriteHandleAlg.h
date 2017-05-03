#ifndef HANDLEWRITE_ALG_H
#define HANDLEWRITE_ALG_H


#include "GaudiAlg/GaudiAlgorithm.h"

#include "GaudiKernel/DataObjectHandle.h"

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"
#include "GaudiExamples/Counter.h"

using namespace Gaudi::Examples;

//------------------------------------------------------------------------------

class WriteHandleAlg : public GaudiAlgorithm {

    friend class AlgFactory<WriteHandleAlg> ;

  public:
    bool isClonable() const override { return true; }

    /// the execution of the algorithm
    StatusCode execute  () override ; // the execution of the algorithm

  protected:

    WriteHandleAlg
    ( const std::string& name , // the algorithm instance name
      ISvcLocator*       pSvc ); // the Service Locator

    /// virtual & protected desctrustor
    ~WriteHandleAlg() override = default;

  private:

    /// the default constructor is disabled
    WriteHandleAlg () ;                              // no default constructor
    /// the copy constructor is disabled
    WriteHandleAlg            ( const WriteHandleAlg& ) ; // no copy constructor
    /// the assignement operator is disabled
    WriteHandleAlg& operator= ( const WriteHandleAlg& ) ; // no assignement

    DataObjectHandle<Collision> m_output_handle;

    bool m_useHandle;

  };

#endif
