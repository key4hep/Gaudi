#ifndef GAUDIKERNEL_DATALISTENERSVC_H
#define GAUDIKERNEL_DATALISTENERSVC_H


#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IMonitorSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "ValueMap.h"
#include "XmlCode.h"
#include "ApMon.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <map>
#include <set>


#include <stdio.h>


// Forward declarations
class ISvcLocator;

/** @class DataListenerSvc DataListenerSvc.h bking/DataListenerSvc.h
 *
 *  DataListenerSvc inherits from IMonitorSvc and allows the GAUDI user to monitor
 *  variables declared in an algorithm. DataListenerSvc generates XML files on the
 *  individual machines where the algorithm is run, which can then be parsed by e.g.
 *  GANGA and sent to a monitoring server such as MonALISA. XML files are written at
 *  a frequency specified by the user and are consumed as they are created when used
 *  within GANGA.
 *
 *  @subpage Usage
 *
 *  \par In Algorithm header file
 *
 *  \li Include DataListenerSvc <tt> #include DataListenerSvc/DataListenerSvc.h </tt>
 *  \li define IMonitorSvc object: e.g. <tt> IMonitorSvc *m_DataListenerSvcSvc; </tt>
 *
 *  \par In Algorithm cpp file
 *
 *  \li instantiate IMonitorSvc object: e.g. <tt> sc = service("DataListenerSvc", m_DataListenerSvcSvc, true);</tt>
 *  \li declare monitored variables: e.g.
 * <tt>  m_DataListenerSvcSvc->declareInfo("Psi ID", m_jPsiID, "The Psi ID", this); </tt> (for variable \c m_jPsiID)
 *
 *  Supports declaration of variables of type: <tt> int, long, bool, double, string </tt>
 *
 *  \par In GAUDI options file
 *
 *  \li Include DLL in ApplicationMgr.DLLs \c DataListenerSvc
 *  \li Include Service in ApplicationMgr.ExtSvc \c DataListenerSvc/DataListenerSvc
 *  \li Set job options e.g. <tt> DataListenerSvc.EventFrequency = 50 </tt>
 *
 *  \par Job options available
 *  @param EventFrequency How data is sent to - default is <em> every 100 events </em>
 *
 *  <hr>
 *  \par XML file format
 *
 *
 *  XML tag format is the following:
 *
 *  Opening parent tags of the form:
 *
 *  <tt>\<?xml version="1.0" encoding="ASCII" standalone="yes"?\></tt>\n
 *  <tt>\<Results Time="Tue Aug 22 10:17:22 2006"\></tt>
 *  <table border=0>
 *  <tr>
 *    <td>&nbsp; &nbsp; &nbsp; &nbsp;</td>
 *    <td><tt>\<Run\></tt></td>
 * </tr>
 * </table>
 *
 *  With then an entry for each monitored variable of the form:\n
 *
 *
 *  <table border=0>
 *  <tr>
 *    <td>&nbsp; &nbsp; &nbsp; &nbsp;</td>
 *    <td><tt>\<Incident\></tt> </td>
 *    <td></td>
 *  </tr>
 *  <tr>
 *    <td>&nbsp; &nbsp; &nbsp; &nbsp;</td>
 *    <td>&nbsp; &nbsp; &nbsp; &nbsp;</td>
 *    <td>
 *      <tt>\<IncType\>EndEvent          \</IncType\> </tt> \n
 *      <tt>\<IncSource\>EventLoopMgr      \</IncSource\> </tt>\n
 *      <tt>\<VarName\>Psi ID          \</VarName\> </tt>\n
 *      <tt>\<Value Result="8"\>443\</Value\> </tt>
 *   </td>
 * </tr>
 * <tr>
 *   <td>&nbsp; &nbsp; &nbsp; &nbsp;</td>
 *   <td><tt>\</Incident\></tt> </td>
 *   <td></td>
 * </tr>
 * </table>
 *
 *  @param IncType What type of incident triggered the handle method
 *  @param IncSource What triggered the IIncident handle method
 *  @param VarName Name of the variable monitored
 *  @param Value Value of the variable
 *  @param Result Number of times the variable already read
 *
 *  @author Ben KING
 *  @date   2006-08-24
 */

class DataListenerSvc : public extends2<Service, IMonitorSvc, IIncidentListener> {
public:
  DataListenerSvc(const std::string& name, ISvcLocator* sl);

  virtual ~DataListenerSvc() {}

  // Service pure virtual member functions
  StatusCode initialize();
  StatusCode finalize();

  /** Declare monitoring information
      @param name Monitoring information name known to the external system
      @param var Monitoring Listener address
      @param desc Textual description
      @param owner Owner identifier of the monitoring information
      (needed to perform clean up
  */
  void declareInfo(const std::string& name, const bool&  var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const int&  var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const long&  var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const double& var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::string& var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::pair<double,double>&var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const AIDA::IBaseHistogram* var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const StatEntity& var,
                   const std::string& desc, const IInterface* owner) ;
  void declareInfo(const std::string& name, const std::string& format,
		   const void * var, int size, const std::string& desc,
		   const IInterface* owner) ;

  // Declaration of declareProperty which is virtual in one of my includes
  // (that would be service.h)
  //  StatusCode declareProperty(const std::string& name, const int& var);


  /** Undeclare monitoring information
      @param name Monitoring information name known to the external system
      @param owner Owner identifier of the monitoring information
  */
  void undeclareInfo( const std::string& name, const IInterface* owner ) ;

  /** Undeclare monitoring information
      @param owner Owner identifier of the monitoring information
  */
  void undeclareAll( const IInterface* owner ) ;

  /// Implementation of the IIncidentListener.h virtual method
  virtual void handle(const Incident&);


  /** Get the names for all declared monitoring informations for a given
      owner. If the owner is NULL, then it returns for all owners
  */
  std::set<std::string> * getInfos(const IInterface* owner = 0);


  /** Send (key,value) pairs to MonALISA server
      @param clusterName Name of cluster where information will be stored
      @param moduleName Name of module where information will be stored
      @param key Key of the (key,value) pair sent to MonALISA
      @param value Value of the (key,value) pair sent to MonALISA
  */
  /// Send a (char*,double) pair to MonALISA
  void apmSend(char* clusterName, char* moduleName, char* key, double val);
  /// Send a (char*,int) pair to MonALISA
  void apmSend(char* clusterName, char* moduleName, char* key, int val);
  /// Send a (char*,long) pair to MonALISA
  void apmSend(char* clusterName, char* moduleName, char* key, long val);
  /// Send a (char*,bool) pair to MonALISA
  void apmSend(char* clusterName, char* moduleName, char* key, bool val);
  /// Send a (char*,char*) pair to MonALISA
  void apmSend(char* clusterName, char* moduleName, char* key, char* val);

private:


  ApMon *apm;


  // ValueMap stores declared data information
  ValueMap m_ValueMap;
  ValueMap m_ValueMapTemp;

  time_t rawTime;
  time_t rawTime2;

  char buffer[50];
  char buffer2[50];
  std::string timeDate;
  std::string timeDate2;
  std::string valToWrite;


  // Used to hold variable name and data information
  typedef std::pair< std::string, ValueMap > Entry;
  typedef std::map<const IInterface*, std::set<std::string> > InfoNamesMap;

  typedef std::map< std::string, ValueMap > ValNamesMap;

  void writeMap(ValNamesMap*, const Incident&, std::ofstream*);
  void writeXML(ValNamesMap*, const Incident&, std::ofstream*);
  std::string resizeString ( const std::string, const unsigned int );

  // Converts types INTO a string with a stringstream
  std::string stringConverter ( const int convertee );
  std::string stringConverter ( const double convertee );
  std::string stringConverter ( const bool convertee );
  std::string stringConverter ( const long convertee );



  InfoNamesMap m_InfoNamesMap;
  InfoNamesMap::iterator m_InfoNamesMapIt;

  ValNamesMap m_ValNamesMap;
  ValNamesMap::iterator m_ValNamesMapIt;
  XmlCode xml;


  // Map associating to each algorithm name a set with the info
  // names from this algorithm
  IIncidentSvc* m_incidentSvc;
  FILE* MonitorLog;
  std::ofstream Log4;
  std::ofstream XMLLog;


  int FreqCriterion;
  int m_EventFrequency;


  /// MonALISA sender method to send Data to MonALISA server
  void sendData(ValNamesMap*);

  // The container below is necessary because usually the descriptions of
  // the services are constant strings, not variables. DimService expects
  // a variable address to monitor as its second argument and we are using
  // a DimService for strings to publish the descriptions.
  std::map<std::string,std::string> m_infoDescriptions;
  std::string infoOwnerName( const IInterface* owner );

  // MonALISA optionsfile variables
  int m_MLjobID;
  int m_MLEventFrequency;
  std::string m_MLClusterName;
  std::string m_ApMonSettingsFile;
  std::string jobIDString;
  int m_MonALISAMonitoring;
  int m_XMLMonitoring;
  int m_FileMonitoring;


  char *keyToSend;


};

#endif // GAUDIKERNEL_DATALISTENERSVC_H
