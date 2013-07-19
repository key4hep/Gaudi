#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/MsgStream.h"

/*BEGIN: perfmon*/
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_core.h>
#include <perfmon/pfmlib_intel_nhm.h>
#include <vector>
#include <map>
#include <utility>
#include <sstream>

#include <perfmon/perfmon.h>
#include <perfmon/perfmon_dfl_smpl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <getopt.h>
#include <time.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

#include <algorithm>
#include <list>
#include <stack>
#include <cmath>
#include <sys/stat.h>
#include "IgHook_IgHookTrace.h"

// dlopen (link with -ldl)
#include <dlfcn.h>


#define MAX_EVT_NAME_LEN 256
#define NUM_PMCS PFMLIB_MAX_PMCS
#define NUM_PMDS PFMLIB_MAX_PMDS
#define FMT_NAME PFM_DFL_SMPL_NAME
#define BPL (sizeof(uint64_t)<<3)
#define LBPL 6

#define SYM_NAME_MAX_LENGTH 10000
#define MAX_OUTPUT_FILENAME_LENGTH 1024
#define MAX_EVENT_NAME_LENGTH 500
#define MAX_PREFIX_NAME_LENGTH 1024
#define FILENAME_MAX_LENGTH 1024

#define MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS 4

#define cpuid(func,ax,bx,cx,dx) __asm__ __volatile__ ("cpuid": "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));


static pfarg_pmd_t pd_smpl[NUM_PMDS];
static uint64_t collected_samples, collected_partial;
static int ctx_fd;
static pfm_dfl_smpl_hdr_t *hdr;
static uint64_t ovfl_count;
static size_t entry_size;
static unsigned int num_smpl_pmds;
static std::vector<std::map<std::string, std::map<unsigned long, unsigned int> > > samples(MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS); //a map of modules each containing numbers of samples of their addresses
static std::vector<std::map<std::string, std::vector<unsigned long int> > > results(MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS); //a map of modules and their result values across multiple events
static uint64_t last_overflow;
static uint64_t last_count;
static int sp[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];

static std::stack<std::pair<INamedInterface *, std::vector<unsigned long int> > > alg_stack;
/*END: perfmon*/



namespace {
  /// Hack to avoid the warning about casting between pointer to object and function [-pedantic]
  template <typename T>
  inline T function_cast(void *p) {
    union {
      void* object;
      T function;
    } caster;
    caster.object = p;
    return caster.function;
  }
  class PFMon {
  public:
    bool loaded;
    typedef void (*pfm_stop_t)(int);
    pfm_stop_t pfm_stop;
    typedef void (*pfm_self_stop_t)(int);
    pfm_self_stop_t pfm_self_stop;
    typedef os_err_t (*pfm_restart_t)(int);
    pfm_restart_t pfm_restart;
    typedef int (*pfm_read_pmds_t)(int, pfarg_pmd_t*, int);
    pfm_read_pmds_t pfm_read_pmds;
    typedef pfm_err_t (*pfm_initialize_t)();
    pfm_initialize_t pfm_initialize;
    typedef pfm_err_t (*pfm_find_full_event_t)(const char *, pfmlib_event_t *);
    pfm_find_full_event_t pfm_find_full_event;
    typedef pfm_err_t (*pfm_dispatch_events_t)(pfmlib_input_param_t *, void *, pfmlib_output_param_t *, void *);
    pfm_dispatch_events_t pfm_dispatch_events;
    typedef os_err_t (*pfm_create_context_t)(pfarg_ctx_t *, char *, void *, size_t);
    pfm_create_context_t pfm_create_context;
    typedef os_err_t (*pfm_write_pmcs_t)(int, pfarg_pmc_t *, int);
    pfm_write_pmcs_t pfm_write_pmcs;
    typedef os_err_t (*pfm_write_pmds_t)(int, pfarg_pmd_t *, int);
    pfm_write_pmds_t pfm_write_pmds;
    typedef os_err_t (*pfm_load_context_t)(int, pfarg_load_t *);
    pfm_load_context_t pfm_load_context;
    typedef os_err_t (*pfm_start_t)(int fd, pfarg_start_t *);
    pfm_start_t pfm_start;
    typedef char* (*pfm_strerror_t)(int);
    pfm_strerror_t pfm_strerror;
    typedef pfm_err_t (*pfm_set_options_t)(pfmlib_options_t *);
    pfm_set_options_t pfm_set_options;
    typedef pfm_err_t (*pfm_get_num_counters_t)(unsigned int *);
    pfm_get_num_counters_t pfm_get_num_counters;
    static PFMon &instance() {
      return s_instance;
    }
  private:
    // static void failure() { throw 1; }

    void* handle;

    PFMon() {
      handle = dlopen("libpfm.so", RTLD_NOW);
      if (handle) { loaded = true; } else { loaded = false; }
      if (loaded) {
        pfm_start = function_cast<pfm_start_t>(dlsym(handle, "pfm_start"));
        pfm_stop = function_cast<pfm_stop_t>(dlsym(handle, "pfm_stop"));
        pfm_self_stop = function_cast<pfm_self_stop_t>(dlsym(handle, "pfm_stop")); //it's the same
        pfm_restart = function_cast<pfm_restart_t>(dlsym(handle, "pfm_restart"));
        pfm_read_pmds = function_cast<pfm_read_pmds_t>(dlsym(handle, "pfm_read_pmds"));
        pfm_initialize = function_cast<pfm_initialize_t>(dlsym(handle, "pfm_initialize"));
        pfm_find_full_event = function_cast<pfm_find_full_event_t>(dlsym(handle, "pfm_find_full_event"));
        pfm_dispatch_events = function_cast<pfm_dispatch_events_t>(dlsym(handle, "pfm_dispatch_events"));
        pfm_create_context = function_cast<pfm_create_context_t>(dlsym(handle, "pfm_create_context"));
        pfm_write_pmcs = function_cast<pfm_write_pmcs_t>(dlsym(handle, "pfm_write_pmcs"));
        pfm_write_pmds = function_cast<pfm_write_pmds_t>(dlsym(handle, "pfm_write_pmds"));
        pfm_load_context = function_cast<pfm_load_context_t>(dlsym(handle, "pfm_load_context"));
        pfm_strerror = function_cast<pfm_strerror_t>(dlsym(handle, "pfm_strerror"));
        pfm_set_options = function_cast<pfm_set_options_t>(dlsym(handle, "pfm_set_options"));
        pfm_get_num_counters = function_cast<pfm_get_num_counters_t>(dlsym(handle, "pfm_get_num_counters"));
      } else {
        // pfm_start = pfm_stop = pfm_self_stop = pfm_restart = pfm_read_pmds = pfm_initialize = pfm_find_full_event = pfm_dispatch_events = pfm_create_context = pfm_write_pmcs = pfm_write_pmds = pfm_load_context = pfm_strerror = pfm_set_options = pfm_get_num_counters = failure;
      }
    }
    ~PFMon() {
      if (handle) dlclose(handle);
    }

    static PFMon s_instance;
  };

  PFMon PFMon::s_instance;
}



// ============================================================================
// GaudiAlg
// ============================================================================
// ============================================================================
/** @class PerfMonAuditor
 *
 *  Performance Monitoring Auditor that uses Perfmon2 library to monitor
 *  algorithms.
 *
 *  @author Daniele Francesco KRUSE daniele.francesco.kruse@cern.ch
 *  @date 2009-10-28
 */

class PerfMonAuditor: virtual public Auditor
{
 public:
  virtual void before(StandardEventType evt, INamedInterface* alg);
  virtual void after(StandardEventType evt, INamedInterface* alg, const StatusCode &sc);
  using Auditor::before;
  using Auditor::after;

 private:
  void i_beforeInitialize(INamedInterface* alg);
  void i_afterInitialize(INamedInterface* alg);
  void i_beforeExecute(INamedInterface* alg);
  void i_afterExecute(INamedInterface* alg);

 public:
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  int is_nehalem() {
#ifdef __ICC
// Disable ICC remark #593: variable "x" was set but never used
#pragma warning(push)
#pragma warning(disable:593)
#endif
    int a,b,c,d;
    cpuid(1,a,b,c,d);
    int sse4_2_mask = 1 << 20;
    if(c & sse4_2_mask) return 1; else return 0;
#ifdef __ICC
#pragma warning(pop)
#endif
  }

 private:
  PFMon &m_pfm;
/*
  typedef void (*pfm_stop_t)(int);
  pfm_stop_t pfm_stop;
  typedef void (*pfm_self_stop_t)(int);
  pfm_self_stop_t pfm_self_stop;

  typedef os_err_t (*pfm_restart_t)(int);
  pfm_restart_t pfm_restart;

  //typedef int (*pfm_read_pmds_t)(int, pfarg_pmd_t*, int);
  //pfm_read_pmds_t pfm_read_pmds;

  typedef pfm_err_t (*pfm_initialize_t)();
  pfm_initialize_t pfm_initialize;
  typedef pfm_err_t (*pfm_find_full_event_t)(const char *, pfmlib_event_t *);
  pfm_find_full_event_t pfm_find_full_event;
  typedef pfm_err_t (*pfm_dispatch_events_t)(pfmlib_input_param_t *, void *, pfmlib_output_param_t *, void *);
  pfm_dispatch_events_t pfm_dispatch_events;
  typedef os_err_t (*pfm_create_context_t)(pfarg_ctx_t *, char *, void *, size_t);
  pfm_create_context_t pfm_create_context;
  typedef os_err_t (*pfm_write_pmcs_t)(int, pfarg_pmc_t *, int);
  pfm_write_pmcs_t pfm_write_pmcs;
  typedef os_err_t (*pfm_write_pmds_t)(int, pfarg_pmd_t *, int);
  pfm_write_pmds_t pfm_write_pmds;
  typedef os_err_t (*pfm_load_context_t)(int, pfarg_load_t *);
  pfm_load_context_t pfm_load_context;
  typedef os_err_t (*pfm_start_t)(int fd, pfarg_start_t *);
  pfm_start_t pfm_start;
  typedef char* (*pfm_strerror_t)(int);
  pfm_strerror_t pfm_strerror;
  typedef pfm_err_t (*pfm_set_options_t)(pfmlib_options_t *);
  pfm_set_options_t pfm_set_options;
  typedef pfm_err_t (*pfm_get_num_counters_t)(unsigned int *);
  pfm_get_num_counters_t pfm_get_num_counters;
*/


 public:
  PerfMonAuditor(const std::string& name, ISvcLocator* pSvc):     // standard constructor
   Auditor(name, pSvc),
   m_pfm(PFMon::instance()),
   m_map(),
   m_indent(0),
   m_inEvent(false)
  {
is_nehalem_ret = is_nehalem();
declareProperty("EVENT0", event_str[0]);
declareProperty("EVENT1", event_str[1]);
declareProperty("EVENT2", event_str[2]);
declareProperty("EVENT3", event_str[3]);
declareProperty("FAMILY", family);
declareProperty("PREFIX", prefix);
declareProperty("INV0", inv[0]);
declareProperty("INV1", inv[1]);
declareProperty("INV2", inv[2]);
declareProperty("INV3", inv[3]);
declareProperty("CMASK0", cmask[0]);
declareProperty("CMASK1", cmask[1]);
declareProperty("CMASK2", cmask[2]);
declareProperty("CMASK3", cmask[3]);
declareProperty("SP0", sp[0]);
declareProperty("SP1", sp[1]);
declareProperty("SP2", sp[2]);
declareProperty("SP3", sp[3]);
declareProperty("SAMPLE", sampling);
declareProperty("START_AT_EVENT", start_at_event);
declareProperty("IS_NEHALEM", is_nehalem_ret);

// MsgStream log(msgSvc(), name());

///////////////////////////////////////////////////////////////////////////////////////
/*
// loading functions from PFM library
  void* handle = dlopen("libpfm.so", RTLD_NOW);
    if (!handle) {
//      log << MSG::ERROR << "Cannot open library: " << dlerror() << endmsg;
    }
  typedef void (*hello_t)();
    hello_t hello = (hello_t) dlsym(handle, "hello");
    if (!hello) {
//        log << MSG::ERROR << "Cannot load symbol 'hello': " << dlerror() << endmsg;
        dlclose(handle);
    }

    pfm_start = (pfm_start_t) dlsym(handle, "pfm_start");
    pfm_stop = (pfm_stop_t) dlsym(handle, "pfm_stop");
    pfm_self_stop = (pfm_self_stop_t) dlsym(handle, "pfm_stop"); //it's the same
    pfm_restart = (pfm_restart_t) dlsym(handle, "pfm_restart");
    //pfm_read_pmds = (pfm_read_pmds_t) dlsym(handle, "pfm_read_pmds");
    pfm_initialize = (pfm_initialize_t) dlsym(handle, "pfm_initialize");
    pfm_find_full_event = (pfm_find_full_event_t) dlsym(handle, "pfm_find_full_event");
    pfm_dispatch_events = (pfm_dispatch_events_t) dlsym(handle, "pfm_dispatch_events");
    pfm_create_context = (pfm_create_context_t) dlsym(handle, "pfm_create_context");
    pfm_write_pmcs = (pfm_write_pmcs_t) dlsym(handle, "pfm_write_pmcs");
    pfm_write_pmds = (pfm_write_pmds_t) dlsym(handle, "pfm_write_pmds");
    pfm_load_context = (pfm_load_context_t) dlsym(handle, "pfm_load_context");
    pfm_strerror = (pfm_strerror_t) dlsym(handle, "pfm_strerror");
    pfm_set_options = (pfm_set_options_t) dlsym(handle, "pfm_set_options");
    pfm_get_num_counters = (pfm_get_num_counters_t) dlsym(handle, "pfm_get_num_counters");
    // use it to do the calculation
//    log << MSG::INFO << "Calling hello..." << endmsg;
//    hello();

    // close the library
//    log << MSG::INFO << "Closing library..." << endmsg;
    dlclose(handle);
*/

///////////////////////////////////////////////////////////////////////////////////////

  }

  virtual ~PerfMonAuditor() {}                                    // virtual destructor

 private:
  PerfMonAuditor();                                               // the default constructor is disabled
  PerfMonAuditor(const PerfMonAuditor&);                          // copy constructor is disabled
  PerfMonAuditor& operator=(const PerfMonAuditor&);               // assignement operator is disabled

 private:
  typedef GaudiUtils::VectorMap<const INamedInterface*,int> Map;
  Map m_map;
  int m_indent;                                                   // indentation level
  bool m_inEvent;                                                 // "In event" flag

 private:
    int is_nehalem_ret;

    pfmlib_input_param_t inp;
    pfmlib_output_param_t outp;
    pfarg_ctx_t ctx;
    pfarg_pmd_t pd[NUM_PMDS];
    pfarg_pmc_t pc[NUM_PMCS];
    pfarg_load_t load_arg;
    int fd;
    unsigned int i;
    int ret;
    void startpm();
    void pausepm();
    void stoppm();
    void finalizepm();
    std::string event_str[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
    std::string prefix;
    std::string family;
    char event_cstr[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS][MAX_EVENT_NAME_LENGTH];
    char prefix_cstr[MAX_PREFIX_NAME_LENGTH];
    unsigned int ph_ev_count;
    bool inv[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
    unsigned int cmask[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
    unsigned int start_at_event;
    pfmlib_core_input_param_t params;
    pfmlib_nhm_input_param_t nhm_params;
    int used_counters_number;
    bool nehalem;
    bool westmere;
    bool core;

    bool sampling;
    int detect_unavail_pmu_regs(int fd, pfmlib_regmask_t *r_pmcs, pfmlib_regmask_t *r_pmds);
    int detect_unavail_pmcs(int fd, pfmlib_regmask_t *r_pmcs){return detect_unavail_pmu_regs(fd, r_pmcs, NULL);}
    void pfm_bv_set(uint64_t *bv, uint16_t rnum){bv[rnum>>LBPL] |= 1UL << (rnum&(BPL-1));}
    int pfm_bv_isset(uint64_t *bv, uint16_t rnum){return bv[rnum>>LBPL] & (1UL <<(rnum&(BPL-1))) ? 1 : 0;}
    void pfm_bv_copy(uint64_t *d, uint64_t *j, uint16_t n){if(n<=BPL) *d = *j; else {memcpy(d, j, (n>>LBPL)*sizeof(uint64_t));}}
    static void process_smpl_buf(pfm_dfl_smpl_hdr_t *hdr, size_t entry_size);
    static void sigio_handler(int, struct siginfo *, struct sigcontext *); // dlopen ==>
    //void sigio_handler(int, struct siginfo *, struct sigcontext *);
    void start_smpl();
    void stop_smpl();
    void finalize_smpl();
    pfm_dfl_smpl_arg_t buf_arg;
    pfarg_load_t load_args;
    void *buf_addr;
    unsigned num_counters;
    unsigned int max_pmd;
    pfmlib_options_t pfmlib_options;


  int level;

  bool first_alg;
  std::string first_alg_name;
  bool event_count_reached;


};

void PerfMonAuditor::startpm()
{
   MsgStream log(msgSvc(), name());
   memset(&ctx,0, sizeof(ctx));
   memset(&inp,0, sizeof(inp));
   memset(&outp,0, sizeof(outp));
   memset(pd, 0, sizeof(pd));
   memset(pc, 0, sizeof(pc));
   memset(&load_arg, 0, sizeof(load_arg));
   memset(&params, 0, sizeof(params));
   memset(&nhm_params, 0, sizeof(nhm_params));

   for(int i=0; i<used_counters_number; i++)
   {
    ret = m_pfm.pfm_find_full_event(event_cstr[i], &inp.pfp_events[i]);
    if(ret != PFMLIB_SUCCESS)
    {
     log << MSG::ERROR << "ERROR: cannot find event: " << event_cstr[i] << ". Aborting..." << endmsg;
    }
   }
   inp.pfp_dfl_plm = PFM_PLM3;
   inp.pfp_event_count = 4;
   for(int i=0; i<used_counters_number; i++)
   {
    if(inv[i])
    {
     (params.pfp_core_counters[i]).flags |= PFM_CORE_SEL_INV;
     (nhm_params.pfp_nhm_counters[i]).flags |= PFM_NHM_SEL_INV;
    }
    if(cmask[i]>0)
    {
     (params.pfp_core_counters[i]).cnt_mask = cmask[i];
     (nhm_params.pfp_nhm_counters[i]).cnt_mask = cmask[i];
    }
   }
   if(nehalem || westmere)
   {
    ret = m_pfm.pfm_dispatch_events(&inp, &nhm_params, &outp, NULL);
   }
   else
   {
    ret = m_pfm.pfm_dispatch_events(&inp, &params, &outp, NULL);
   }
   if(ret != PFMLIB_SUCCESS)
   {
    log << MSG::ERROR << "ERROR: cannot dispatch events: " << m_pfm.pfm_strerror(ret) << ". Aborting..." << endmsg;
   }
   for(unsigned int i=0; i<outp.pfp_pmc_count; i++)
   {
    pc[i].reg_num = outp.pfp_pmcs[i].reg_num;
    pc[i].reg_value = outp.pfp_pmcs[i].reg_value;
   }
   for(unsigned int i=0; i<outp.pfp_pmd_count; i++)
   {
    pd[i].reg_num = outp.pfp_pmds[i].reg_num;
    pd[i].reg_value = 0;
   }
   fd = m_pfm.pfm_create_context(&ctx, NULL, 0, 0);
   if(fd == -1)
   {
    log << MSG::ERROR << "ERROR: Context not created. Aborting..." << endmsg;
   }
   if(m_pfm.pfm_write_pmcs(fd, pc, outp.pfp_pmc_count) == -1)
   {
    log << MSG::ERROR << "ERROR: Could not write pmcs. Aborting..." << endmsg;
   }
   if(m_pfm.pfm_write_pmds(fd, pd, outp.pfp_pmd_count) == -1)
   {
    log << MSG::ERROR << "ERROR: Could not write pmds. Aborting..." << endmsg;
   }
   load_arg.load_pid = getpid();
   if(m_pfm.pfm_load_context(fd, &load_arg) == -1)
   {
    log << MSG::ERROR << "ERROR: Could not load context. Aborting..." << endmsg;
//  MsgStream log(msgSvc(), name());
//  log << MSG::ERROR << "Could not read pmds" << endmsg;
   }

   m_pfm.pfm_start(fd, NULL);
}





   // stoppm()
   // const ModuleDescription& desc : description of the module that just finished its execution (we are only interested in its name)
   // stops the counting calling pfm_stop() and stores the counting results into the "results" map
void PerfMonAuditor::stoppm()
{
 MsgStream log(msgSvc(), name());
 m_pfm.pfm_stop(fd);
 if(m_pfm.pfm_read_pmds(fd, pd, inp.pfp_event_count) == -1)
 {
  log << MSG::ERROR << "Could not read pmds" << endmsg;
 }
   for(int i=0; i<used_counters_number; i++)
   {
 results[i][(alg_stack.top().first)->name()].push_back(alg_stack.top().second[i] + pd[i].reg_value);
   }

 close(fd);
}


void PerfMonAuditor::pausepm()
{
 MsgStream log(msgSvc(), name());
 m_pfm.pfm_stop(fd);
 if(m_pfm.pfm_read_pmds(fd, pd, inp.pfp_event_count) == -1)
 {
  log << MSG::ERROR << "Could not read pmds" << endmsg;
 }

   for(int i=0; i<used_counters_number; i++)
   {
 alg_stack.top().second[i] += pd[i].reg_value;
   }

 close(fd);
}


   // finalizepm()
   // called when all the countings of the current event are finished, it dumps the results
   // into the output file corresponding to the event being counted
void PerfMonAuditor::finalizepm()
{
 MsgStream log(msgSvc(), name());
   log << MSG::INFO << "start of finalizepm ucn:" << used_counters_number << endmsg;
   char filename[MAX_OUTPUT_FILENAME_LENGTH];
   char to_cat[50];
   FILE *outfile;
   for(int i=0; i<used_counters_number; i++)
   {
    bzero(filename, MAX_OUTPUT_FILENAME_LENGTH);
    sprintf(filename, "%s_%s", prefix_cstr, event_cstr[i]);
    for(int j=0; j<(int)strlen(filename); j++)
    {
     if(filename[j]==':')
     {
      filename[j]='-';
     }
    }
    bzero(to_cat, 50);
    if(inv[i])
    {
     strcpy(to_cat, "_INV_1");
    }
    if(cmask[i]>0)
    {
     sprintf(to_cat, "%s_CMASK_%d", to_cat, cmask[i]);
    }
    sprintf(filename, "%s%s.txt", filename, to_cat);
    log << MSG::INFO << "Filename:" << filename << endmsg;
    outfile = fopen(filename, "w");
    if(nehalem)
    {
     fprintf(outfile, "NHM ");
    }
    else if(westmere)
    {
     fprintf(outfile, "WSM ");
    }
    else if(core)
    {
     fprintf(outfile, "CORE ");
    }
    fprintf(outfile, "%s %d %d %d\n", event_cstr[i], cmask[i], inv[i], sp[i]);
    for(std::map<std::string, std::vector<unsigned long int> >::iterator it=(results[i]).begin(); it!=(results[i]).end(); it++)
    {
     fprintf(outfile, "%s\n", (it->first).c_str());
     for(std::vector<unsigned long int>::iterator j=(it->second).begin(); j!=(it->second).end(); j++)
     {
      fprintf(outfile, "%lu\n", *j);
     }
    }
    fclose(outfile);
   }
}

StatusCode PerfMonAuditor::initialize()
{
 MsgStream log(msgSvc(), name());

 if (!m_pfm.loaded) {
  log << MSG::ERROR << "pfm library could not be loaded" << endmsg;
  return false;
 }

 log << MSG::INFO << "Initializing..." << endmsg;
 StatusCode sc = Auditor::initialize() ;
 if(sc.isFailure())
 {
  return sc;
 }
   used_counters_number = 0;
   for(int i=0; i<MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS; i++)
   {
    if(event_str[i].length()>0) used_counters_number++;
   }
   for(int i=0; i<MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS; i++)
   {
    strcpy(event_cstr[i], event_str[i].c_str());
   }
   strcpy(prefix_cstr, prefix.c_str());

 if(m_pfm.pfm_initialize() != PFMLIB_SUCCESS)
 {
 log << MSG::ERROR << "Cannot initialize perfmon!!" << endmsg;
 }
 ph_ev_count = 0;
 first_alg = true;
 event_count_reached = false;
 nehalem = false;
 core = false;
 westmere = false;
 if(family.compare("CORE")==0) core = true;
 else if(family.compare("NEHALEM")==0) nehalem = true;
 else if(family.compare("WESTMERE")==0) westmere = true;
 else
 {
  log << MSG::ERROR << "ERROR: Unsupported processor family " << family  << ". aborting..." << endmsg;
 }

 log << MSG::INFO << "Initialized!" << endmsg;
 return StatusCode::SUCCESS ;
}

   // process_smpl_buf()
   // pfm_dfl_smpl_hdr_t *hdr : pointer to header of the buffer containing addresses sampled during the sampling process
   // size_t entry_size       : size of each entry, used to navigate through the various entries
   // called when the sampling buffer is full, saves the samples into memory ("samples" map)
void PerfMonAuditor::process_smpl_buf(pfm_dfl_smpl_hdr_t *hdr, size_t entry_size)
{
////////
   pfm_dfl_smpl_entry_t *ent;
   size_t pos, count;
   uint64_t entry;
   if(hdr->hdr_overflows == last_overflow && hdr->hdr_count == last_count)
   {
    printf("skipping identical set of samples...\n");
    return;
   }
   count = hdr->hdr_count;
   ent = (pfm_dfl_smpl_entry_t *)(hdr+1);
   pos = (unsigned long)ent;
   entry = collected_samples;
   while(count--)
   {
    //if(ent->ovfl_pmd>=0 && ent->ovfl_pmd<=3)
    if(ent->ovfl_pmd<=3)
    {
     ((samples[ent->ovfl_pmd])[(alg_stack.top().first)->name()])[(unsigned long)(ent->ip)]++;
    }
    pos += entry_size;
    ent = (pfm_dfl_smpl_entry_t *)pos;
    entry++;
   }
   collected_samples = entry;
   last_overflow = hdr->hdr_overflows;
   if (last_count != hdr->hdr_count && (last_count || last_overflow == 0))
   {
    collected_partial += hdr->hdr_count;
   }
   last_count = hdr->hdr_count;
 return;
}

   // sigio_handler()
   // int n                 : signal number of the signal being delivered
   // struct siginfo *info  : pointer to a siginfo_t structure containing info about the signal
   // struct sigcontext *sc : context of the signal, NULL in our case
   // signal handler used to catch sampling buffer overflows. When they occur it calls the process_smpl_buf() function
void PerfMonAuditor::sigio_handler(int /*n*/, struct siginfo */*info*/, struct sigcontext */*sc*/)
{
   //MsgStream log(msgSvc(), name());
   PFMon& pfm = PFMon::instance();
   pfarg_msg_t msg;
   int fd = ctx_fd;
   int r;
   if(fd != ctx_fd)
   {
    //log << MSG::ERROR << "ERROR: handler does not get valid file descriptor. Aborting..." << endmsg;
   }
   if(pfm.pfm_read_pmds(fd, pd_smpl+1, 1) == -1)
   {
    //log << MSG::ERROR << "ERROR: pfm_read_pmds: " << strerror(errno) << ". Aborting..." << endmsg;
   }
   while(true)
   {
    r = read(fd, &msg, sizeof(msg));
    if(r!=sizeof(msg))
    {
     if(r==-1 && errno==EINTR)
     {
      printf("read interrupted, retrying\n");
      continue;
     }
     //log << MSG::ERROR << "ERROR: cannot read overflow message: " << strerror(errno) << ". Aborting..." << endmsg;
    }
    break;
   }
   switch(msg.type)
   {
    case PFM_MSG_OVFL: // the sampling buffer is full
     process_smpl_buf(hdr, entry_size);
     ovfl_count++;
     if(pfm.pfm_restart(fd))
     {
      if(errno!=EBUSY)
      {
       //log << MSG::ERROR << "ERROR: pfm_restart error errno " << errno << ". Aborting..." << endmsg;
      }
      else
      {
       printf("pfm_restart: task probably terminated \n");
      }
     }
     break;
    default:
     //log << MSG::ERROR << "ERROR: unknown message type " << msg.type << ". Aborting..." << endmsg;
     break;
   }

}







   // start_smpl()
   // const ModuleDescription& desc : description of the module that is just starting its execution (we are only interested in its name)
   // initializes all the necessary structures to start the sampling, calling pfm_self_start()
void PerfMonAuditor::start_smpl()
{
   MsgStream log(msgSvc(), name());
   ovfl_count = 0;
   num_smpl_pmds = 0;
   last_overflow = ~0;
   max_pmd = 0;
   memset(&pfmlib_options, 0, sizeof(pfmlib_options));
   pfmlib_options.pfm_debug   = 0;
   pfmlib_options.pfm_verbose = 0;
   m_pfm.pfm_set_options(&pfmlib_options);
   ret = m_pfm.pfm_initialize();
   if(ret != PFMLIB_SUCCESS)
   {
    log << MSG::ERROR << "ERROR: Cannot initialize library: " << m_pfm.pfm_strerror(ret) << ". Aborting..." << endmsg;
   }
   struct sigaction act;
   memset(&act, 0, sizeof(act));
   act.sa_handler = (sig_t)sigio_handler; // dlopen() ==>
   //act.sa_handler = (sig_t)&sigio_handler;
   sigaction(SIGIO, &act, 0);
   memset(&ctx, 0, sizeof(ctx));
   memset(&buf_arg, 0, sizeof(buf_arg));
   memset(&inp,0, sizeof(inp));
   memset(&outp,0, sizeof(outp));
   memset(pd_smpl, 0, sizeof(pd_smpl));
   memset(pc, 0, sizeof(pc));
   memset(&load_args, 0, sizeof(load_args));
   m_pfm.pfm_get_num_counters(&num_counters);
   memset(&params, 0, sizeof(params));
   memset(&nhm_params, 0, sizeof(nhm_params));

   for(int i=0; i<used_counters_number; i++)
   {
    ret = m_pfm.pfm_find_full_event(event_cstr[i], &inp.pfp_events[i]);
    if(ret != PFMLIB_SUCCESS)
    {
     log << MSG::ERROR << "ERROR: cannot find event: " << event_cstr[i] << ". Aborting..." << endmsg;
    }
   }
   inp.pfp_dfl_plm = PFM_PLM3;
   inp.pfp_event_count = 4;
   for(int i=0; i<used_counters_number; i++)
   {
    if(inv[i])
    {
     (params.pfp_core_counters[i]).flags |= PFM_CORE_SEL_INV;
     (nhm_params.pfp_nhm_counters[i]).flags |= PFM_NHM_SEL_INV;
    }
    if(cmask[i]>0)
    {
     (params.pfp_core_counters[i]).cnt_mask = cmask[i];
     (nhm_params.pfp_nhm_counters[i]).cnt_mask = cmask[i];
    }
   }
   if(nehalem || westmere)
   {
    ret = m_pfm.pfm_dispatch_events(&inp, &nhm_params, &outp, NULL);
   }
   else
   {
    ret = m_pfm.pfm_dispatch_events(&inp, &params, &outp, NULL);
   }
   if(ret != PFMLIB_SUCCESS)
   {
    log << MSG::ERROR << "ERROR: cannot configure events: " << m_pfm.pfm_strerror(ret) << ". Aborting..." << endmsg;
   }
   for(unsigned int i=0; i<outp.pfp_pmc_count; i++)
   {
    pc[i].reg_num   = outp.pfp_pmcs[i].reg_num;
    pc[i].reg_value = outp.pfp_pmcs[i].reg_value;
   }
   for(unsigned int i=0; i<outp.pfp_pmd_count; i++)
   {
    pd_smpl[i].reg_num = outp.pfp_pmds[i].reg_num;
    if(i)
    {
     pfm_bv_set(pd_smpl[0].reg_smpl_pmds, pd_smpl[i].reg_num);
     if(pd_smpl[i].reg_num>max_pmd)
     {
      max_pmd = pd_smpl[i].reg_num;
     }
     num_smpl_pmds++;
    }
   }
   for(int i=0; i<used_counters_number; i++)
   {
    pd_smpl[i].reg_flags |= PFM_REGFL_OVFL_NOTIFY | PFM_REGFL_RANDOM;
    pfm_bv_copy(pd_smpl[i].reg_reset_pmds, pd_smpl[i].reg_smpl_pmds, max_pmd);
    pd_smpl[i].reg_value = (uint64_t)(sp[i] * -1);
    pd_smpl[i].reg_short_reset = (uint64_t)(sp[i] * -1);
    pd_smpl[i].reg_long_reset = (uint64_t)(sp[i] * -1);
    pd_smpl[i].reg_random_seed = 5; //tocheck
    pd_smpl[i].reg_random_mask = 0xff; //tocheck
   }
   entry_size = sizeof(pfm_dfl_smpl_entry_t)+(num_smpl_pmds<<3);
   ctx.ctx_flags = 0;
   buf_arg.buf_size = 3*getpagesize()+512;
   ctx_fd = m_pfm.pfm_create_context(&ctx, (char *)FMT_NAME, &buf_arg, sizeof(buf_arg));
   if(ctx_fd==-1)
   {
    if(errno==ENOSYS)
    {
     log << MSG::ERROR << "ERROR: Your kernel does not have performance monitoring support! Aborting..." << endmsg;
    }
    log << MSG::ERROR << "ERROR: Can't create PFM context " << strerror(errno) << ". Aborting..." << endmsg;
   }
   buf_addr = mmap(NULL, (size_t)buf_arg.buf_size, PROT_READ, MAP_PRIVATE, ctx_fd, 0);
   if(buf_addr==MAP_FAILED)
   {
    log << MSG::ERROR << "ERROR: cannot mmap sampling buffer: " << strerror(errno) << ". Aborting..." << endmsg;
   }
   hdr = (pfm_dfl_smpl_hdr_t *)buf_addr;
   if(PFM_VERSION_MAJOR(hdr->hdr_version)<1)
   {
    log << MSG::ERROR << "ERROR: invalid buffer format version. Aborting..." << endmsg;
   }
   if(m_pfm.pfm_write_pmcs(ctx_fd, pc, outp.pfp_pmc_count))
   {
    log << MSG::ERROR << "ERROR: pfm_write_pmcs error errno " << strerror(errno) << ". Aborting..." << endmsg;
   }
   if(m_pfm.pfm_write_pmds(ctx_fd, pd_smpl, outp.pfp_pmd_count))
   {
    log << MSG::ERROR << "ERROR: pfm_write_pmds error errno " << strerror(errno) << ". Aborting..." << endmsg;
   }
   load_args.load_pid = getpid();
   if(m_pfm.pfm_load_context(ctx_fd, &load_args))
   {
    log << MSG::ERROR << "ERROR: pfm_load_context error errno " << strerror(errno) << ". Aborting..." << endmsg;
   }
   ret = fcntl(ctx_fd, F_SETFL, fcntl(ctx_fd, F_GETFL, 0) | O_ASYNC);
   if(ret == -1)
   {
    log << MSG::ERROR << "ERROR: cannot set ASYNC: " << strerror(errno) << ". Aborting..." << endmsg;
   }
   ret = fcntl(ctx_fd, F_SETOWN, getpid());
   if(ret == -1)
   {
    log << MSG::ERROR << "ERROR: cannot setown: " << strerror(errno) << ". Aborting..." << endmsg;
   }
   //pfm_self_start(ctx_fd); ==>
   m_pfm.pfm_start(ctx_fd, NULL);
  }


   // stop_smpl()
   // const ModuleDescription& desc : description of the module that just finished its execution (we are only interested in its name)
   // stops the sampling and calls process_smpl_buf() one last time to process all the remaining samples
void PerfMonAuditor::stop_smpl()
  {
 MsgStream log(msgSvc(), name());
   m_pfm.pfm_self_stop(ctx_fd);
   process_smpl_buf(hdr, entry_size);
   close(ctx_fd);
   ret = munmap(hdr, (size_t)buf_arg.buf_size);
   if(ret)
   {
  log << MSG::ERROR << "Cannot unmap buffer: %s" << strerror(errno) << endmsg;
   }
   return;
  }



   // finalize_smpl()
   // processes the sampling results in order to find library and offset of each sampled address, using the symbol() and tosymbol() functions,
   // and then dumps the new found information into gzipped output files, to be processed later
void PerfMonAuditor::finalize_smpl()
{
   MsgStream log(msgSvc(), name());
   char filename[MAX_OUTPUT_FILENAME_LENGTH];
   bzero(filename, MAX_OUTPUT_FILENAME_LENGTH);
   char to_cat[50];
   gzFile outfile;
   int err;
   for(int i=0; i<used_counters_number; i++)
   {
    sprintf(filename, "%s_%s", prefix_cstr, event_cstr[i]);
    for(int j=0; j<(int)strlen(filename); j++)
    {
     if(filename[j]==':')
     {
      filename[j]='-';
     }
    }
    bzero(to_cat, 50);
    if(inv[i])
    {
     strcpy(to_cat, "_INV_1");
    }
    if(cmask[i]>0)
    {
     sprintf(to_cat, "%s_CMASK_%d", to_cat, cmask[i]);
    }
    sprintf(filename, "%s%s.txt.gz", filename, to_cat);
    outfile = gzopen(filename, "wb");
    if(outfile!=NULL)
    {
     if(nehalem)
     {
      gzprintf(outfile, "NHM ");
     }
     else if(westmere)
     {
      gzprintf(outfile, "WSM ");
     }
     else if(core)
     {
      gzprintf(outfile, "CORE ");
     }
     if(gzprintf(outfile, "%s %d %d %d\n", event_cstr[i], cmask[i], inv[i], sp[i]) < (int)strlen(event_cstr[i]))
     {
      log << MSG::ERROR << "ERROR: gzputs err: " << gzerror(outfile, &err) << ". Aborting..." << endmsg;
     }
     for(std::map<std::string, std::map<unsigned long, unsigned int> >::iterator it=samples[i].begin(); it!=samples[i].end(); it++)
     {
      unsigned long long sum = 0;
      for(std::map<unsigned long, unsigned int>::iterator jt=(it->second).begin(); jt!=(it->second).end(); jt++)
      {
       sum += jt->second;
      }
      if(gzprintf(outfile, "%s%%%llu\n", (it->first).c_str(), sum) < (int)((it->first).length()))
      {
       log << MSG::ERROR << "ERROR: gzputs err: " << gzerror(outfile, &err) << ". Aborting..." << endmsg;
      }
      for(std::map<unsigned long, unsigned int>::iterator jt=(it->second).begin(); jt!=(it->second).end(); jt++)
      {
       char sym_name[SYM_NAME_MAX_LENGTH];
       bzero(sym_name, SYM_NAME_MAX_LENGTH);
       const char *libName;
       const char *symbolName;
       int libOffset = 0;
       int offset = 0;
       void *sym_addr = IgHookTrace::tosymbol((void *)(jt->first));
       if(sym_addr != NULL)
       {
        bool success = IgHookTrace::symbol(sym_addr, symbolName, libName, offset, libOffset);
        if(success)
        {
         if(symbolName!=NULL && strlen(symbolName)>0)
         {
  	  strcpy(sym_name, symbolName);
	  strcat(sym_name, " ");
         }
         else
         {
          strcpy(sym_name, "??? ");
         }
         if(libName!=NULL && strlen(libName)>0)
         {
          strcat(sym_name, libName);
  	  strcat(sym_name, " ");
         }
         else
         {
          strcat(sym_name, "??? ");
         }
         sprintf(sym_name, "%s%d ", sym_name, libOffset);
         if(strlen(sym_name)<=0)
         {
          log << MSG::ERROR << "ERROR: Symbol name length is zero. Aborting..." << endmsg;
         }
        }
        else
        {
         strcpy(sym_name,"??? ??? 0 ");
        }
       }
       else
       {
        strcpy(sym_name,"??? ??? 0 ");
       }
       if(gzprintf(outfile, "%s %d\n", sym_name, jt->second) < (int)strlen(sym_name))
       {
        log << MSG::ERROR << "ERROR: gzputs err: " << gzerror(outfile, &err) << endmsg;
       }
      }
     }
    }
    else
    {
     log << MSG::ERROR << "ERROR: Could not open file: " << filename << ". Aborting..." << endmsg;
    }
    gzclose(outfile);
   }
  }



StatusCode PerfMonAuditor::finalize()
{
 if(sampling == 0) finalizepm();
 else finalize_smpl();
 return Auditor::finalize();
}


void PerfMonAuditor::before(StandardEventType evt, INamedInterface *alg)
{
 switch(evt)
 {
  case IAuditor::Initialize:
   i_beforeInitialize(alg);
   break;
  case IAuditor::Execute:
   i_beforeExecute(alg);
   break;
  default:
   break;
 }
 return;
}

void PerfMonAuditor::after(StandardEventType evt, INamedInterface *alg, const StatusCode &)
{
 switch(evt)
 {
  case IAuditor::Initialize:
   i_afterInitialize(alg);
   break;
  case IAuditor::Execute:
   i_afterExecute(alg);
   break;
  default:
   break;
 }
 return;
}

void PerfMonAuditor::i_beforeInitialize(INamedInterface* alg)
{
 if(alg == 0)
 {
  return;
 }
 return;
}

void PerfMonAuditor::i_afterInitialize(INamedInterface* alg)
{
 if(alg == 0)
 {
  return;
 }
 return;
}

void PerfMonAuditor::i_beforeExecute(INamedInterface* alg)
{
 MsgStream log(msgSvc(), name());
 if(alg == 0)
 {
  return;
 }
 //log << MSG::INFO << "before:inside! " << alg->name() << endmsg;
 if(first_alg)
 {
  first_alg = false;
  first_alg_name = alg->name();
  //log << MSG::INFO << "first_alg_name= " << alg->name() << endmsg;
 }
 if(!event_count_reached)
 {
  if(!first_alg_name.compare(alg->name()))
  {
   ph_ev_count++;
   //log << MSG::INFO << "EVENT COUNT: " << ph_ev_count << endmsg;
   if(ph_ev_count==start_at_event)
   {
    event_count_reached = true;
    //log << MSG::INFO << "!!! EVENT COUNT REACHED: " << ph_ev_count << endmsg;
   }
  }
 }
 if(event_count_reached)
 {
   //log << MSG::INFO << "before:inside! " << alg->name() << endmsg;

  if(!alg_stack.empty())
  {
   if(sampling == 0) pausepm(); //pausing father algorithm counting
   else stop_smpl();
  }
  ++m_indent;
  std::vector <unsigned long int> zeroes(4,0);
  alg_stack.push(std::make_pair(alg, zeroes));
  if(sampling == 0) startpm();
  else start_smpl();
 }
 return;
}

void PerfMonAuditor::i_afterExecute(INamedInterface* alg)
{
 MsgStream log(msgSvc(), name());
 if(alg == 0)
 {
  return;
 }// log << MSG::INFO << "after:inside! " << alg->name() << endmsg;

 if(event_count_reached)
 {
   //log << MSG::INFO << "after:inside! " << alg->name() << endmsg;

  if(sampling == 0) stoppm();
  else stop_smpl();
  alg_stack.pop();
  --m_indent;
  if(!alg_stack.empty())
  {
   if(sampling == 0) startpm();
   else start_smpl(); //resuming father algorithm counting
   }
 }
 return;
}

DECLARE_COMPONENT(PerfMonAuditor)
