// $Id: DataListenerSvc.cpp,v 1.6 2008/10/27 19:22:21 marcocle Exp $
// Include files
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "DataListenerSvc.h"
#include "ValueMap.h"
#include "XmlCode.h"
#ifdef _WIN32
#define NOMSG
#define NOGDI
#include <windows.h>
#include <winsock.h>
#include <process.h>
#undef NOMSG
#undef NOGDI
#else
#include <unistd.h>
#include <exception>
#endif
#include <sys/types.h>

/********************************************
 *
 * This program implements DataListenerSvc.h which supplies a handle method for the declareInfo service.
 * Given options in the optsfile, txt and XML files containing the variable names and their values
 * are written out for the last read record specified by the user.
 *
 * Currently there are no undeclare or getInfo methods
 *
 * Author: Ben King
 *
 ********************************************/


// Factory for instantiation of service objects
DECLARE_SERVICE_FACTORY(DataListenerSvc)

namespace  {
  // num of Incidents counter
  int numIncidents = 0;
  int defaultMLFrequency = 100;
  int defaultjobID = 0;
  std::string defaultClusterName = "DataListenerSvc";
  std::string defaultApMonSettingsFile = "http://ganga.web.cern.ch/ganga/monalisa.conf";
  int defaultMonALISAMonitoring = 1;
  int defaultXMLMonitoring = 1;
  int defaultFileMonitoring = 1;

  // fileNum counter
  int fileCounter = 0;
  int defaultFrequency = 100;
}

// Constructor
DataListenerSvc::DataListenerSvc(const std::string& name, ISvcLocator* sl)
  : base_class(name, sl) {

  declareProperty ("EventFrequency", m_EventFrequency=defaultFrequency);
  declareProperty ("MLEventFrequency", m_MLEventFrequency=defaultMLFrequency);
  declareProperty ("MLjobID", m_MLjobID=defaultjobID);
  declareProperty ("MLClusterName", m_MLClusterName=defaultClusterName);
  declareProperty ("MonALISAMonitoring", m_MonALISAMonitoring=defaultMonALISAMonitoring);
  declareProperty ("ApMonSource", m_ApMonSettingsFile=defaultApMonSettingsFile);

  declareProperty ("XMLMonitoring", m_XMLMonitoring=defaultXMLMonitoring);
  declareProperty ("FileMonitoring", m_FileMonitoring=defaultFileMonitoring);

}

// implement Service methods
StatusCode DataListenerSvc::initialize()
{

  m_ApMonSettingsFile = defaultApMonSettingsFile;
  StatusCode sc = Service::initialize();
  MsgStream msg(msgSvc(),name());

  jobIDString = "jobID: " + stringConverter(m_MLjobID);

  if ( !sc.isSuccess() )   {
    msg << MSG::ERROR << "Failed to initialize Service base class." << endmsg;
    return StatusCode::FAILURE;
  }

  sc = service("IncidentSvc", m_incidentSvc, true);
  if (!sc.isSuccess()) {
    msg << MSG::ERROR << "Failed to access incident service." << endmsg;
    return StatusCode::FAILURE;
  }
  m_incidentSvc->addListener(this, IncidentType::EndEvent);

  if (m_MonALISAMonitoring){
    // Initialise MonALISA
    apm = new ApMon(const_cast<char*>(m_ApMonSettingsFile.c_str()));
  }

  // DEBUG info on job options specified in GAUDI
  if (m_MLEventFrequency == defaultMLFrequency) {
    msg << MSG::DEBUG
        << "Frequency to send data to MonALISA not specified, taken as the default value: "
        << defaultMLFrequency << endmsg;
  } else {
    msg << MSG::INFO
        << "Data sent to MonALISA every "
        << m_MLEventFrequency << " events" << endmsg;
  }

  if (m_MLjobID == defaultjobID){
    msg << MSG::DEBUG
        << "DataListenerSvc MonALISA Job ID not specified, taken as the default value: "
        << defaultjobID << endmsg;
  }

  if (m_MLClusterName == defaultClusterName){
    msg << MSG::DEBUG
        << "DataListenerSvc MonALISA cluster name not specified, taken as the default value: "
        << defaultClusterName << endmsg;
  }

  if (m_ApMonSettingsFile == defaultApMonSettingsFile){
    msg << MSG::DEBUG
        << "MonALISA ApMon configuration file destination not specified,  will be read from: "
        << defaultClusterName << endmsg;
  }

  if (m_EventFrequency == defaultFrequency){
    msg << MSG::INFO
        << "XML Log file frequency not specified, taken as the default value: "
        << defaultFrequency << endmsg;
  } else {
    msg << MSG::INFO
        << "Frequency XML logs are written at: "
        << m_EventFrequency << endmsg;
  }

  if (m_MonALISAMonitoring) {
    msg << MSG::INFO
	<< "Data will be published to a MonALISA server"
	<< endmsg;
  }

  if (m_XMLMonitoring) {
    msg << MSG::INFO
	<< "Data will be written to XML log files"
	<< endmsg;
  }
  if (m_FileMonitoring) {
    msg << MSG::INFO
	<< "Data will be updated in a Text log file"
	<< endmsg;
  }

  return StatusCode::SUCCESS;
}


StatusCode DataListenerSvc::finalize()
{
  MsgStream msg(msgSvc(),"DataListenerSvc");

  m_infoDescriptions.clear();
  m_ValNamesMap.clear();

  if (m_MonALISAMonitoring){
    if ( 0!= apm ) {
      delete apm;
      apm = 0 ;
    }
  }

  msg << MSG::DEBUG << "ApMon deleted" << endmsg;
  msg << MSG::INFO << "finalized successfully" << endmsg;

  if( m_incidentSvc ) m_incidentSvc->release();

  return Service::finalize();
}

void DataListenerSvc::declareInfo(const std::string& name, const double& var,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
  ValueMap::m_type vartype;
  vartype = ValueMap::m_double;

  MsgStream msg(msgSvc(),"DataListenerSvc");

  // In the ValueMap, store the pointer to the data (converted to void*)
  // and also store the pointer type (with an enum) so it can be converted back
  m_ValueMap.set_ptr(  static_cast <void*> (const_cast <double*> (&var) )  );
  m_ValueMap.set_ptrType(vartype);

  Entry p1(name, m_ValueMap);

  // m_ValNamesMap is then composed of n (variable Name: ValueMap) pairs
  m_ValNamesMap.insert(p1);

}

void DataListenerSvc::declareInfo(const std::string& name, const long& var,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{

  ValueMap::m_type vartype;
  vartype = ValueMap::m_long;

  MsgStream msg(msgSvc(),"DataListenerSvc");

  m_ValueMap.set_ptr(  static_cast <void*> (const_cast <long*> (&var) )  );
  m_ValueMap.set_ptrType(vartype);

  Entry p1(name, m_ValueMap);

  m_ValNamesMap.insert(p1);

}

void DataListenerSvc::declareInfo(const std::string& name, const int& var,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
  ValueMap::m_type vartype;
  vartype = ValueMap::m_int;

  MsgStream msg(msgSvc(),"DataListenerSvc");

  m_ValueMap.set_ptr(  static_cast <void*> (const_cast <int*> (&var) )  );
  m_ValueMap.set_ptrType(vartype);

  Entry p1(name, m_ValueMap);

  m_ValNamesMap.insert(p1);

}

void DataListenerSvc::declareInfo(const std::string& name, const bool& var,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{

  ValueMap::m_type vartype;
  vartype = ValueMap::m_bool;

  MsgStream msg(msgSvc(),"DataListenerSvc");

  m_ValueMap.set_ptr(  static_cast <void*> (const_cast <bool*> (&var) )  );
  m_ValueMap.set_ptrType(vartype);

  Entry p1(name, m_ValueMap);

  m_ValNamesMap.insert(p1);

}

void DataListenerSvc::declareInfo(const std::string& name,
				  const std::string& var,
				  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
  ValueMap::m_type vartype;
  vartype = ValueMap::m_string;

  MsgStream msg(msgSvc(),"DataListenerSvc");

  m_ValueMap.set_ptr(  static_cast <void*> (const_cast <std::string*> (&var) )  );  m_ValueMap.set_ptrType(vartype);

  Entry p1(name, m_ValueMap);

  m_ValNamesMap.insert(p1);

}

void DataListenerSvc::declareInfo(const std::string& /*name*/,
                                  const std::pair<double,double>& /*var*/,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
}

void DataListenerSvc::declareInfo(const std::string& /*name*/,
                                  const StatEntity& /*var*/,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
}

void DataListenerSvc::declareInfo(const std::string& /*name*/,
                                  const AIDA::IBaseHistogram* /*var*/,
                                  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
}

void DataListenerSvc::declareInfo(const std::string& /*name*/,
				  const std::string& /*format*/,
				  const void* /*var*/,
				  int /*size*/,
				  const std::string& /*desc*/,
				  const IInterface* /*owner*/)
{
}

void DataListenerSvc::undeclareInfo( const std::string& /*name*/,
                                     const IInterface* /*owner*/ )
{
}

void DataListenerSvc::undeclareAll( const IInterface* /*owner*/ )
{
}

std::set<std::string> * DataListenerSvc::getInfos(const IInterface* /*owner*/ )
{
  std::set<std::string> * returnData = NULL;
  return returnData;
}

std::string DataListenerSvc::infoOwnerName( const IInterface* /*owner*/ )
{
  std::string returnData = "";
  return returnData;
}

void DataListenerSvc::handle (const Incident& Inc)
{

  MsgStream msg(msgSvc(),"DataListenerSvc");
  numIncidents++;


  if (numIncidents % m_EventFrequency == 0){

    try {
      Log4.open("MonitorLog.txt", std::ios::trunc);
      XMLLog.open(("./log" + stringConverter(fileCounter)  + ".xml").c_str() , std::ios::trunc);
    } catch(std::exception /*&e*/) {
      msg << MSG::INFO
	  << "Log File could not be opened, no monitoring available"
	  << endmsg;
    }

    if (m_MonALISAMonitoring){
      // Send Data to MonALISA
      sendData(&m_ValNamesMap);
    }

    if (m_FileMonitoring){
      // Write simple text log
      writeMap(&m_ValNamesMap, Inc, &Log4 );
      Log4.close();
    }

    if (m_XMLMonitoring){
      // Write XML files
      writeXML(&m_ValNamesMap, Inc, &XMLLog );
      XMLLog.close();
    }

    fileCounter++;
  }

}

// With a String as argument, resize with whitespace until specified length
std::string DataListenerSvc::resizeString ( const std::string text,
					    const unsigned int length)
{
  std::string temp;
  unsigned int counter=0;

  do {

    if (text.length() > length) {
      temp = text.substr(0,(length-1));
    } else {
      if (counter==0) {
	temp = text;
	counter++;
      }
      temp.append(" ");
    }

  } while (temp.length() != length);

  return temp;
}

// Convert argument to a string so that can format for text file
std::string DataListenerSvc::stringConverter (const int convertee)
{
  std::stringstream s;
  std::string temp;
  s << convertee;
  s >> temp;
  s.flush();
  return temp;
}

std::string DataListenerSvc::stringConverter (const double convertee)
{
  std::stringstream s;
  std::string temp;
  s << convertee;
  s >> temp;
  s.flush();
  return temp;
}

std::string DataListenerSvc::stringConverter (const long convertee)
{
  std::stringstream s;
  std::string temp;
  s << convertee;
  s >> temp;
  s.flush();
  return temp;
}

std::string DataListenerSvc::stringConverter (const bool convertee)
{
  std::stringstream s;
  std::string temp;
  s << convertee;
  s >> temp;
  s.flush();
  return temp;
}


void DataListenerSvc::writeXML(ValNamesMap* m_ValNamesMap,
			       const Incident& Inc,
			       std::ofstream* MyFile)
{

  MsgStream msg(msgSvc(),"DataListenerSvc");

  // prepare time ctime.h and remove trailing carriage return
  time ( &rawTime );
  sprintf (buffer, "%s", ctime(&rawTime));
  timeDate = buffer;
  timeDate.erase(timeDate.size()-1, 1);


  msg << MSG::INFO << "XML written to file: " << MyFile << endmsg;


  *MyFile << xml.declaration("1.0", "ASCII", "yes") << std::endl;

  *MyFile << xml.tagBegin("Results", 0, "Time", timeDate)
          << xml.tagBegin("Run",1);


  for (m_ValNamesMapIt = m_ValNamesMap->begin();
       m_ValNamesMapIt != m_ValNamesMap->end();
       ++m_ValNamesMapIt) {

    m_ValueMapTemp = m_ValNamesMapIt->second;

    *MyFile << xml.tagBegin("Incident",2);

    *MyFile << xml.tagBegin("IncType", 3)
            << xml.data(resizeString(Inc.type(),18))
            << xml.tagEnd("IncType", 0);

    *MyFile << xml.tagBegin("IncSource", 3)
            << xml.data(resizeString(Inc.source(),18))
            << xml.tagEnd("IncSource", 0);

    *MyFile << xml.tagBegin("VarName", 3)
            << xml.data(resizeString(m_ValNamesMapIt->first,20))
            << xml.tagEnd("VarName", 0);

    *MyFile << xml.tagBegin("Value", 3, "Result", stringConverter(numIncidents));
    // Check the variable type and in each case, cast accordingly



    // re-convert pointers to their original form and then to strings
    // so that they can be written with XMLCode.h
    if (m_ValueMapTemp.get_ptrType() == ValueMap::m_double){
      valToWrite = stringConverter(*(double*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_int){
      valToWrite = stringConverter(*(int*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_long){
      valToWrite = stringConverter(*(long*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_bool){
      valToWrite = stringConverter(*(bool*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_string){
      valToWrite = *(std::string*)m_ValueMapTemp.get_ptr();
    }


    *MyFile << xml.data(valToWrite);
    msg << MSG::DEBUG << "XML written: " << m_ValNamesMapIt->first << " , "
        << valToWrite << endmsg;

    *MyFile << xml.tagEnd("Value", 0);
    *MyFile << xml.tagEnd("Incident",2);
  }


  *MyFile << xml.tagEnd("Run",1)
          << xml.tagEnd("Results", 0);

}


// Write the map to a standard text logfile
void DataListenerSvc::writeMap (ValNamesMap* m_ValNamesMap,
                   const Incident& Inc, std::ofstream* MyFile)
{
  // prepare time ctime.h and remove trailing carriage return
  time ( &rawTime2 );
  sprintf (buffer2, "%s", ctime(&rawTime2));
  timeDate2 = buffer2;
  timeDate2.erase(timeDate2.size()-1, 1);

  // headings for the text file with spacing numbers
  std::string space = "   ";
  std::string heading0 = "Time                       "; // 27
  std::string heading1 = "No      "; // 8 chars long
  std::string heading2 = "Incident Type     "; // 18
  std::string heading3 = "Incident Source   "; // 18
  std::string heading4 = "Name            ";  // 16
  std::string heading5 = "Value               "; // 20

  *MyFile << heading0 + heading1 + heading2 + heading3 + heading4 + heading5
	  << std::endl;

  for (m_ValNamesMapIt = m_ValNamesMap->begin();
       m_ValNamesMapIt != m_ValNamesMap->end();
       ++m_ValNamesMapIt){

    m_ValueMapTemp = m_ValNamesMapIt->second;

    *MyFile << timeDate2 << space

	    << resizeString(stringConverter(numIncidents),8) // Counts # incidents
	    << resizeString(Inc.type(),18)                 // Incident Type
	    << resizeString(Inc.source(),18)               // Source of the Incident
	    << resizeString(m_ValNamesMapIt->first,20);    // Variable Name

    // Check the variable type and in each case, cast accordingly
    if (m_ValueMapTemp.get_ptrType() == ValueMap::m_double){
      *MyFile << resizeString(stringConverter(*(double*)m_ValueMapTemp.get_ptr()), 12)
	      << std::endl;
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_int){
      *MyFile << resizeString(stringConverter(*(int*)m_ValueMapTemp.get_ptr()), 12)
	      << std::endl;
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_long){
      *MyFile << resizeString(stringConverter(*(long*)m_ValueMapTemp.get_ptr()), 12)
	      << std::endl;
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_bool){
      *MyFile << resizeString(stringConverter(*(bool*)m_ValueMapTemp.get_ptr()), 12)
	      << std::endl;
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_string){
      *MyFile << resizeString((*(std::string*)m_ValueMapTemp.get_ptr()), 12)
	      << std::endl;
      }

      }

}


void DataListenerSvc::sendData(ValNamesMap* m_ValNamesMap)
{

  for (m_ValNamesMapIt = m_ValNamesMap->begin();
       m_ValNamesMapIt != m_ValNamesMap->end();
       ++m_ValNamesMapIt){

    m_ValueMapTemp = m_ValNamesMapIt->second;
    keyToSend = const_cast<char*>((m_ValNamesMapIt->first).c_str());


    if (m_ValueMapTemp.get_ptrType() == ValueMap::m_double){
      apmSend(const_cast <char*>(m_MLClusterName.c_str()),
	      const_cast <char*> (jobIDString.c_str()),
	      keyToSend, *(double*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_int){
      apmSend(const_cast <char*>(m_MLClusterName.c_str()),
	      const_cast <char*> (jobIDString.c_str()),
	      keyToSend, *(int*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_long){
      apmSend(const_cast <char*>(m_MLClusterName.c_str()),
	      const_cast <char*> (jobIDString.c_str()),
	      keyToSend, *(long*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_bool){
      apmSend(const_cast <char*>(m_MLClusterName.c_str()),
	      const_cast <char*> (jobIDString.c_str()),
	      keyToSend, *(bool*)m_ValueMapTemp.get_ptr());
    } else if (m_ValueMapTemp.get_ptrType() == ValueMap::m_string){
      apmSend(const_cast <char*>(m_MLClusterName.c_str()),
	      const_cast <char*> (jobIDString.c_str()),
	      keyToSend, const_cast <char*>((*(std::string*)m_ValueMapTemp.get_ptr()).c_str()));
    }
  }

}


void DataListenerSvc::apmSend(char* clusterName, char* clusterNode,
			      char* key, double val)
{
  MsgStream msg(msgSvc(),"DataListenerSvc");
  msg << MSG::DEBUG << "ApMon instantiated" << endmsg;

  try {
    DataListenerSvc::apm->sendParameter( clusterName, clusterNode, key, val);
    msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
    msg << MSG::INFO << "Sent parameters to MonALISA sever:"
        << m_MLClusterName << "->"
        << jobIDString.c_str() << "->"
        << key << ":" << val << endmsg;
  } catch(std::runtime_error &e) {
    msg << MSG::WARNING << "WARNING sending to ApMon:\t" << e.what() << endmsg;
  }
}

void DataListenerSvc::apmSend(char* clusterName, char* clusterNode,
			      char* key, int val)
{
  MsgStream msg(msgSvc(),"DataListenerSvc");
  msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
  try {
    DataListenerSvc::apm->sendParameter( clusterName, clusterNode, key, val);
    msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
    msg << MSG::INFO << "Sent parameters to MonALISA sever:"
        << m_MLClusterName << "->"
        << jobIDString.c_str() << "->"
        << key << ":" << val << endmsg;
  } catch(std::runtime_error &e) {
    msg << MSG::WARNING << "WARNING sending to ApMon:\t" << e.what() << endmsg;
  }
}

void DataListenerSvc::apmSend(char* clusterName, char* clusterNode,
			      char* key, long val)
{
  MsgStream msg(msgSvc(),"DataListenerSvc");
  msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
  double temp;
  temp = *(double*)val;

  try {
    DataListenerSvc::apm->sendParameter( clusterName, clusterNode, key, temp);
    msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
    msg << MSG::INFO << "Sent parameters to MonALISA sever:"
        << m_MLClusterName << "->"
        << jobIDString.c_str() << "->"
        << key << ":" << val << endmsg;
  } catch (std::runtime_error &e){
    msg << MSG::WARNING << "WARNING sending to ApMon:\t" << e.what() << endmsg;
  }
}

void DataListenerSvc::apmSend(char* clusterName, char* clusterNode,
			      char* key, bool val)
{
  MsgStream msg(msgSvc(),"DataListenerSvc");
  msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
  try {
    DataListenerSvc::apm->sendParameter( clusterName, clusterNode, key, val);
    msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
    msg << MSG::INFO << "Sent parameters to MonALISA sever:"
        << m_MLClusterName << "->"
        << jobIDString.c_str() << "->"
        << key << ":" << val << endmsg;
  } catch (std::runtime_error &e){
    msg << MSG::WARNING << "WARNING sending to ApMon:\t" << e.what() << endmsg;
  }
}

void DataListenerSvc::apmSend(char* clusterName, char* clusterNode,
			      char* key, char* val)
{
  MsgStream msg(msgSvc(),"DataListenerSvc");
  msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
  try {
    DataListenerSvc::apm->sendParameter( clusterName, clusterNode, key, val);
    msg << MSG::DEBUG << "ApMon instantiated" << endmsg;
    msg << MSG::INFO << "Sent parameters to MonALISA sever:"
        << m_MLClusterName << "->"
        << jobIDString.c_str() << "->"
        << key << ":" << val << endmsg;
  } catch (std::runtime_error &e){
    msg << MSG::WARNING << "WARNING sending to ApMon:\t" << e.what() << endmsg;
  }
}
