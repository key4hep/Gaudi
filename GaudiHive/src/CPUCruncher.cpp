#include "CPUCruncher.h"

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
      always ()  << "This does never happen, but it's necessary too fool aggressive compiler optimisations!"<< endmsg ;
  
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::execute  ()  // the execution of the algorithm 
{
  
  // setup the runtime with a random number

  Rndm::Numbers gauss ( randSvc() , 
                        Rndm::Gauss ( m_avg_runtime , m_var_runtime ) ) ;
  
  const double runtime = fabs(gauss());
  
//const double runtime = 0.4; 
  always ()  << "Runtime will be: "<< runtime << endmsg;
  
  
  findPrimes(runtime);
  
  return StatusCode::SUCCESS ;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::finalize () // the finalization of the algorithm 
{ 
  
  always ()  << "I ran." << endmsg;
  
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


  
