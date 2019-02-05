#ifndef GAUDIHIVE_HIVENUMBERS_H
#define GAUDIHIVE_HIVENUMBERS_H

// Framework include files
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/SmartIF.h"

// STL includes
#include <memory>
#include <vector>

#include "tbb/spin_rw_mutex.h"

// Forward declarations
class IRndmGen;
class IRndmGenSvc;

/*
 * This is a first solution to the problem of the thread safe random generation.
 * It is locking, but the locking is "diluted" by a caching mechanism of the
 * random numbers.
 */

namespace HiveRndm {

  typedef tbb::spin_rw_mutex_v3 HiveNumbersMutex;

  class GAUDI_API HiveNumbers {
  private:
    unsigned int            m_buffer_index;
    const unsigned int      m_buffer_size;
    std::vector<double>     m_buffer;
    static HiveNumbersMutex m_genMutex;

  protected:
    /// Pointer to random number generator
    IRndmGen* m_generator;

  public:
    /// Standard constructor
    HiveNumbers();
    /// Copy constructor
    HiveNumbers( const HiveNumbers& copy );
    /// Initializing constructor
    HiveNumbers( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par );
    /// Standard destructor
    virtual ~HiveNumbers();
    /// Initialization
    virtual StatusCode initialize( const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par );
#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
    /// Initializing constructor
    HiveNumbers( IRndmGenSvc* svc, const IRndmGen::Param& par );
    /// Initialization
    virtual StatusCode initialize( IRndmGenSvc* svc, const IRndmGen::Param& par );
#endif
    /// Finalization
    virtual StatusCode finalize();
    /// Check if the number supply is possible
    operator bool() const { return m_generator != 0; }
    /// Operator () for the use within STL
    double operator()() { return this->shoot(); }
    /// Pop a new number from the buffer
    double pop() { return this->shoot(); }
    /// Pop a new number from the buffer
    double shoot() {
      if ( 0 != m_generator ) {
        if ( m_buffer_index == 0 ) { // we are out of numbers
          this->shootArray( m_buffer, m_buffer_size );
          m_buffer_index = m_buffer_size - 1;
        }
        const double number = m_buffer[m_buffer_index];
        m_buffer_index--;
        return number;
      }
      return -1;
    }
    /// Pop a new number from the buffer
    StatusCode shootArray( std::vector<double>& array, long num, long start = 0 ) {
      if ( 0 != m_generator ) {
        StatusCode status;
        {
          HiveNumbersMutex::scoped_lock lock( m_genMutex );
          status = m_generator->shootArray( array, num, start );
        }
        return status;
      }
      return StatusCode::FAILURE;
    }
  };

} // namespace HiveRndm

#endif // GAUDIHIVE_HIVENumbers_H
