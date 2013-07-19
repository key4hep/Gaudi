// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataStoreLeaves.h"
#include "GaudiKernel/MsgStream.h"
#include "InputCopyStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT(InputCopyStream)

// Standard Constructor
InputCopyStream::InputCopyStream(const std::string& name, ISvcLocator* pSvcLocator)
    : OutputStream  ( name, pSvcLocator),
      m_leavesTool  ( NULL             ),
      m_tesVetoList (                  )
{
  m_doPreLoad      = false;
  m_doPreLoadOpt   = false;
  declareProperty( "TESVetoList", m_tesVetoList );
}

// Standard Destructor
InputCopyStream::~InputCopyStream() { }

// Initialize the instance.
StatusCode InputCopyStream::initialize()
{
  StatusCode sc = OutputStream::initialize();
  if ( sc.isFailure() ) return sc;

  sc = toolSvc()->retrieveTool("DataSvcFileEntriesTool", "InputCopyStreamTool",
                               m_leavesTool);

  return sc;
}

// Finalize the instance.
StatusCode InputCopyStream::finalize()
{
  toolSvc()->releaseTool(m_leavesTool).ignore();
  m_leavesTool = NULL;
  return OutputStream::finalize();
}

/// Collect all objects to be written to the output stream
StatusCode InputCopyStream::collectObjects()
{
  // Get the objects in the same file as the root node
  try
  {
    // Get all the leaves on the input stream
    const IDataStoreLeaves::LeavesList & leaves = m_leavesTool->leaves();

    // Do we need to veto anything ?
    if ( UNLIKELY( !m_tesVetoList.empty() ) )
    {
      // Veto out locations
      IDataStoreLeaves::LeavesList filteredLeaves;
      filteredLeaves.reserve( leaves.size() );
      for ( IDataStoreLeaves::LeavesList::const_iterator iL = leaves.begin();
            iL != leaves.end(); ++iL )
      {
        if ( *iL && (*iL)->registry() )
        {
          if ( std::find( m_tesVetoList.begin(),
                          m_tesVetoList.end(),
                          (*iL)->registry()->identifier() ) == m_tesVetoList.end() )
          {
            filteredLeaves.push_back( (*iL) );
          }
        }
      }

      // saved the veto'ed list
      m_objects.assign( filteredLeaves.begin(), filteredLeaves.end() );

    }
    else
    {
      // no veto'ing, so save everything
      m_objects.assign( leaves.begin(), leaves.end() );
    }
  }
  catch ( const GaudiException & e )
  {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << e.message() << endmsg;
    return StatusCode::FAILURE;
  }

  // Collect the other objects from the transient store (mandatory and optional)
  return OutputStream::collectObjects();
}
