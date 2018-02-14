#ifndef GaudiKernel_EXPORTS // defined when compiling GaudiKernel
#warning deprecated header (will be removed in Gaudi v30r3)
#endif
#ifndef GAUDI_KERNEL_THREADLIBGAUDI_H
#define GAUDI_KERNEL_THREADLIBGAUDI_H

#include "GaudiKernel/System.h" /* ThreadHandle */
#include <map>
#include <memory>
#include <string>

/** @class ThreadGaudi ThreadGaudi.h GaudiKernel/ThreadGaudi.h
 *  @brief singleton mapping the pthread ID to the Gaudi thread ID
 *  @author Werner Wiedenmann
 */
class GAUDI_API ThreadGaudi
{
public:
  typedef std::map<System::ThreadHandle, std::string> ThreadMap;
  /// singleton access
  static ThreadGaudi* instance();
  /// associate Gaudi ID to pthread
  void setThreadID( const std::string& threadID );
  /// access the whole map
  ThreadMap* getThreadMap();
  /// get Gaudi ID of current pthread
  const std::string& getThreadID();

protected:
  ThreadGaudi();
  virtual ~ThreadGaudi() = default;
  std::unique_ptr<ThreadMap> m_threadMap;
};

/// helper function to extract Gaudi Thread ID from thread copy number
GAUDI_API std::string getGaudiThreadIDfromID( int iCopy );
/// helper function to extract Gaudi Thread ID from thread copy name
GAUDI_API std::string getGaudiThreadIDfromName( const std::string& name );
/// helper function to extract Gaudi instance name from thread copy name
GAUDI_API std::string getGaudiThreadGenericName( const std::string& name );
/// test if current Gaudi object is running /will run in a thread
GAUDI_API bool isGaudiThreaded( const std::string& name );

#endif // GAUDI_KERNEL_THREADLIBGAUDI_H
