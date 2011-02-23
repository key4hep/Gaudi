// Dear emacs, this is -*- c++ -*-

const int MAX_THREADS(1023); //to check command line parameter
// Include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IClassManager.h"

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ThreadGaudi.h"

#include <cassert>
#include <iostream>

#include <pthread.h>
/**
 * Defines a global locking mechanism so we can see when all worker threads
 * are finished.
 */
int worker_done;
pthread_mutex_t mutex;
pthread_cond_t condition;

pthread_mutex_t coutmutex;

#include <sstream>
#define COUTTHREAD(_message_) { \
  pthread_mutex_lock(&coutmutex); \
  std::ostringstream _oss_; \
  _oss_<<_message_; \
  std::cout<<_oss_.str()<<std::endl; \
  pthread_mutex_unlock(&coutmutex); \
  }



/**
 * Defines the work that has to be done in each thread
 *
 * @param counter The number of times the same work has to be done.
 */

void* work (void* counter)
{
  int id_thread = *(static_cast<int*>(counter));
  assert( id_thread >= 0 );
  std::string threadStrID(getGaudiThreadIDfromID(id_thread));
  COUTTHREAD("++++ Thread : " << id_thread << " string = " << threadStrID) ;

  int m_evtMax = 0;

  // Get an instance of the Pesa application manager
  IInterface* m_pesaAppMgr = Gaudi::createApplicationMgr();

  // Get the EventLoopMgr name from the Application Manager
  std::string nameEventLoopMgr = "EventLoopMgr" ;

  std::string value ;
  SmartIF<IProperty> propMgr ( m_pesaAppMgr );
  if( !propMgr.isValid() ) {
    COUTTHREAD(" Fatal error while retrieving Gaudi PropertyMgr ")
  } else {
    StatusCode sc = propMgr->getProperty( "EventLoop", value );
    if( sc.isFailure() ) {
      COUTTHREAD(" Fatal error while retrieving Property EventLoop ")
    } else {
      nameEventLoopMgr.assign(value, value.find_first_of("\"")+1,
                                     value.find_last_of("\"")-value.find_first_of("\"")-1) ;
    }

    sc = propMgr->getProperty( "EvtMax", value );
    if( sc.isFailure() ) {
      COUTTHREAD(" Fatal error while retrieving Property EvtMax ")
    } else {
      m_evtMax = std::atoi(value.c_str()) ;
    }
  }

  nameEventLoopMgr = nameEventLoopMgr + threadStrID ;

  COUTTHREAD("---> Thread : " << id_thread << " - Name for EventLoopManager : " + nameEventLoopMgr)
  StatusCode sc ;
  IEventProcessor* m_processingMgr = 0 ;
  SmartIF<ISvcLocator> svcLoc( m_pesaAppMgr );
  if (svcLoc.isValid()) {
    sc = svcLoc->service( nameEventLoopMgr , m_processingMgr);
    if( !sc.isSuccess() )  {
      COUTTHREAD("FATAL Error retrieving Processing manager:")
      m_processingMgr = 0 ;
    }
  }

  for (int i = 0; i < m_evtMax; i++) {
    // ExecuteEvent from the application manager
    if ( 0 != m_processingMgr ) {
      SmartIF<IEventProcessor> processor(m_processingMgr);

      if ( processor.isValid() )    {
	COUTTHREAD(" ---> Executing WorkerThread---> " << id_thread)
	sc = processor->executeEvent(NULL);
	if( sc.isFailure() ) {
	  COUTTHREAD("Fatal error for executeEvent in the ApplicationMgr " << id_thread)
	}
      }
      else {
	COUTTHREAD("---> executeEvent ApplicationMgr : no valid event processor " << id_thread) ;
      }
    }
  }

  pthread_mutex_lock(&mutex);
  --worker_done;
  if (worker_done == 0) pthread_cond_signal(&condition);
  pthread_mutex_unlock(&mutex);
  return 0;
}

//--- Example main program
int main (int argc, char** argv)
{
  //initialises the mutex and the condition
  pthread_mutex_init (&mutex, 0); //fast mutex (or recursive?)
  pthread_mutex_init (&coutmutex, 0); //fast mutex (or recursive?)
  pthread_cond_init (&condition, 0);


  // Create an instance of an application manager
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty>     propMgr ( iface );
  SmartIF<IAppMgrUI>     appMgr  ( iface );
  SmartIF<IClassManager> dllMgr  ( iface );

  if( !appMgr.isValid() || !propMgr.isValid() || !dllMgr.isValid()) {
    std::cout << "Fatal error while creating the ApplicationMgr " << std::endl;
    return 1;
  }

  // check the arguments
  if ( argc < 2 ) {
    std::cout << "usage: " << argv[0] << "<JobOptions file> <number-of-threads=4>"
	      << std::endl;
    return 1;
  }

  // Get the input configuration file from arguments
  std:: string opts = (argc>1) ? argv[1] : "jobOptions.txt";

  // Get the number of worker threads from arguments
  std:: string s_nt = (argc>2) ? argv[2] : "4";

  // Set properties
  propMgr->setProperty( "JobOptionsPath", opts );
  propMgr->setProperty( "NoOfThreads", s_nt);
  propMgr->setProperty( "EvtSel",         "NONE" );
  propMgr->setProperty( "MessageSvcType", "MTMessageSvc" );
  if( opts.substr( opts.length() - 3, 3 ) == ".py" ) {
    std::cout << "Running with Python not supported" << std::endl;
    return 1;
  }

  //load MTMessageSvc library
  if (!(dllMgr->loadModule("GaudiMTExample")).isSuccess()) {
    std::cerr << "Can not load MTMessageSvc module (GaudiMTExample)" << std::endl;
    return 1;
  }

  StatusCode sc ;
  std::string v_nt ;
  int nt ;
  sc = propMgr->getProperty("NoOfThreads", v_nt);
  if( sc.isFailure() ) {
    std::cout << "Cannot get get number of worker threads" << std::endl;
    return 1;
  } else {
    nt = std::atoi(v_nt.c_str()) ;
    if ( nt <= 0 || nt > MAX_THREADS ) {
      std::cout << "Invalid number of worker threads =>> " << v_nt << std::endl;
      return 1;
    } else {
      std::cout << "---> Use " << nt << " worker threads <---" << std::endl;
    }
  }

  // Configure the application manager
  sc = appMgr->configure();
  std::cout << "---> Configure ApplicationMgr : " << appMgr->stateName() << " Status : " << sc.getCode() << std::endl;
  if( sc.isFailure() ) {
    std::cout << "---> Fatal error while configuring the ApplicationMgr " << std::endl;
    return 1;
  }

  // Initialize the application manager
  sc = appMgr->initialize();
  std::cout << "---> Initialize ApplicationMgr : " << appMgr->stateName() << " Status : " << sc.getCode() << std::endl;
  if( sc.isFailure() ) {
    std::cout << "---> Fatal error while initializing the ApplicationMgr " << std::endl;
    return 1;
  }

  // Create threads and process events
  worker_done = nt; //Initialized counter

  pthread_t thread[nt];
  int tID[nt];

  for (int i=0; i<nt; ++i) {
    /**
     * This will start the worker thread. It will stop by itself when the
     * called non-member method exits.
     */
    tID[i]=i;
    pthread_create(&thread[i], 0, work, static_cast<void*>(&tID[i]));
  }

  pthread_mutex_lock(&mutex);
  if (worker_done != 0) pthread_cond_wait(&condition, &mutex);
  pthread_mutex_unlock(&mutex);

  // Finalize the application manager
  sc = appMgr->finalize();
  std::cout << "---> Finalize  ApplicationMgr : " << appMgr->stateName() << " Status : " << sc.getCode() << std::endl;

  if( sc.isFailure() ) {
    std::cout << "---> Fatal error while finalizing  the ApplicationMgr " << std::endl;
    return 1;
  }

  // Terminate the application manager
  sc = appMgr->terminate();
  std::cout << "---> Terminate ApplicationMgr : " << appMgr->stateName() << " Status : " << sc.getCode() << std::endl;
  if( sc.isFailure() ) {
    std::cout << "---> Fatal error while terminating the ApplicationMgr " << std::endl;
    return 1;
  }

  //destroyes the mutex resources allocated
  pthread_cond_destroy(&condition);
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&coutmutex);

  // All done - exit
  iface->release();
  return 0;
}
