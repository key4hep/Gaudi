// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/NTupleImplementation.h,v 1.7 2006/12/10 20:29:17 leggett Exp $
#ifndef GAUDIKERNEL_NTUPLEIMP_H
#define GAUDIKERNEL_NTUPLEIMP_H

// Framework include files
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Kernel.h"

// Forward declarations
class INTupleSvc;
class IConversionSvc;

namespace NTuple   {
  // Concrete N tuple class definition
  class GAUDI_API TupleImp : public Tuple  {
  protected:
    /// Container with N tuple _Columns
    ItemContainer     m_items;
    /// Flag wether N tuple is booked
    bool              m_isBooked;
    /// N tuple title
    std::string       m_title;
    /// Possibly hanging selector
    ISelectStatement* m_pSelector;
    /// Buffer size
    char*             m_buffer;
    /// Reference to N-tuple service used
    INTupleSvc*       m_ntupleSvc;
    /// Reference to the conversion service used
    IConversionSvc*   m_cnvSvc;
  private:
    /// Standard Copy Constructor
    TupleImp(const TupleImp&) ;

  public:
    /// Internally used by abstract classes
    virtual INTupleItem* i_find(const std::string& name)  const;
  public:
    /// Standard Constructor
    TupleImp( const std::string& title);
    /// Standard Destructor
    virtual ~TupleImp();
    /// Access item container
    ItemContainer& items()    {
      return m_items;
    }
    /// Access item container   (CONST)
    const ItemContainer& items()    const   {
      return m_items;
    }
    /// Object title
    virtual const std::string&  title()         const   {
      return m_title;
    }
    /// Find an item row of the Ntuple (CONST)
    virtual const INTupleItem* find(const std::string& name) const    {
      return i_find(name);
    }
    /// Find an item row of the Ntuple
    virtual INTupleItem* find(const std::string& name)    {
      return i_find(name);
    }
    /// Access N tuple data buffer
    char* buffer()    {
      return m_buffer;
    }
    /// Access N tuple data buffer    (CONST)
    const char* buffer()  const  {
      return m_buffer;
    }
    /// Access conversion service
    IConversionSvc* conversionService()   const  {
      return m_cnvSvc;
    }
    /// Access conversion service
    void setConversionService( IConversionSvc* svc )    {
      m_cnvSvc = svc;
    }
    /// Access conversion service
    INTupleSvc* tupleService()   const  {
      return m_ntupleSvc;
    }
    /// Access conversion service
    void setTupleService( INTupleSvc* svc )    {
      m_ntupleSvc = svc;
    }
    /// Attach selector
    virtual StatusCode attachSelector(ISelectStatement* sel);
    /// Access selector
    virtual ISelectStatement* selector();
    /// Set N tuple data buffer
    virtual void setBuffer(char* buff);
    /// Reset all entries to their default values
    virtual void reset();
    /// Add an item row to the N tuple
    virtual StatusCode add(INTupleItem* item);
    /// Remove an item row (identified by pointer) from the N tuple
    virtual StatusCode remove(INTupleItem* item);
    /// Remove an item row (identified by name) from the N tuple
    virtual StatusCode remove(const std::string& name);
      /// Write record of the NTuple (Shortcut of writeRecord)
    virtual StatusCode write();
    /// Write record of the NTuple
    virtual StatusCode writeRecord();
    /// Read record of the NTuple (Shortcut of readRecord)
    virtual StatusCode read();
    /// Read record of the NTuple
    virtual StatusCode readRecord();
    /// Save the NTuple
    virtual StatusCode save();

  };  // end class definition: Tuple


  // Concrete column wise N tuple class definition
  class ColumnWiseTuple : public TupleImp    {
  public:
    /// Standard Constructor
    ColumnWiseTuple(const std::string& title ) : TupleImp(title)    {
    }
    /// Standard Destructor
    virtual ~ColumnWiseTuple()   {
    }
    /// Retrieve Reference to class defininition structure
    virtual const CLID& clID() const   {
      return classID();
    }
    /// Static access to class defininition structure
    static const CLID& classID()      {
      return CLID_ColumnWiseTuple;
    }
  };  // end class definition: ColumnWiseTuple

  // Concrete column wise N tuple class definition
  class RowWiseTuple : public TupleImp    {
  public:
    /// Standard Constructor
    RowWiseTuple( const std::string& title ) : TupleImp(title)    {
    }
    /// Standard Destructor
    virtual ~RowWiseTuple()   {
    }
    /// Retrieve Reference to class defininition structure
    virtual const CLID& clID() const   {
      return classID();
    }
    /// Static access to class defininition structure
    static const CLID& classID()      {
      return CLID_RowWiseTuple;
    }
  };  // end class definition: RowWiseTuple
}   // end name space NTuple

#endif  // GAUDIKERNEL_NTUPLEIMP_H
