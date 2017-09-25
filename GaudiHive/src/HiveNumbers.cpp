// Framework include files
#include "HiveNumbers.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/SmartIF.h"

namespace
{
  constexpr double HIVENUMBERS_BUFFER_SIZE = 1000;
}

HiveRndm::HiveNumbersMutex HiveRndm::HiveNumbers::m_genMutex;

// Standard constructor
HiveRndm::HiveNumbers::HiveNumbers() : m_buffer_index( 0 ), m_buffer_size( HIVENUMBERS_BUFFER_SIZE ), m_generator( 0 )
{
  m_buffer.reserve( m_buffer_size );
}

// Copy constructor
HiveRndm::HiveNumbers::HiveNumbers( const HiveRndm::HiveNumbers& copy )
    : m_buffer_index( 0 ), m_buffer_size( HIVENUMBERS_BUFFER_SIZE ), m_generator( copy.m_generator )
{
  m_buffer.reserve( m_buffer_size );
  if ( 0 != m_generator ) {
    m_generator->addRef();
  }
}

// Construct and initialize the generator
HiveRndm::HiveNumbers::HiveNumbers( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par )
    : m_buffer_index( 0 ), m_buffer_size( HIVENUMBERS_BUFFER_SIZE ), m_generator( 0 )
{
  m_buffer.reserve( m_buffer_size );
  StatusCode status = initialize( svc, par );
  if ( !status.isSuccess() ) {
    throw GaudiException( "Initialization failed !", "HiveRndm::HiveNumbers", status );
  }
}

// Standard destructor
HiveRndm::HiveNumbers::~HiveNumbers() { finalize().ignore(); }

// Initialize the generator
StatusCode HiveRndm::HiveNumbers::initialize( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par )
{
  if ( svc.isValid() && 0 == m_generator ) {
    /// @FIXME: this is a hack, but I do not have the time to review the
    ///         correct constantness of all the methods
    return const_cast<IRndmGenSvc*>( svc.get() )->generator( par, m_generator );
  }
  return StatusCode::FAILURE;
}

// Initialize the generator
StatusCode HiveRndm::HiveNumbers::finalize()
{
  if ( 0 != m_generator ) {
    m_generator->finalize().ignore();
    m_generator->release();
    m_generator = 0;
  }
  return StatusCode::SUCCESS;
}

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
// Construct and initialize the generator
HiveRndm::HiveNumbers::HiveNumbers( IRndmGenSvc* svc, const IRndmGen::Param& par )
    : m_buffer_index( 0 ), m_buffer_size( HIVENUMBERS_BUFFER_SIZE ), m_generator( 0 )
{
  StatusCode status = initialize( svc, par );
  if ( !status.isSuccess() ) {
    throw GaudiException( "Initialization failed !", "HiveRndm::HiveNumbers", status );
  }
}
#endif
// Initialize the generator
StatusCode HiveRndm::HiveNumbers::initialize( IRndmGenSvc* svc, const IRndmGen::Param& par )
{
  return initialize( SmartIF<IRndmGenSvc>( svc ), par );
}
