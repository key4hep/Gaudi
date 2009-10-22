#ifndef GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H

// Required for inheritance
#include "OutputStream.h"

/** @class SequentialOutputStream SequentialOutputStream.h
 * Extension of OutputStream to write run records after last event
 *
 * @author  M.Frank
 * @version 1.0
 */
class SequentialOutputStream : public OutputStream     {
   
 protected:
   
   /// OutputStream override: Select the different objects and write them to file 
   virtual StatusCode writeObjects();

 public:

   /// Standard algorithm Constructor
   SequentialOutputStream(const std::string& nam, ISvcLocator* svc);
   /// Standard Destructor
   virtual ~SequentialOutputStream() {}

   virtual StatusCode execute();

 private:

   // Properties
   unsigned int m_eventsPerFile;
   unsigned int m_nNumbersAdded;
   bool m_numericFilename;

   // Data members
   unsigned int m_events;
   unsigned int m_iFile;

   // Helper Methods
   void makeFilename();

};

#endif // GAUDISVC_PERSISTENCYSVC_SEQUENTIALOUTPUTSTREAM_H
