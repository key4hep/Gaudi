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
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/Service.h"

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
class EventSelector : public extends<Service, IEvtSelector>
{
public:
  typedef std::vector<EventSelectorDataStream*>     Streams;
  typedef std::vector<std::string>                  StreamSpecs;
  typedef std::vector<Gaudi::Property<std::string>> Properties;

  long int m_streamID;

protected:
  /// Reference to the indicent service
  SmartIF<IIncidentSvc> m_incidentSvc = nullptr;

  SmartIF<IToolSvc> m_toolSvc = nullptr;

  IDataStreamTool* m_streamtool = nullptr;

  /// Reconfigure occurred
  bool m_reconfigure = false;
  /// Input stream specifiers (last used)
  StreamSpecs m_streamSpecsLast;
  /// Input streams
  Streams m_streams;
  /// Input stream counter (0..oo, monotonely increasing)
  int m_streamCount = 0;

  // Properties
  Gaudi::Property<StreamSpecs> m_streamSpecs{this, "Input", {}, "input stream specifiers (for job options)"};
  Gaudi::Property<int>         m_firstEvent{this, "FirstEvent", 0, "first event to be processed"};
  Gaudi::Property<int>         m_evtMax{this, "EvtMax", INT_MAX, "maximum number of events to be processed"};
  Gaudi::Property<int>         m_evtPrintFrequency{this, "PrintFreq", 10, "printout frequency"};
  Gaudi::Property<std::string> m_streamManager{this, "StreamManager", "DataStreamTool", ""};

  /// Progress report
  virtual void printEvtInfo( const EvtSelectorContext* iter ) const;

public:
  /// IService implementation: Db event selector override
  StatusCode initialize() override;

  /// IService implementation: Service finalization
  StatusCode finalize() override;

  /// Service override: Reinitialize service
  StatusCode reinitialize() override;

  /// Create a new event loop context
  /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode createContext( Context*& refpCtxt ) const override;

  /// Get next iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode next( Context& refCtxt ) const override;

  /// Get next iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode next( Context& refCtxt, int jump ) const override;

  /// Get previous iteration item from the event loop context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode previous( Context& refCtxt ) const override;

  /// Get previous iteration item from the event loop context, but skip jump elements
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param jump      [IN]      Number of events to be skipped
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode previous( Context& refCtxt, int jump ) const override;

  /// Rewind the dataset
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode rewind( Context& refCtxt ) const override;

  /// Create new Opaque address corresponding to the current record
  /** @param refCtxt   [IN/OUT]  Reference to the context
    * @param refpAddr  [OUT]     Reference to address pointer
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode createAddress( const Context& refCtxt, IOpaqueAddress*& refpAddr ) const override;

  /// Release existing event iteration context
  /** @param refCtxt   [IN/OUT]  Reference to the context
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode releaseContext( Context*& refCtxt ) const override;

  /** Will set a new criteria for the selection of the next list of events and will change
    * the state of the context in a way to point to the new list.
    *
    * @param cr The new criteria string.
    * @param c Reference pointer to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode resetCriteria( const std::string& cr, Context& c ) const override;

  /** Access last item in the iteration
    * @param c Reference to the Context object.
    *
    * @return StatusCode indicating success or failure
    */
  StatusCode last( Context& c ) const override;

  /// Retrieve first entry of the next data stream
  StatusCode firstOfNextStream( bool shutDown, EvtSelectorContext& it ) const;
  /// Retrieve last entry of the previous data stream
  StatusCode lastOfPreviousStream( bool shutDown, EvtSelectorContext& it ) const;

  /// inherit constructor
  using extends::extends;

  /// Standard Destructor
  ~EventSelector() override = default;
};

#endif // GAUDISVC_EVENTSELECTOR_EVENTSELECTOR_H
