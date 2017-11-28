#ifndef GAUDIKERNEL_NTUPLEIMP_H
#define GAUDIKERNEL_NTUPLEIMP_H

#include <memory>
// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartIF.h"

// Forward declarations
class INTupleSvc;
class IConversionSvc;

namespace NTuple
{
  // Concrete N tuple class definition
  class GAUDI_API TupleImp : public Tuple
  {
  protected:
    /// Container with N tuple _Columns
    ItemContainer m_items;
    /// Flag wether N tuple is booked
    bool m_isBooked = false;
    /// N tuple title
    std::string m_title;
    /// Possibly hanging selector
    SmartIF<ISelectStatement> m_pSelector;
    /// Buffer
    std::unique_ptr<char[]> m_buffer;
    /// Reference to N-tuple service used
    INTupleSvc* m_ntupleSvc = nullptr;
    /// Reference to the conversion service used
    IConversionSvc* m_cnvSvc = nullptr;

  private:
    /// Standard Copy Constructor
    TupleImp( const TupleImp& ) = delete;

  public:
    /// Internally used by abstract classes
    INTupleItem* i_find( const std::string& name ) const override;

  public:
    /// Standard Constructor
    TupleImp( std::string title );
    /// Standard Destructor
    ~TupleImp() override;
    /// Access item container
    ItemContainer& items() override { return m_items; }
    /// Access item container   (CONST)
    const ItemContainer& items() const override { return m_items; }
    /// Object title
    const std::string& title() const override { return m_title; }
    /// Find an item row of the Ntuple (CONST)
    const INTupleItem* find( const std::string& name ) const override { return i_find( name ); }
    /// Find an item row of the Ntuple
    INTupleItem* find( const std::string& name ) override { return i_find( name ); }
    /// Access N tuple data buffer
    char* buffer() override { return m_buffer.get(); }
    /// Access N tuple data buffer    (CONST)
    const char* buffer() const override { return m_buffer.get(); }
    /// Access conversion service
    IConversionSvc* conversionService() const { return m_cnvSvc; }
    /// Access conversion service
    void setConversionService( IConversionSvc* svc ) { m_cnvSvc = svc; }
    /// Access conversion service
    INTupleSvc* tupleService() const { return m_ntupleSvc; }
    /// Access conversion service
    void setTupleService( INTupleSvc* svc ) { m_ntupleSvc = svc; }
    /// Attach selector
    StatusCode attachSelector( ISelectStatement* sel ) override;
    /// Access selector
    ISelectStatement* selector() override;
    /// Set N tuple data buffer
    virtual char* setBuffer( std::unique_ptr<char[]>&& buff );
    char* setBuffer( char* buff ) override;
    /// Reset all entries to their default values
    void reset() override;
    /// Add an item row to the N tuple
    StatusCode add( INTupleItem* item ) override;
    /// Remove an item row (identified by pointer) from the N tuple
    StatusCode remove( INTupleItem* item ) override;
    /// Remove an item row (identified by name) from the N tuple
    StatusCode remove( const std::string& name ) override;
    /// Write record of the NTuple (Shortcut of writeRecord)
    StatusCode write() override;
    /// Write record of the NTuple
    StatusCode writeRecord() override;
    /// Read record of the NTuple (Shortcut of readRecord)
    StatusCode read() override;
    /// Read record of the NTuple
    StatusCode readRecord() override;
    /// Save the NTuple
    StatusCode save() override;

  }; // end class definition: Tuple

  // Concrete column wise N tuple class definition
  class ColumnWiseTuple : public TupleImp
  {
  public:
    /// Standard Constructor
    ColumnWiseTuple( std::string title ) : TupleImp( std::move( title ) ) {}
    /// Retrieve Reference to class defininition structure
    const CLID& clID() const override { return classID(); }
    /// Static access to class defininition structure
    static const CLID& classID() { return CLID_ColumnWiseTuple; }
  }; // end class definition: ColumnWiseTuple

  // Concrete column wise N tuple class definition
  class RowWiseTuple : public TupleImp
  {
  public:
    /// Standard Constructor
    RowWiseTuple( std::string title ) : TupleImp( std::move( title ) ) {}
    /// Retrieve Reference to class defininition structure
    const CLID& clID() const override { return classID(); }
    /// Static access to class defininition structure
    static const CLID& classID() { return CLID_RowWiseTuple; }
  }; // end class definition: RowWiseTuple
} // end name space NTuple

#endif // GAUDIKERNEL_NTUPLEIMP_H
