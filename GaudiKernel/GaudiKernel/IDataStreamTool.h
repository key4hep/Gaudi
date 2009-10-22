// $Id: IDataStreamTool.h,v 1.2 2007/01/12 17:32:31 hmd Exp $
#ifndef GAUDIKERNEL_IDATASTREAMTOOL_H
#define GAUDIKERNEL_IDATASTREAMTOOL_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IAlgTool.h"

class IEvtSelector;
class EventSelectorDataStream;

/** @class IDataStreamTool IDataStreamTool.h GaudiKernel/IDataStreamTool.h
 *
 *
 *  @author Andres Felipe Osorio Oliveros
 *  @author Marco Clemencic
 *  @date   2006-09-21
 */

class GAUDI_API IDataStreamTool: virtual public IAlgTool {
public:
  /// InterfaceID
  DeclareInterfaceID(IDataStreamTool,2,0);

  typedef std::vector<std::string>                         StreamSpecs;
  typedef long                                             size_type;

  virtual StatusCode initializeStream( EventSelectorDataStream* ) = 0;

  virtual StatusCode finalizeStream( EventSelectorDataStream* ) = 0;

  virtual StatusCode getNextStream( const EventSelectorDataStream* &, size_type & ) = 0;

  virtual StatusCode getPreviousStream( const EventSelectorDataStream* &, size_type & ) = 0;

  virtual StatusCode addStream(const std::string &) = 0;

  virtual StatusCode addStreams(const StreamSpecs &) = 0;

  virtual StatusCode eraseStream( const std::string& ) = 0;

  virtual EventSelectorDataStream* getStream( size_type ) = 0;

  virtual EventSelectorDataStream* getStream( const std::string& ) = 0;

  virtual EventSelectorDataStream* lastStream() = 0;

  virtual size_type size() = 0;

  virtual StatusCode clear() = 0;

protected:

  virtual StatusCode createSelector(const std::string& , const std::string& ,
                                    IEvtSelector*&) = 0;

  virtual StatusCode createStream(const std::string&, const std::string&,
                                  EventSelectorDataStream*& ) = 0;

};
#endif // GAUDIKERNEL_IDATASTREAMTOOL_H
