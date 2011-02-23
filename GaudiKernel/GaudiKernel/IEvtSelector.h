//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IEvtSelector.h,v 1.5 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IEVTSELECTOR_H
#define GAUDIKERNEL_IEVTSELECTOR_H 1

// Include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/StatusCode.h"
#include <string>

/** @class IEvtSelector IEvtSelector.h GaudiKernel/IEvtSelector.h

    The Event Selector Interface. The EventSelector component is able
    to produce a list of event  given a set of stream specifications.

    @author C. Cioffi
    @date   14/11/2003
*/
class GAUDI_API IEvtSelector: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IEvtSelector,2,0);

  class Context{
  public:
    virtual ~Context() { }
    virtual void* identifier() const=0;
  };

  /**Create and return a context object that will
     keep track of the state of selection.

     @param c Reference of a pointer to a Context object.
  */
  virtual StatusCode createContext(Context*& c) const = 0;

  /**Fetch the next event or the first event if it will be use soon
     after the creation of the context.
     It will return StatusCode::FAILURE if there have been problem in the fetching or it
     has been reached the end of the list of events.

     @param c Reference to the Context object.
  */
  virtual StatusCode next(Context& c) const  = 0;

  /**Same of next(const Context&) plus the possibility to jump the next n-1 events.

     @param c Reference to the Context object.
     @param jump The event to jump to from the current event.
  */
  virtual StatusCode next(Context& c,int jump) const  = 0;

  /**Fetch the previous event.
     It will return StatusCode::FAILURE if there have been problem in the fetching or it
     has been reached the begin of the list of events.

     @param c Reference to the Context object.
   */
  virtual StatusCode previous(Context& c) const = 0;

  /**Same of previous(Context& c) the possibility to jump the previous n-1 events.

     @param c Reference to the Context object.
     @param jump The event to jump to from the current event.
   */
  virtual StatusCode previous(Context& c,int jump) const = 0;

  /** Access last item in the iteration
    * @param refContext [IN/OUT] Reference to the Context object.
    */
  virtual StatusCode last(Context& refContext) const = 0;

  /** Will set the state of the context in a way that the next event read
    * is the first of the list.
    *
    * @param c Reference to the Context object.
    */
  virtual StatusCode rewind(Context& c) const = 0;

  /** Create an IOpaqueAddress object from the event fetched.
    *
    * @param c Reference to the Context object.
    * @param iop Refernce pointer to a IOpaqueAddress object
    *
    */
  virtual StatusCode createAddress(const Context& c,IOpaqueAddress*& iop) const = 0;

  /** Release the Context object.
    *
    * @param c Reference pointer to the Context object.
    */
  virtual StatusCode releaseContext(Context*&)const=0;

  /** Will set a new criteria for the selection of the next list of events and will change
    * the state of the context in a way to point to the new list.
    *
    * @param cr The new criteria string.
    * @param c Reference pointer to the Context object.
    */
  virtual StatusCode resetCriteria(const std::string& cr,Context& c)const=0;
};

#endif //GAUDIKERNEL_IEVTSELECTOR_H
