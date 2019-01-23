#include "ContextEventCounter.h"

// ----------------------------------------------------------------------------
// Implementation file for class: ContextEventCounterPtr
//
// 27/10/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT( ContextEventCounterPtr )
DECLARE_COMPONENT( ContextEventCounterData )

// ============================================================================
// Main execution
// ============================================================================
StatusCode ContextEventCounterPtr::execute()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  if ( !m_ctxtSpecCounter ) {
    m_ctxtSpecCounter = new int( 1 );
  } else {
    ++( *m_ctxtSpecCounter );
  }
  debug() << "Context " << Gaudi::Hive::currentContextId() << " " << m_ctxtSpecCounter.get() << " -> "
          << *m_ctxtSpecCounter << endmsg;

  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode ContextEventCounterPtr::finalize()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  debug() << "Partial counts:" << endmsg;

  info() << "Total count of events: "
         << m_ctxtSpecCounter.accumulate(
                [this]( const int* p ) {
                  const int r = ( p ? *p : 0 );
                  // print partial counts
                  this->debug() << " " << p << " -> " << r << endmsg;
                  return r;
                },
                0 )
         << endmsg;

  m_ctxtSpecCounter.deleteAll();

  return Algorithm::finalize(); // must be called after all other actions
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode ContextEventCounterData::execute()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  ++m_ctxtSpecCounter;
  debug() << "Context " << Gaudi::Hive::currentContextId() << " " << m_ctxtSpecCounter << endmsg;

  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode ContextEventCounterData::finalize()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Partial counts:" << endmsg;
    m_ctxtSpecCounter.for_each( [this]( const int i ) { this->debug() << " " << i << endmsg; } );
  }

  info() << "Total count of events: " << m_ctxtSpecCounter.accumulate( 0 ) << endmsg;

  return Algorithm::finalize(); // must be called after all other actions
}

// ============================================================================
