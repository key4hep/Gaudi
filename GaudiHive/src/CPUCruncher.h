
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/RegistryEntry.h"

#include <tbb/concurrent_hash_map.h>

//------------------------------------------------------------------------------

  /** @class CPUCruncher
   * 
   * A class that implements a search for prime numbers.
   * A lot of allocations are performed on purpose not to stress only the FP 
   * unit of the CPU.
   * 
   */
  class CPUCruncher : public GaudiAlgorithm 
  {

    friend class AlgFactory<CPUCruncher> ;

  public:

    typedef tbb::concurrent_hash_map<std::string,unsigned int> CHM;

    /// the execution of the algorithm 
    virtual StatusCode execute  () ; // the execution of the algorithm 
    /// Its initialization
    virtual StatusCode initialize();
    /// the finalization of the algorithm 
    virtual StatusCode finalize () ; // the finalization of the algorithm 
    /// Get the inputs
    virtual const std::vector<std::string> get_inputs();
    /// Get the outputs
    virtual const std::vector<std::string> get_outputs();
    
  protected:

    CPUCruncher 
    ( const std::string& name , // the algorithm instance name 
      ISvcLocator*       pSvc ); // the Service Locator 

    /// virtual & protected desctrustor 
    virtual ~CPUCruncher() {}     // virtual & protected desctrustor

  private:

    /// the default constructor is disabled 
    CPUCruncher () ;                              // no default constructor
    /// the copy constructor is disabled
    CPUCruncher            ( const CPUCruncher& ) ; // no copy constructor 
    /// the assignement operator is disabled
    CPUCruncher& operator= ( const CPUCruncher& ) ; // no assignement
    /// The CPU intensive function
    void findPrimes (const unsigned long int ) ; 

// We do not need this if we have a whiteboard    
//     /// putting data into the event store
//     long int write (DataObject* obj, const std::string& location){
//     	if (!m_event_context)
//     		return -1;
//       return m_event_context->m_registry->add(location,obj);
//     }
//     /// retrieving data from the event store 
//     template < typename T >    
//     T* read(const std::string& path){
//     	if (!m_event_context)
//     		return nullptr;
//       T* obj = dynamic_cast<T*>( m_event_context->m_registry->find(path)->object());
//       //      Assert(obj, "get():: No valid data at '" + path + "'");
//       return obj;
//     }
    /// Calibrate
    void calibrate();
    long unsigned int getNCaliIters(double);

    double m_avg_runtime ; //Avg Runtime
    double m_var_runtime ; //Variance of Runtime
    bool m_local_rndm_gen; //Decide what random number generation to use
    bool m_shortCalib;
    // To calib only once
    static std::vector<unsigned int>m_niters_vect;
    static std::vector<double> m_times_vect;

    // For the concurrency
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_outputs;    

    static CHM m_name_ncopies_map;

  };

