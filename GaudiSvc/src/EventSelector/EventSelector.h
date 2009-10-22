//====================================================================
//	EventSelector.h
//--------------------------------------------------------------------
//
//	Package    : EventSelector  (LHCb Event Selector Package)
//
//	Author     : M.Frank
//  Created    : 4/10/00
//
//====================================================================
//  Modified   : 11/11/06
//
//  Author     : Andres Felipe OSORIO OLIVEROS
//             : Marco CLEMENCIC
//
//====================================================================

#ifndef GAUDISVC_EVENTSELECTOR_EVENTSELECTOR_H
#define GAUDISVC_EVENTSELECTOR_EVENTSELECTOR_H 1

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IEvtSelector.h"

// STL include files
#include <vector>

// Forward declarations
class IIncidentSvc;
class IAddressCreator;
class IToolSvc;
class EventSelectorDataStream;
class EvtSelectorContext;
class IDataStreamTool;

/** Definition of class EventSelector

    Basic event selector service. The event selector service
    itself is able to connect other services to attached streams.

    History:
    +---------+----------------------------------------------+------------+
    |    Date |                 Comment                      |    Who     |
    +---------+----------------------------------------------+------------+
    | 3/10/00 | Initial version                              | M.Frank    |
    +---------+----------------------------------------------+------------+
    | 4/09/09 | Added firing incident on opening/ending file | R. Lambert |
    +---------+----------------------------------------------+------------+
   @author Markus Frank
   @author R. Lambert
   @version 1.0
*/
class EventSelector : public extends1<Service, IEvtSelector> {
public:
  typedef std::vector<EventSelectorDataStream*>  Streams;
  typedef std::vector<std::string>               StreamSpecs;
  typedef std::vector<StringProperty>            Properties;

  long int m_streamID;

protected:
  /// Reference to the indicent service
  SmartIF<IIncidentSvc> m_incidentSvc;

  SmartIF<IToolSvc>     m_toolSvc;

  IDataStreamTool*      m_streamtool;

  /// Reconfigure occurred
  bool                  m_reconfigure;
  /// Input stream specifiers (for job options)
  StreamSpecs           m_streamSpecs;
  /// Input stream specifiers (last used)
  StreamSpecs           m_streamSpecsLast;
  /// Input streams
  Streams               m_streams;
  /// Input stream counter (0..oo, monotonely increasing)
  int                   m_streamCount;
  /// First event to be processed
  int                   m_firstEvent;
  /// Maximum number of events to be processed
  int                   m_evtMax;
  /// Printout frequency
  int                   m_evtPrintFrequency;

  std::string           m_streamManager;



  /// Progress report
  virtual void printEvtInfo(const EvtSelectorContext* iter) const;

public:
  /// IService implementation: Db event selector override
  virtual StatusCode initialize();

  /// IService implementation: Service finalization
  virtual StatusCode finalize();

  /// Service override: Reinitialize service
  virtual StatusCode reinitialize();

  /// Create a new event loop context
  /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createContext(Context*& refpCtxt) const;

  /// Get next iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode next(Context& refCtxt) const;

  /// Get next iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode next(Context& refCtxt,int jump) const;

  /// Get previous iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode previous(Context& refCtxt) const;

  /// Get previous iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode previous(Context& refCtxt,int jump) const;

  /// Rewind the dataset
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode rewind(Context& refCtxt) const;

  /// Create new Opaque address corresponding to the current record
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param refpAddr  [OUT]     Reference to address pointer
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode createAddress(const Context& refCtxt, IOpaqueAddress*& refpAddr) const;

  /// Release existing event iteration context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode releaseContext(Context*& refCtxt) const;

  /** Will set a new criteria for the selection of the next list of events and will change
    * the state of the context in a way to point to the new list.
    *
    * @param cr The new criteria string.
    * @param c Reference pointer to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode resetCriteria(const std::string& cr,Context& c)const;

  /** Access last item in the iteration
    * @param c Reference to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  virtual StatusCode last(Context& c) const;

  /// Retrieve first entry of the next data stream
  StatusCode firstOfNextStream( bool shutDown, EvtSelectorContext& it) const;
  /// Retrieve last entry of the previous data stream
  StatusCode lastOfPreviousStream ( bool shutDown, EvtSelectorContext& it) const;

  /// Standard Constructor
  EventSelector( const std::string& name, ISvcLocator* svcloc );

  /// Standard Destructor
  virtual ~EventSelector();
};

#endif  // GAUDISVC_EVENTSELECTOR_EVENTSELECTOR_H
