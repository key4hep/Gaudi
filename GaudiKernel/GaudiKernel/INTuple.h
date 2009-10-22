// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/INTuple.h,v 1.7 2008/10/27 19:22:20 marcocle Exp $
#ifndef GAUDIKERNEL_INTUPLE_H
#define GAUDIKERNEL_INTUPLE_H

// STL include files
#include <string>
#include <typeinfo>

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"

// Forward declarations
class INTuple;
class INTupleItem;
class ISelectStatement;

/** @class INTupleItem INTuple.h GaudiKernel/INTuple.h

    NTuple interface class definition

    Definition of the interface to an Ntuple class beeing
    a sequence(=Item) of data entries.

    @author M.Frank
*/
class GAUDI_API INTupleItem {
protected:
  /// Set the properties of the INTupleItem
  virtual void                setType (long typ)            = 0;
public:
  /// destructor
  virtual ~INTupleItem() { }

  /// Destruct object
  virtual void                release()                     = 0;
  /// Compiler type ID
  virtual const std::type_info& typeID()      const         = 0;
  /// Proper type name of the object
  virtual std::string         typeName()      const         = 0;
  /// Access data buffer (CONST)
  virtual const void*         buffer()        const         = 0;
  /// Is the tuple have an index item?
  virtual bool                hasIndex()      const         = 0;
  /// Access the index _Item
  virtual const std::string&  index()         const         = 0;
  /// Access _Item name
  virtual const std::string&  name()          const         = 0;
  /// Type information of the item
  virtual long                type()          const         = 0;
  /// Access the buffer length
  virtual long                length()        const         = 0;
  /// Reset column
  virtual void                reset()                       = 0;
  /// Dimension
  virtual long                ndim()          const         = 0;
  /// Access individual dimensions
  virtual long                dim(long i)     const         = 0;
  /// Size of entire object
  virtual long                size()          const         = 0;
  /// Number of items filled
  virtual long                filled()        const         = 0;
  /// Pointer to index column (if present, 0 else)
  virtual INTupleItem*        indexItem()                   = 0;
  /// Pointer to index column (if present, 0 else) (CONST)
  virtual const INTupleItem*  indexItem()     const         = 0;
  /// NTuple the item belongs to
  virtual INTuple*            tuple()                       = 0;
};

/** @class INTuple INTuple.h GaudiKernel/INTuple.h

    NTuple interface class definition

    Definition of the interface to an Ntuple class beeing
    a sequence(=Item) of data entries.

    @author M.Frank
*/
class GAUDI_API INTuple {
protected:
  /// Internally used by abstract classes
  virtual INTupleItem*          i_find(const std::string& name)  const = 0;
public:
  // Definition of _Item container
  typedef std::vector< INTupleItem* > ItemContainer;
  /// Access item container
  virtual ItemContainer&        items()   =  0;
  /// Access item container   (CONST)
  virtual const ItemContainer&  items()    const   = 0;
  /// Attach data buffer
  virtual void                  setBuffer(char* buff) = 0;
  /// Access data buffer (CONST)
  virtual const char*           buffer()  const = 0;
  /// Access data buffer
  virtual char*                 buffer()  = 0;
  /// Object title
  virtual const std::string&    title()         const         = 0;
  /// Reset all entries to their default values
  virtual void                  reset() = 0;
  /// Find an item row of the Ntuple (CONST)
  virtual const INTupleItem*    find(const std::string& name) const = 0;
  /// Find an item row of the   Ntuple
  virtual       INTupleItem*    find(const std::string& name) = 0;
  /// Add an item row to the N tuple
  virtual StatusCode            add(INTupleItem* item) = 0;
  /// Remove an item row (identified by pointer) from the N tuple
  virtual StatusCode            remove(INTupleItem* item) = 0;
  /// Remove an item row (identified by name) from the N tuple
  virtual StatusCode            remove(const std::string& name) = 0;
  /// Attach selector
  virtual StatusCode            attachSelector(ISelectStatement* sel) = 0;
  /// Access selector
  virtual ISelectStatement*     selector() = 0;
  /// Write record of the NTuple (Shortcut of writeRecord)
  virtual StatusCode            write() = 0;
  /// Write record of the NTuple
  virtual StatusCode            writeRecord() = 0;
  /// Read record of the NTuple (Shortcut of readRecord)
  virtual StatusCode            read() = 0;
  /// Read record of the NTuple
  virtual StatusCode            readRecord() = 0;
  /// Save the NTuple
  virtual StatusCode            save() = 0;
  virtual ~INTuple() {}
};
#endif
