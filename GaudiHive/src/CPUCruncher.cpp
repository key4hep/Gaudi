#include "CPUCruncher.h"
#include "HiveNumbers.h"
#include <ctime>
#include <sys/resource.h>
#include <sys/times.h>

#include <tbb/tick_count.h>

std::vector<unsigned int> CPUCruncher::m_niters_vect;
std::vector<double> CPUCruncher::m_times_vect;


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
      declareProperty ( "localRndm", m_local_rndm_gen = true, "Decide if the local random generator is to be used");
      declareProperty ( "NIterationsVect", m_niters_vect , "Number of iterations for the calibration." ) ;
      declareProperty ( "NTimesVect", m_times_vect , "Number of seconds for the calibration." ) ;

    }  

StatusCode CPUCruncher::initialize(){
if (m_times_vect.size()==0)
  calibrate();
return StatusCode::SUCCESS ;
}

/*
Calibrate the crunching finding the right relation between max number to be searched and time spent.
The relation is a sqrt for times greater than 10^-4 seconds.
*/
void CPUCruncher::calibrate(){

  MsgStream log(msgSvc(), name());
  // Tunded once by hand to cover from 0 to 4
  m_niters_vect.push_back(0);
  m_niters_vect.push_back(500);
  m_niters_vect.push_back(600);
  m_niters_vect.push_back(700);
  m_niters_vect.push_back(800);
  m_niters_vect.push_back(1000);
  m_niters_vect.push_back(1300);
  m_niters_vect.push_back(1600);   
  m_niters_vect.push_back(2000);
  m_niters_vect.push_back(2300);
  m_niters_vect.push_back(2600);
  m_niters_vect.push_back(3000);
  m_niters_vect.push_back(3300);
  m_niters_vect.push_back(3500);
  m_niters_vect.push_back(3900);
  m_niters_vect.push_back(4200);
  m_niters_vect.push_back(5000);
  m_niters_vect.push_back(6000);
  m_niters_vect.push_back(8000);
  m_niters_vect.push_back(10000);
  m_niters_vect.push_back(12000);
  m_niters_vect.push_back(15000);
  m_niters_vect.push_back(17000);
  m_niters_vect.push_back(20000);
  m_niters_vect.push_back(25000);
  m_niters_vect.push_back(30000);
  m_niters_vect.push_back(35000);
  m_niters_vect.push_back(40000);
  m_niters_vect.push_back(60000);
  m_niters_vect.push_back(100000);
  m_times_vect.push_back(0);

  for (unsigned int i=1;i<m_niters_vect.size();++i){
   unsigned long niters=m_niters_vect[i];
   do{
    auto start_cali=tbb::tick_count::now();
    findPrimes(niters);
    auto stop_cali=tbb::tick_count::now();
    double deltat = (stop_cali-start_cali).seconds();   
    m_times_vect.push_back(deltat);
    log << MSG::INFO << "Calibration: # iters = " << niters << " => " << deltat << endmsg;
    } while(m_times_vect[i]<m_times_vect[i-1]); // make sure that they are monotonic
   }

}

unsigned long CPUCruncher::getNCaliIters(double runtime){
 
 int smaller_i=-1;
 for (auto& time:m_times_vect )
   if (time<runtime) smaller_i++;
 
 // Case 1: we are outside the interpolation range
 if (smaller_i==m_times_vect.size())
   return m_times_vect[smaller_i];

 // Case 2: we maeke a linear interpolation
 // y=mx+q
 double x0=m_times_vect[smaller_i];
 double x1=m_times_vect[smaller_i+1];
 double y0=m_niters_vect[smaller_i];
 double y1=m_niters_vect[smaller_i+1];
 double m=(y1-y0)/(x1-x0);
 double q=y0-m*x0;


 //always() << x0 << "<" << runtime << "<" << x1 << endmsg;

 return m * runtime + q ;
 }


void CPUCruncher::findPrimes (const unsigned long int n_iterations)  { 

  
  MsgStream log(msgSvc(), name());

  // Flag to trigger the allocation
  bool is_prime;
  
  // Let's prepare the material for the allocations
  unsigned int primes_size=1;
  unsigned long* primes = new unsigned long[primes_size];
  primes[0]=2;

  unsigned long i = 2;

  // Loop on numbers
  for (unsigned long int iiter=0;iiter<n_iterations;iiter++ ){
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
              
  } // end of while loop
  delete[] primes;
  
  // Fool Compiler optimisations:
  for (unsigned int prime_index=0; prime_index<primes_size;prime_index++)
    if (primes[prime_index] == 4)
      log << "This does never happen, but it's necessary too fool aggressive compiler optimisations!"<< endmsg ;
  
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::execute  ()  // the execution of the algorithm 
{

  MsgStream logstream(msgSvc(), name());

  float runtime;

  if (m_local_rndm_gen){
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
  runtime = fabs(getGausRandom( m_avg_runtime , m_var_runtime ));
  //End Of temp block
  }
  else{
  // Should be a member.
  HiveRndm::HiveNumbers rndmgaus(randSvc(), Rndm::Gauss( m_avg_runtime , m_var_runtime ));
  runtime = std::fabs(rndmgaus());
  }

  tbb::tick_count starttbb=tbb::tick_count::now();
  logstream  << MSG::INFO << "Runtime will be: "<< runtime << endmsg;
  logstream  << MSG::INFO << "Start event " <<  getContext()->m_evt_num
		     << " on pthreadID " << getContext()->m_thread_id << endmsg;
  
  for (std::string& input : m_inputs){
    read<DataObject>(input);
  }

  const unsigned long n_iters= getNCaliIters(runtime);
  findPrimes( n_iters );

  for (std::string& output: m_outputs){
    write(new DataObject(), output);
  }

  tbb::tick_count endtbb=tbb::tick_count::now();

  const double actualRuntime=(endtbb-starttbb).seconds();

  logstream << MSG::INFO << "Finish event " <<  getContext()->m_evt_num
		     << " on pthreadID " << getContext()->m_thread_id
		     << " in " << actualRuntime  << " seconds" << endmsg;

  logstream << MSG::INFO << "Timing: ExpectedRuntime= " << runtime 
                         << " ActualRuntime= " << actualRuntime 
                         << " Ratio= " << runtime/actualRuntime
                         << " Niters= " << n_iters << endmsg;


  return StatusCode::SUCCESS ;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::finalize () // the finalization of the algorithm 
{ 
  MsgStream log(msgSvc(), name());
  log  << MSG::INFO << "I ran with an average runtime of " << m_avg_runtime << endmsg;
  
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


  
