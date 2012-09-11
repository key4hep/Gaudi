#include "CPUCruncher.h"
#include "HiveNumbers.h"

DECLARE_ALGORITHM_FACTORY(CPUCruncher) 

//------------------------------------------------------------------------------  

CPUCruncher::CPUCruncher ( const std::string& name , // the algorithm instance name 
              ISvcLocator*       pSvc )
            : GaudiAlgorithm ( name , pSvc ) 
              , m_avg_runtime ( 1. )
              , m_var_runtime ( .01 )
    {
      // For Concurrent run
      declareProperty("Inputs", m_inputs, "List of required inputs");
      declareProperty("Outputs", m_outputs, "List of provided outputs");
      
      declareProperty ( "avgRuntime" , m_avg_runtime , "Average runtime of the module." ) ;
      declareProperty ( "varRuntime" , m_var_runtime , "Variance of the runtime of the module." ) ;     
    }  
  
void CPUCruncher::findPrimes (const double runtime)  { 
  
  MsgStream log(msgSvc(), name());

  // Limit the exercise in time
  const double start = std::clock ();
  
  // Flag to trigger the allocation
  bool is_prime;
  
  // Let's prepare the material for the allocations
  unsigned int primes_size=1;
  unsigned long* primes = new unsigned long[primes_size];
  primes[0]=2;

  unsigned long i =2;

  // Loop on numbers
  while ( (std::clock () - start)/CLOCKS_PER_SEC < runtime ){
    // Once at max, it returns to 0
    i+=1;    
        
    // Check if it can be divided by the smaller ones
    is_prime = true;
    for (unsigned long j=2;j<i && is_prime;++j){
      if (i%j == 0){     
        is_prime = false;
      }      
    }// end loop on numbers < than tested one
    if (is_prime){
      // copy the array of primes (INEFFICIENT ON PURPOSE!)
      unsigned int new_primes_size = 1 + primes_size;
      unsigned long* new_primes = new unsigned long[new_primes_size];
    
      for (unsigned int prime_index=0; prime_index<primes_size;prime_index++){
        new_primes[prime_index]=primes[prime_index];    
      }
      // attach the last prime
      new_primes[primes_size]=i;
      
      // Update primes array
      delete[] primes;
      primes = new_primes;        
      primes_size=new_primes_size;
    } // end is prime
            
  //   for (unsigned int prime_index=0; prime_index<primes_size;prime_index++) 
  //     always ()  << primes[prime_index] << " is prime." << endmsg;
  
  } // end of while loop
  
  
  // Fool Compiler optimisations:
  for (unsigned int prime_index=0; prime_index<primes_size;prime_index++)
    if (primes[prime_index] == 4)
      log << "This does never happen, but it's necessary too fool aggressive compiler optimisations!"<< endmsg ;
  
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::execute  ()  // the execution of the algorithm 
{

  MsgStream logstream(msgSvc(), name());


  /* This will disappear with a thread safe random number generator svc
   * Use box mueller to generate gaussian randoms
   * The quality is not good for in depth study given that the generator is a
   * linear congruent.
   * Throw away basically a free number: we are in a cpu cruncher after all.
   * The seed is taken from the clock, but we could assign a seed per module to
   * ensure reproducibility.
   *
   * This is not an overkill but rather an exercise towards a thread safe
   * random number generation.
   */
  /*
  auto getGausRandom = [] (double mean, double sigma) -> double {

    unsigned int seed = std::clock();

    auto getUnifRandom = [] (unsigned int & seed) ->double {
      // from numerical recipies
      constexpr unsigned int m = 232;
      constexpr unsigned int a = 1664525;
      constexpr unsigned int c = 1013904223;
      seed = (a * seed + c) % m;
      const double unif = double(seed) / m;
      return unif;
      };

    const double unif1 = getUnifRandom(seed);
    const double unif2 = getUnifRandom(seed);
    const double normal = sqrt(-2.*log(unif1))*cos(2*M_PI*unif2);
    return normal*sigma + mean;
    };
  const double runtime = fabs(getGausRandom( m_avg_runtime , m_var_runtime ));
  //End Of temp block
  */

  HiveRndm::HiveNumbers rndmgaus(randSvc(), Rndm::Gauss( m_avg_runtime , m_var_runtime ));
  const double runtime = std::fabs(rndmgaus());

  logstream  << MSG::ALWAYS << "Runtime will be: "<< runtime << endmsg;
  logstream  << "Event " <<  getContext()->m_evt_num
		     << " on pthreadID " << getContext()->m_thread_id  << endmsg;
  
  // get products from the event
  for (std::string& input : m_inputs){
    get<DataObject>(input);
  }

  findPrimes(runtime);

  // write products to the event
  for (std::string& output: m_outputs){
    put(new DataObject(), output);
  }

  return StatusCode::SUCCESS ;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::finalize () // the finalization of the algorithm 
{ 
  MsgStream log(msgSvc(), name());
  log  << MSG::ALWAYS << "I ran." << endmsg;
  
  return GaudiAlgorithm::finalize () ;
}

//------------------------------------------------------------------------------

const std::vector<std::string>
CPUCruncher::get_inputs()
{
  return m_inputs;
}

//------------------------------------------------------------------------------

const std::vector<std::string>
CPUCruncher::get_outputs()
{
  return m_outputs;
}

//------------------------------------------------------------------------------


  
