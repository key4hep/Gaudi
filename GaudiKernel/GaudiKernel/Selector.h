//====================================================================
//  NTuple name space: Selector class definition
//--------------------------------------------------------------------
//
//  Package    : Gaudi/NTupleSvc ( The LHCb Offline System)
//  Author     : M.Frank
//
//  +---------+----------------------------------------------+--------+
//  |    Date |                 Comment                      | Who    |
//  +---------+----------------------------------------------+--------+
//  | 10/10/00| Initial version.                             | MF     |
//  +---------+----------------------------------------------+--------+
//====================================================================
#ifndef GAUDI_NTUPLESVC_SELECTOR_H
#define GAUDI_NTUPLESVC_SELECTOR_H 1

// Framework include files
#include "GaudiKernel/SelectStatement.h"

// Forward declarations
namespace NTuple    {
  class Tuple;
}

namespace NTuple    {

  /** NTuple Selector class.
  */
  class GAUDI_API Selector : public SelectStatement {
  protected:
    /// reference to parent interface
    IInterface* m_parent;
    /// Boolean to indicate need for initialization
    bool        m_firstCall;
    /// StatusCode indication initialization result
    StatusCode  m_status;
  public:
    /// Standard constructor
    Selector(IInterface* svc) : m_parent(svc), m_firstCall(true)
    {
    }
    /// Standard Destructor
    virtual ~Selector() {
    }
    /// Check for first call
    bool firstCall()  const   {
      return m_firstCall;
    }
    /// Access initialization status
    StatusCode initResult()  const    {
      return m_status;
    }
    /// Default callback from interface
    virtual bool operator()(void* nt);
    /// Specialized callback for NTuples
    virtual bool operator()(NTuple::Tuple* nt);
    /// Selector Initialization
    virtual StatusCode initialize(NTuple::Tuple* nt);
  };
}
#endif // GAUDI_NTUPLESVC_SELECTOR_H
