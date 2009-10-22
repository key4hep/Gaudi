/**
 * \file proc_utils.cpp
 * This file contains the implementations of the methods for extracting 
 * information from the proc filesystem.
 */

/*
 * ApMon - Application Monitoring Tool
 * Version: 2.2.0
 *
 * Copyright (C) 2006 California Institute of Technology
 *
 * Permission is hereby granted, free of charge, to use, copy and modify 
 * this software and its documentation (the "Software") for any
 * purpose, provided that existing copyright notices are retained in 
 * all copies and that this notice is included verbatim in any distributions
 * or substantial portions of the Software. 
 * This software is a part of the MonALISA framework (http://monalisa.cacr.caltech.edu).
 * Users of the Software are asked to feed back problems, benefits,
 * and/or suggestions about the software to the MonALISA Development Team
 * (developers@monalisa.cern.ch). Support for this software - fixing of bugs,
 * incorporation of new features - is done on a best effort basis. All bug
 * fixes and enhancements will be made available under the same terms and
 * conditions as the original software,

 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
 * EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. THIS SOFTWARE IS
 * PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO
 * OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 */

#include "mon_constants.h"
#include "utils.h"
#include "ApMon.h"
#include "proc_utils.h"

#ifndef WIN32
#include <dirent.h>
#endif

using namespace apmon_utils;

void ProcUtils::getCPUUsage(ApMon& apm, double& cpuUsage, 
			       double& cpuUsr, double& cpuSys, 
			       double& cpuNice, double& cpuIdle, int numCPUs) 
  throw (runtime_error, procutils_error) {
#ifndef WIN32
  FILE *fp1;
  char line[MAX_STRING_LEN];
  char s1[20];  double usrTime, sysTime, niceTime, idleTime, totalTime;
  int indU, indS, indN, indI;

  time_t crtTime = time(NULL);
  fp1 = fopen("/proc/stat", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getCPUUsage() ] Could not open /proc/stat");

  while (fgets(line, MAX_STRING_LEN, fp1)) {
      if (strstr(line, "cpu") == line)
	break;
  }
  
  if (line == NULL) {
    fclose(fp1);
    throw procutils_error((char*)"[ getCPUUsage() ] Could not obtain CPU usage info from /proc/stat");
  }
  fclose(fp1);

  sscanf(line, "%s %lf %lf %lf %lf", s1, &usrTime, &niceTime, &sysTime, 
	 &idleTime);

  indU = getVectIndex( (char*)"cpu_usr", apm.sysMonitorParams, apm.nSysMonitorParams);
  indS = getVectIndex((char*)"cpu_sys", apm.sysMonitorParams, apm.nSysMonitorParams);
  indN = getVectIndex((char*)"cpu_nice", apm.sysMonitorParams, apm.nSysMonitorParams);
  indI = getVectIndex((char*)"cpu_idle", apm.sysMonitorParams, apm.nSysMonitorParams);
  if (idleTime < apm.lastSysVals[indI]) {
    apm.lastSysVals[indU] = usrTime;
    apm.lastSysVals[indS] = sysTime;
    apm.lastSysVals[indN] = niceTime;
    apm.lastSysVals[indI] = idleTime;
    throw runtime_error("[ getCPUUsage() ] CPU usage counter reset");
  }

  if (numCPUs == 0)
    throw procutils_error((char*)"[ getCPUUsage() ] Number of CPUs was not initialized");

  //printf("### crtTime %ld lastSysInfo %ld\n", crtTime, apm.lastSysInfoSend);  
  if (crtTime <= apm.lastSysInfoSend)
    throw runtime_error("[ getCPUUsage() ] Current time <= time of the previous sysInfoSend");
  
  totalTime = (usrTime - apm.lastSysVals[indU]) + 
    (sysTime -apm.lastSysVals[indS]) +
    (niceTime - apm.lastSysVals[indN]) + 
    (idleTime - apm.lastSysVals[indI]);

  cpuUsr = 100 * (usrTime - apm.lastSysVals[indU]) / totalTime;
  cpuSys = 100 * (sysTime - apm.lastSysVals[indS]) / totalTime;
  cpuNice = 100 * (niceTime - apm.lastSysVals[indN]) / totalTime;
  cpuIdle = 100 * (idleTime - apm.lastSysVals[indI]) / totalTime;
  cpuUsage = 100 * (totalTime - (idleTime - apm.lastSysVals[indI])) / totalTime;

  apm.lastSysVals[indU] = usrTime;
  apm.lastSysVals[indN] = niceTime;
  apm.lastSysVals[indI] = idleTime;
  apm.lastSysVals[indS] = sysTime;
#endif
}

void ProcUtils::getSwapPages(ApMon& apm, double& pagesIn, 
			       double& pagesOut, double& swapIn, 
			     double& swapOut) 
  throw (runtime_error, procutils_error) {
#ifndef WIN32
  FILE *fp1;
  char line[MAX_STRING_LEN];
  char s1[20];
  bool foundPages, foundSwap;
  double p_in, p_out, s_in, s_out;
  int ind1, ind2;

  time_t crtTime = time(NULL);
  fp1 = fopen("/proc/stat", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getSwapPages() ] Could not open /proc/stat");
  if (crtTime <= apm.lastSysInfoSend)
    throw runtime_error("[ getSwapPages() ] Current time <= time of the previous sysInfoSend");

  foundPages = foundSwap = false;
  while (fgets(line, MAX_STRING_LEN, fp1)) {
    if (strstr(line, "page") == line) {
      foundPages = true;
      sscanf(line, "%s %lf %lf ", s1, &p_in, &p_out);

      ind1 = getVectIndex((char*)"pages_in", apm.sysMonitorParams, apm.nSysMonitorParams);
      ind2 = getVectIndex((char*)"pages_out", apm.sysMonitorParams, apm.nSysMonitorParams);
      if (p_in < apm.lastSysVals[ind1] || p_out < apm.lastSysVals[ind2]) {
	apm.lastSysVals[ind1] = p_in;
	apm.lastSysVals[ind2] = p_out;
	throw runtime_error("[ getSwapPages() ] Pages in/out counter reset");
      }
      pagesIn = (p_in - apm.lastSysVals[ind1]) / (crtTime - apm.lastSysInfoSend);
      pagesOut = (p_out - apm.lastSysVals[ind2]) / (crtTime - apm.lastSysInfoSend);
      apm.lastSysVals[ind1] = p_in;
      apm.lastSysVals[ind2] = p_out;

    }

    if (strstr(line, "swap") == line) {
      foundSwap = true;
      sscanf(line, "%s %lf %lf ", s1, &s_in, &s_out);

      ind1 = getVectIndex((char*)"swap_in", apm.sysMonitorParams, apm.nSysMonitorParams);
      ind2 = getVectIndex((char*)"swap_out", apm.sysMonitorParams, apm.nSysMonitorParams);
      if (s_in < apm.lastSysVals[ind1] || s_out < apm.lastSysVals[ind2]) {
	apm.lastSysVals[ind1] = s_in;
	apm.lastSysVals[ind2] = s_out;
	throw runtime_error("[ getSwapPages() ] Swap in/out counter reset");
      }
      swapIn = (s_in - apm.lastSysVals[ind1]) / (crtTime - apm.lastSysInfoSend);
      swapOut = (s_out - apm.lastSysVals[ind2]) / (crtTime - apm.lastSysInfoSend);
      apm.lastSysVals[ind1] = s_in;
      apm.lastSysVals[ind2] = s_out;

    }
  }
   
  fclose(fp1);

  if (!foundPages || !foundSwap) {
    throw procutils_error((char*)"[ getSwapPages() ] Could not obtain swap/pages in/out from /proc/stat");
  }
#endif
}

void ProcUtils::getLoad(double &load1, double &load5, 
	   double &load15, double &processes) 
  throw(procutils_error) {
#ifndef WIN32
  double v1, v5, v15, activeProcs, totalProcs;
  FILE *fp1;

  fp1 = fopen("/proc/loadavg", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getLoad() ] Could not open /proc/loadavg");

  fscanf(fp1, "%lf %lf %lf", &v1, &v5, &v15);
  load1 = v1;
  load5 = v5;
  load15 = v15;

  fscanf(fp1, "%lf/%lf", &activeProcs, &totalProcs);
  processes = totalProcs;
  fclose(fp1);
#endif
}

void ProcUtils::getProcesses(double& processes, double states[]) 
  throw(runtime_error) {
#ifndef WIN32
  char *argv[4];
  char psstat_f[40];
  pid_t mypid = getpid();
  pid_t cpid;
  int status;
  char ch, buf[100];
  FILE *pf;

  sprintf(psstat_f, "/tmp/apmon_psstat%d", mypid);

 switch (cpid = fork()) {
  case -1:
    throw runtime_error("[ getProcesses() ] Unable to fork()");
  case 0:
    argv[0] = (char *)"/bin/sh"; argv[1] = (char *)"-c";
    sprintf(buf, "ps -A -o state > %s",
	    psstat_f);
    argv[2] = buf;
    argv[3] = 0;
    execv("/bin/sh", argv);
    exit(RET_ERROR);
  default:
    if (waitpid(cpid, &status, 0) == -1) {
      sprintf(buf, "[ getProcesses() ] The number of processes could not be determined");
      throw runtime_error(buf); 
    }
  }

  pf = fopen(psstat_f, "rt");
  if (pf == NULL) {
    unlink(psstat_f);
    sprintf(buf, "[ getProcesses() ] The number of processes could not be determined");
    throw runtime_error(buf);
  } 

  processes = 0;
  // the states table keeps an entry for each alphabet letter, for efficient 
  // indexing
  for (int i = 0; i < NLETTERS; i++)
    states[i] = 0.0;
  while (fgets(buf, 10, pf) > 0) {
    ch = buf[0];
    states[ch - 65]++;
    processes++;
  }

  fclose(pf);   
  unlink(psstat_f);
#endif
}

void ProcUtils::getSysMem(double &totalMem, double &totalSwap) 
  throw(procutils_error) {
#ifndef WIN32
  char s1[20], line[MAX_STRING_LEN];
  bool memFound = false, swapFound = false;
  double valMem, valSwap;
  FILE *fp1;

  fp1 = fopen("/proc/meminfo", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getSysMem() ] Could not open /proc/meminfo");

  while (fgets(line, MAX_STRING_LEN, fp1)) {
    if (strstr(line, "MemTotal:") == line) {
      sscanf(line, "%s %lf", s1, &valMem);
      memFound = true;
      continue;
    }

    if (strstr(line, "SwapTotal:") == line) {
      sscanf(line, "%s %lf", s1, &valSwap);
      swapFound = true;
      continue;
    }
    
  }
  fclose(fp1); 

  if (!memFound || !swapFound)
    throw procutils_error((char*)"[ getSysMem() ] Could not obtain memory info from /proc/meminfo");
  totalMem = valMem;
  totalSwap = valSwap;
#endif
}

void ProcUtils::getMemUsed(double &usedMem, double& freeMem, 
				  double &usedSwap, double& freeSwap) 
  throw(procutils_error) {
#ifndef WIN32
  double mFree = 0, mTotal = 0, sFree = 0, sTotal = 0;
  char s1[20], line[MAX_STRING_LEN];
  bool mFreeFound = false, mTotalFound = false;
  bool sFreeFound = false, sTotalFound = false;
  FILE *fp1;

  fp1 = fopen("/proc/meminfo", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getMemUsed() ] Could not open /proc/meminfo");

  while (fgets(line, MAX_STRING_LEN, fp1)) {
    if (strstr(line, "MemTotal:") == line) {
      sscanf(line, "%s %lf", s1, &mTotal);
      mTotalFound = true;
      continue;
    }

    if (strstr(line, "MemFree:") == line) {
      sscanf(line, "%s %lf", s1, &mFree);
      mFreeFound = true;
      continue;
    }

    if (strstr(line, "SwapTotal:") == line) {
      sscanf(line, "%s %lf", s1, &sTotal);
      sTotalFound = true;
      continue;
    }

    if (strstr(line, "SwapFree:") == line) {
      sscanf(line, "%s %lf", s1, &sFree);
      sFreeFound = true;
      continue;
    }
    
  }
  fclose(fp1); 

  if (!mFreeFound || !mTotalFound || !sFreeFound || !sTotalFound)
    throw procutils_error((char*)"[ getMemUsed() ] Could not obtain memory info from /proc/meminfo");

  usedMem = (mTotal - mFree) / 1024;
  freeMem = mFree / 1024;
  usedSwap = (sTotal - sFree) / 1024;
  freeSwap = sFree / 1024;
#endif
}

void ProcUtils::getNetworkInterfaces(int &nInterfaces, 
		      char names[][20]) throw(procutils_error) {
#ifndef WIN32
  char line[MAX_STRING_LEN], *tmp;
//  char buf[MAX_STRING_LEN];
//  char *pbuf = buf;
  FILE *fp1;

  nInterfaces = 0;

  fp1 = fopen("/proc/net/dev", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getMemUsed() ] Could not open /proc/net/dev");
  while (fgets(line, MAX_STRING_LEN, fp1)) {
    if (strchr(line, ':') == NULL)
      continue;

    tmp = strtok/*_r*/(line, " :");//, &pbuf);

    if (strcmp(tmp, "lo") == 0)
      continue;
    
    strcpy(names[nInterfaces], tmp);
    nInterfaces++;
  }
    
  fclose(fp1);
#endif
}

void ProcUtils::getNetInfo(ApMon& apm, double **vNetIn, 
				  double **vNetOut, double **vNetErrs) 
  throw(runtime_error, procutils_error) {
#ifndef WIN32
  double *netIn, *netOut, *netErrs, bytesReceived, bytesSent;
  int errs;
  char line[MAX_STRING_LEN], msg[MAX_STRING_LEN];
//  char buf[MAX_STRING_LEN];
//  char *pbuf = buf;
  char *tmp, *tok;
  double bootTime = 0;
  FILE *fp1;
  time_t crtTime = time(NULL);
  int ind, i;

  if (apm.lastSysInfoSend == 0) {
    try {
      bootTime = getBootTime();
    } catch (procutils_error& err) {
      logger(WARNING, "[ getNetInfo() ] Error obtaining boot time. The first system monitoring datagram will contain incorrect data.");
      bootTime = 0;
    }
  }

  if (crtTime <= apm.lastSysInfoSend)
    throw runtime_error("[ getNetInfo() ] Current time <= time of the previous sysInfoSend");

  fp1 = fopen("/proc/net/dev", "r");
  if (fp1 == NULL)
    throw procutils_error((char*)"[ getNetInfo() ] Could not open /proc/net/dev");

  netIn = (double *)malloc(apm.nInterfaces * sizeof(double));
  netOut = (double *)malloc(apm.nInterfaces * sizeof(double));
  netErrs = (double *)malloc(apm.nInterfaces * sizeof(double));

  while (fgets(line, MAX_STRING_LEN, fp1)) {
    if (strchr(line, ':') == NULL)
      continue;
    tmp = strtok/*_r*/(line, " :");//, &pbuf);
    
    /* the loopback interface is not considered */
    if (strcmp(tmp, "lo") == 0)
      continue;

    /* find the index of the interface in the vector */
    ind = -1;
    for (i = 0; i < apm.nInterfaces; i++)
      if (strcmp(apm.interfaceNames[i], tmp) == 0) {
	ind = i;
	break;
      }
    
    if (ind < 0) {
      fclose(fp1);
      free(netIn); free(netOut); free(netErrs);
      sprintf(msg, "[ getNetInfo() ] Could not find interface %s in /proc/net/dev", 
	      tmp);  
      throw runtime_error(msg);
    }

    /* parse the rest of the line */
    tok = strtok/*_r*/(NULL, " ");//, &pbuf);
    bytesReceived = atof(tok); /* bytes received */
    tok = strtok/*_r*/(NULL, " ");//, &pbuf); /* packets received */
    tok = strtok/*_r*/(NULL, " ");//, &pbuf); /* input errors */
    errs = atoi(tok);
    /* some parameters that we are not monitoring */
    for (i = 1; i <= 5; i++)
      tok = strtok/*_r*/(NULL, " ");//, &pbuf);

    tok = strtok/*_r*/(NULL, " ");//, &pbuf); /* bytes transmitted */
    bytesSent = atof(tok);
    tok = strtok/*_r*/(NULL, " ");//, &pbuf); /* packets transmitted */
    tok = strtok/*_r*/(NULL, " ");//, &pbuf); /* output errors */
    errs += atoi(tok);

    //printf("### bytesReceived %lf lastRecv %lf\n", bytesReceived, 
    // apm.lastBytesReceived[ind]); 
    if (bytesReceived < apm.lastBytesReceived[ind] || bytesSent < 
	apm.lastBytesSent[ind] || errs < apm.lastNetErrs[ind]) {
      apm.lastBytesReceived[ind] = bytesReceived;
      apm.lastBytesSent[ind] = bytesSent;
      apm.lastNetErrs[ind] = errs;
      fclose(fp1);
      free(netIn); free(netOut); free(netErrs);
      throw runtime_error("[ getNetInfo() ] Network interface(s) restarted.");
    }

    if (apm.lastSysInfoSend == 0) {
      netIn[ind] = bytesReceived/(crtTime - bootTime);
      netOut[ind] = bytesSent/(crtTime - bootTime);
      netErrs[ind] = errs;
    }
    else {
      netIn[ind] = (bytesReceived - apm.lastBytesReceived[ind]) / (crtTime -
						     apm.lastSysInfoSend);
      netIn[ind] /= 1024; /* netIn is measured in KBps */
      netOut[ind] = (bytesSent - apm.lastBytesSent[ind]) / (crtTime - 
						     apm.lastSysInfoSend);
      netOut[ind] /= 1024; /* netOut is measured in KBps */
      /* for network errors give the total number */
      netErrs[ind] = errs; // - apm.lastNetErrs[ind];
    }

    apm.lastBytesReceived[ind] = bytesReceived;
    apm.lastBytesSent[ind] = bytesSent;
    apm.lastNetErrs[ind] = errs;
  }
    
  fclose(fp1);
  *vNetIn = netIn;
  *vNetOut = netOut;
  *vNetErrs = netErrs;
#endif
 }

int ProcUtils::getNumCPUs() throw(procutils_error) {
#ifdef WIN32
	return 0;
#else
  int numCPUs = 0;
  char line[MAX_STRING_LEN];

  FILE *fp = fopen("/proc/stat", "r");

  if (fp == NULL)
    throw procutils_error((char*)"[ getNumCPUs() ] Could not open /proc/stat.");

  while(fgets(line, MAX_STRING_LEN, fp)) {
    if (strstr(line, "cpu") == line && isdigit(line[3]))
      numCPUs++;
  }

  fclose(fp);
  return numCPUs;
#endif
}

void ProcUtils::getCPUInfo(ApMon& apm) throw(procutils_error) {
#ifndef WIN32
  double freq = 0;
  char line[MAX_STRING_LEN], s1[100], s2[100], s3[100];
//  char buf[MAX_STRING_LEN];
//  char *pbuf = buf;
  char *tmp, *tmp_trim;
  bool freqFound = false, bogomipsFound = false;

  FILE *fp = fopen("/proc/cpuinfo", "r");
  if (fp == NULL)
    throw procutils_error((char*)"[ getCPUInfo() ] Could not open /proc/cpuinfo");

  while (fgets(line, MAX_STRING_LEN, fp)) {
    if (strstr(line, "cpu MHz") == line) {
      sscanf(line, "%s %s %s %lf", s1, s2, s3, &freq);
      apm.currentGenVals[GEN_CPU_MHZ] = freq;
      freqFound = true;
      continue;
    }

    if (strstr(line, "bogomips") == line) {
      sscanf(line, "%s %s %lf", s1, s2, &(apm.currentGenVals[GEN_BOGOMIPS]));
      bogomipsFound = true;
      continue;
    }

    if (strstr(line, "vendor_id") == line) {
      tmp = strtok/*_r*/(line, ":");//, &pbuf);
      /* take everything that's after the ":" */
      tmp = strtok/*_r*/(NULL, ":");//, &pbuf);
      tmp_trim = trimString(tmp);
      strcpy(apm.cpuVendor, tmp_trim);
      free(tmp_trim);
      continue;
    } 

    if (strstr(line, "cpu family") == line) {
      tmp = strtok/*_r*/(line, ":");//, &pbuf);
      tmp = strtok/*_r*/(NULL, ":");//, &pbuf);
      tmp_trim = trimString(tmp);
      strcpy(apm.cpuFamily, tmp_trim);
      free(tmp_trim);
      continue;
    }

    if (strstr(line, "model") == line && strstr(line, "model name") != line) {
      tmp = strtok/*_r*/(line, ":");//, &pbuf);
      tmp = strtok/*_r*/(NULL, ":");//, &pbuf);
      tmp_trim = trimString(tmp);
      strcpy(apm.cpuModel, tmp_trim);
      free(tmp_trim);
      continue;
    }  

    if (strstr(line, "model name") == line) {
      tmp = strtok/*_r*/(line, ":");//, &pbuf);
      /* take everything that's after the ":" */
      tmp = strtok/*_r*/(NULL, ":");//, &pbuf);
      tmp_trim = trimString(tmp);
      strcpy(apm.cpuModelName, tmp_trim);
      free(tmp_trim);
      continue;
    } 
  }

  fclose(fp);
  if (!freqFound || !bogomipsFound)
    throw procutils_error((char*)"[ getCPUInfo() ] Could not find frequency or bogomips in /proc/cpuinfo");
#endif
}

/**
 * Returns the system boot time in milliseconds since the Epoch.
 */
long ProcUtils::getBootTime() throw (procutils_error) {
#ifdef WIN32
	return 0;
#else
  char line[MAX_STRING_LEN], s[MAX_STRING_LEN];
  long btime = 0;
  FILE *fp = fopen("/proc/stat", "rt");
  if (fp == NULL)
    throw procutils_error((char*)"[ getBootTime() ] Could not open /proc/stat");

  while (fgets(line, MAX_STRING_LEN, fp)) {
    if (strstr(line, "btime") == line) {
      sscanf(line, "%s %ld", s, &btime);
      break;
    }
  }  
  fclose(fp);
  if (btime == 0)
    throw procutils_error((char*)"[ getBootTime() ] Could not find boot time in /proc/stat");
  return btime;
#endif
}


double ProcUtils::getUpTime() throw(procutils_error) {
#ifdef WIN32
	return 0;
#else
  double uptime = 0;
  FILE *fp = fopen("/proc/uptime", "rt");
  if (fp == NULL) {
    throw procutils_error((char*)"[ getUpTime() ] Could not open /proc/uptime");
  }

  fscanf(fp, "%lf", &uptime);
  fclose(fp);

  if (uptime <= 0) {
    throw procutils_error((char*)"[ getUpTime() ] Could not find uptime in /proc/uptime");
  }
  return uptime / (24 * 3600);
#endif
}

int ProcUtils::countOpenFiles(long pid) throw(procutils_error) {
#ifdef WIN32
	return 0;
#else
  char dirname[50];
  char msg[MAX_STRING_LEN];
  DIR *dir;
  struct dirent *dir_entry;
  int cnt = 0;
 
  /* in /proc/<pid>/fd/ there is an entry for each opened file descriptor */
  sprintf(dirname, "/proc/%ld/fd", pid);
  dir = opendir(dirname);
  if (dir == NULL) {
    sprintf(msg, "[ countOpenFiles() ] Could not open %s", dirname); 
    throw procutils_error(msg);
  }

  /* count the files from /proc/<pid>/fd/ */
  while ((dir_entry = readdir(dir)) != NULL) {
    cnt++;
  }
  
  closedir(dir);

  /* don't take into account . and .. */
  cnt -= 2;
  if (cnt < 0) {
    sprintf(msg, "[ countOpenFiles() ] Directory %s has less than 2 entries", 
	    dirname);
    logger(FINE, msg);
    cnt = 0;
  }

  return cnt;
#endif
}

void ProcUtils::getNetstatInfo(ApMon& apm, double nsockets[], 
				      double tcp_states[]) 
  throw(runtime_error) {

  // the states table keeps an entry for each alphabet letter, for efficient 
  // indexing
  int i;
  for (i = 0; i < 4; i++)
    nsockets[i] = 0.0;
  for (i = 0; i < N_TCP_STATES; i++)
    tcp_states[i] = 0.0;

#ifndef WIN32
  char *argv[4];
  char netstat_f[40];
  pid_t mypid = getpid();
  pid_t cpid;
  int status, idx;
  char /*ch,*/ buf[100], msg[100];
  char *pbuf = buf, *tmp, *tmp2;
  FILE *pf;

  sprintf(netstat_f, "/tmp/apmon_netstat%d", mypid);

  switch (cpid = fork()) {
  case -1:
    throw runtime_error("[ getNetstatInfo() ] Unable to fork()");
  case 0:
    argv[0] = (char *)"/bin/sh"; argv[1] = (char *)"-c";
    sprintf(buf, "netstat -an > %s",
	    netstat_f);
    argv[2] = buf;
    argv[3] = 0;
    execv("/bin/sh", argv);
    exit(RET_ERROR);
  default:
    if (waitpid(cpid, &status, 0) == -1) {
      sprintf(msg, "[ getNetstatInfo() ] The netstat information could not be collected");
      throw runtime_error(msg); 
    }
  }

  pf = fopen(netstat_f, "rt");
  if (pf == NULL) {
    unlink(netstat_f);
    sprintf(msg, "[ getNetstatInfo() ] The netstat information could not be collected");
    throw runtime_error(msg);
  } 

  while (fgets(buf, 200, pf) > 0) {
    tmp = strtok_r(buf, " \t\n", &pbuf);
    if (strstr(tmp, "tcp") == tmp) {
      nsockets[SOCK_TCP]++;

      /* go to the "State" field */
      for (i = 1; i <= 5; i++)
	tmp2 = strtok_r(NULL, " \t\n", &pbuf);

      idx = getVectIndex(tmp2, apm.socketStatesMapTCP, N_TCP_STATES);
      if (idx >= 0) {
	tcp_states[idx]++;
      } else {
	sprintf(msg, "[ getNestatInfo() ] Invalid socket state: %s q", tmp2);
	logger(WARNING, msg);
      }
    } else {
      if (strstr(tmp, "udp") == tmp) {
	nsockets[SOCK_UDP]++;
      } else {
	if (strstr(tmp, "unix") == tmp)
	  nsockets[SOCK_UNIX]++;
	else if (strstr(tmp, "icm") == tmp)
	  nsockets[SOCK_ICM]++;
      }
    }
  }

  fclose(pf);   
  unlink(netstat_f);
#endif
}
