#ifndef PfmCodeAnalyserH
#define PfmCodeAnalyserH 1

#include <stdlib.h>
#include <stdio.h>
#include <perfmon/pfmlib.h>
#include <perfmon/perfmon.h>
#include <perfmon/perfmon_dfl_smpl.h>
#include <perfmon/pfmlib_core.h>
#include <perfmon/pfmlib_intel_nhm.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_EVT_NAME_LEN 256
#define NUM_PMCS PFMLIB_MAX_PMCS
#define NUM_PMDS PFMLIB_MAX_PMDS
#define FMT_NAME PFM_DFL_SMPL_NAME
#define MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS 4
#define cpuid(func,eax,ebx,ecx,edx)	__asm__ __volatile__ ("cpuid": "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (func));

class PfmCodeAnalyser
{
 private:
  int used_counters_number;
  int nehalem;
  pfmlib_input_param_t inp;
  pfmlib_output_param_t outp;
  pfarg_ctx_t ctx;
  pfarg_pmd_t pd[NUM_PMDS];
  pfarg_pmc_t pc[NUM_PMCS];
  pfarg_load_t load_arg;
  int fd;
  char event_str[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS][MAX_EVT_NAME_LEN];
  bool inv[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
  unsigned int cmask[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
  pfmlib_core_input_param_t params;
  pfmlib_nhm_input_param_t nhm_params;
  int ret;
  unsigned i;
  unsigned long sum[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
  unsigned count;
  unsigned overhead_avg[MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS];
  
 private:
  PfmCodeAnalyser(const char *event0, unsigned int cmask_v0, bool inv_v0,
                  const char *event1, unsigned int cmask_v1, bool inv_v1,
                  const char *event2, unsigned int cmask_v2, bool inv_v2,
                  const char *event3, unsigned int cmask_v3, bool inv_v3); // constructor hidden
  PfmCodeAnalyser(PfmCodeAnalyser const&); // copy constructor hidden
  PfmCodeAnalyser& operator=(PfmCodeAnalyser const&); // assign operator hidden
  ~PfmCodeAnalyser(); // destructor hidden

 public:
  static PfmCodeAnalyser& Instance(const char *event0 = "UNHALTED_CORE_CYCLES", unsigned int cmask_v0 = 0, bool inv_v0 = false,
                                   const char *event1 = "", unsigned int cmask_v1 = 0, bool inv_v1 = false,
                                   const char *event2 = "", unsigned int cmask_v2 = 0, bool inv_v2 = false,
                                   const char *event3 = "", unsigned int cmask_v3 = 0, bool inv_v3 = false);
  void start();
  void stop();
  void stop_init();
};

PfmCodeAnalyser::PfmCodeAnalyser(const char *event0, unsigned int cmask_v0, bool inv_v0,
                                 const char *event1, unsigned int cmask_v1, bool inv_v1,
                                 const char *event2, unsigned int cmask_v2, bool inv_v2,
                                 const char *event3, unsigned int cmask_v3, bool inv_v3)
{
 int ax,bx,cx,dx;
 cpuid(1,ax,bx,cx,dx);
 int sse4_2_mask = 1 << 20;
 nehalem = cx & sse4_2_mask;
 strcpy(event_str[0], event0);
 strcpy(event_str[1], event1);
 strcpy(event_str[2], event2);
 strcpy(event_str[3], event3);
 cmask[0] = cmask_v0;
 cmask[1] = cmask_v1;
 cmask[2] = cmask_v2;
 cmask[3] = cmask_v3;
 inv[0] = inv_v0;
 inv[1] = inv_v1;
 inv[2] = inv_v2;
 inv[3] = inv_v3;
 used_counters_number = 0;
 for(int i=0; i<MAX_NUMBER_OF_PROGRAMMABLE_COUNTERS; i++) 
 {
  if(strlen(event_str[i])>0) used_counters_number++;
 }
 for(int i=0; i<used_counters_number; i++) 
 {
  sum[i] = 0;
  overhead_avg[i] = 0;
 }
 count = 0;
 if(pfm_initialize() != PFMLIB_SUCCESS)
 {
  printf("Cannot initialize perfmon!!\nExiting...\n");
  exit(0);
 }
 while(count<3)
 {
  start();
  stop_init();
 }
 for(int i=0; i<used_counters_number; i++) 
 {
  sum[i] = 0;
 }
 count = 0;
 while(count<10)
 {
  start();
  stop_init();
 }
 for(int i=0; i<used_counters_number; i++) 
 {
  overhead_avg[i] = sum[i]/count;
  sum[i] = 0;
 }
 count = 0;
}

PfmCodeAnalyser& PfmCodeAnalyser::Instance(const char *event0, unsigned int cmask_v0, bool inv_v0,
                                           const char *event1, unsigned int cmask_v1, bool inv_v1,
                                           const char *event2, unsigned int cmask_v2, bool inv_v2,
                                           const char *event3, unsigned int cmask_v3, bool inv_v3)
{
 static PfmCodeAnalyser theSingleton(event0, cmask_v0, inv_v0, event1, cmask_v1, inv_v1, event2, cmask_v2, inv_v2, event3, cmask_v3, inv_v3);
 return theSingleton;
}

 // start()
 // initializes all the necessary structures to start the actual counting, calling pfm_start()
void PfmCodeAnalyser::start()
{
 memset(&ctx,0, sizeof(ctx));
 memset(&inp,0, sizeof(inp));
 memset(&outp,0, sizeof(outp));
 memset(pd, 0, sizeof(pd));
 memset(pc, 0, sizeof(pc));
 memset(&load_arg, 0, sizeof(load_arg));
 memset(&params, 0, sizeof(params));
 for(int i=0; i<used_counters_number; i++) 
 {
  ret = pfm_find_full_event(event_str[i], &inp.pfp_events[i]);
  if(ret != PFMLIB_SUCCESS)
  {
   fprintf(stderr, "ERROR: cannot find event: %s\naborting...\n", event_str[i]);
   exit(1);
  }
 }
 inp.pfp_dfl_plm = PFM_PLM3; 
 inp.pfp_event_count = used_counters_number;
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
 if(nehalem)
 {
  ret = pfm_dispatch_events(&inp, &nhm_params, &outp, NULL);
 }
 else
 {
  ret = pfm_dispatch_events(&inp, &params, &outp, NULL);
 }
 if(ret != PFMLIB_SUCCESS)
 {
  fprintf(stderr, "ERROR: cannot dispatch events: %s\naborting...\n", pfm_strerror(ret));
  exit(1);
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
 fd = pfm_create_context(&ctx, NULL, 0, 0);
 if(fd == -1)
 {
  fprintf(stderr, "ERROR: Context not created\naborting...\n");
  exit(1);
 }
 if(pfm_write_pmcs(fd, pc, outp.pfp_pmc_count) == -1)
 {
  fprintf(stderr, "ERROR: Could not write pmcs\naborting...\n");
  exit(1);
 }
 if(pfm_write_pmds(fd, pd, outp.pfp_pmd_count) == -1)
 {
  fprintf(stderr, "ERROR: Could not write pmds\naborting...\n");
  exit(1);
 }
 load_arg.load_pid = getpid();
 if(pfm_load_context(fd, &load_arg) == -1)
 {
  fprintf(stderr, "ERROR: Could not load context\naborting...\n");
  exit(1);
 }
 pfm_start(fd, NULL);   
}



 // stop()
 // const ModuleDescription& desc : description of the module that just finished its execution (we are only interested in its name)
 // stops the counting calling pfm_stop() and stores the counting results into the "results" map
void PfmCodeAnalyser::stop()
{
 pfm_stop(fd);
 if(pfm_read_pmds(fd, pd, inp.pfp_event_count) == -1)
 {
  fprintf(stderr, "ERROR: Could not read pmds\naborting...\n");
  exit(1);
 }
 
 for(int i=0; i<used_counters_number; i++) 
 {
  sum[i] += (pd[i].reg_value - overhead_avg[i]);
 }
 count++;
 close(fd);
}

void PfmCodeAnalyser::stop_init()
{
 pfm_stop(fd);
 if(pfm_read_pmds(fd, pd, inp.pfp_event_count) == -1)
 {
  fprintf(stderr, "ERROR: Could not read pmds\naborting...\n");
  exit(1);
 }
 
 for(int i=0; i<used_counters_number; i++) 
 {
  sum[i] += (pd[i].reg_value);
 }
 count++;
 close(fd);
}

PfmCodeAnalyser::~PfmCodeAnalyser()
{
 for(int i=0; i<used_counters_number; i++) 
 {
  printf("Event: %s\nTotal count:%lu\nNumber of counts:%u\nAverage count:%f\nOverhead removed:%u\n", event_str[i], sum[i], count, (double)sum[i]/count, overhead_avg[i]);
 }
}

#endif //PfmCodeAnalyserH
