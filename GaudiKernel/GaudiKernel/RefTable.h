// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/RefTable.h,v 1.6 2006/02/06 13:22:20 hmd Exp $
#ifndef GAUDIKERNEL_REFTABLE_H
#define GAUDIKERNEL_REFTABLE_H 1


// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/SmartRefVector.h"
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/HashMap.h" // Cannot use maps through sharable images....

// Externals 
static const CLID CLID_RefTable1to1 = 300;
static const CLID CLID_RefTable1toN = 301;


//------------------------------------------------------------------------------
//
// Implementation and definition of template classes
// RefTableBase, RefTable1to1, RefTable1toN
//
// Author                  :  Markus Frank
//
//------------------------------------------------------------------------------
/** template <class FROM, class TO, class MAPENTRY> class RefTable
  
   ClassName:   RefTableBase
  
   Description: Essential information of the RefTable
                References can be of several kinds:
                multiplicity:  FROM       TO
                                1        0   = 1 (no existing Reference)
                                         1   1 <-> 1 relationship
                                         n   1 <-> n relationship
                Template parameters:
                FROM           type of the source object
                TO             type of the referenced object(s)
                MAPENTRY       entry type used for the referenced objects
                               within the map.

   Author:      Markus Frank

*/
template <class FROM, class MAPENTRY> class RefTableBase : public DataObject    {
public:
  // Type of the key
  typedef FROM                                      KeyType;
  // Type of map entries
  typedef MAPENTRY                                  EntryType;
  // My own type
  typedef RefTableBase<FROM, EntryType>             BaseType;
  /** Define Reference map. Need to use index into vector, 
      because location of vector entries is not fixed 
  */
  typedef GaudiUtils::HashMap< const void* , EntryType >      TableType;
  /// Definition of map iterator
  typedef TableType::iterator       iterator;
  /// Definition of map iterator (CONST)
  typedef TableType::const_iterator const_iterator;
  /// Class id of the reference table
  CLID       m_clid;
private:
  /// Reference map
  TableType  m_table;
  /** This is a completely useless entry, but the compiler
      wants it to be instantiated before the serialize
      function is defined.
  */
  SmartRef<KeyType> m_fromRef;

protected:
  /// Insert new Entry into Reference container
  bool insertMapElement ( const KeyType* from, EntryType& to )    {
    return m_table.insert( from, to );
  }
  /// Find Reference from it's source entry
  EntryType* i_reference(const KeyType* from)  {
    iterator i = m_table.find( from );
    if ( i != 0 )    {
      return &((*i).second);
    }
    return 0; 
  }
  /// Find Reference from it's source entry (CONST)
  const EntryType* i_reference(const KeyType* from)  const  {
    const_iterator i = m_table.find( from );
    if ( i != m_table.end() )    {
      return &((*i).second);
    }
    return 0; 
  }

public:
	/// Constructors
  RefTableBase(const CLID& clid, int len) : m_clid(clid), m_table(len)   {
  }
  /// Destructor
  virtual ~RefTableBase()  { 
    clear();
  }
  /// Clear Reference map
  virtual void clear()    {
    m_table.clear();
  }
  /// Start of direct access iterator
  iterator begin()   {
    return m_table.begin();
  }
  /// Start of direct access iterator (CONST)
  const_iterator begin()  const  {
    return m_table.begin();
  }
  /// End of direct access iterator
  iterator end()   {
    return m_table.end();
  }
  /// End of direct access iterator (CONST)
  const_iterator end()  const  {
    return m_table.end();
  }
  /// Size of References
  long size()   const   {
    return m_table.size();
  }
  /// Size of References
  void reserve(int len)   {
    m_table.reserve(len);
  }
  /// Serialize the object for writing
  virtual StreamBuffer& serialize( StreamBuffer& s ) const   {
    DataObject::serialize(s) << m_table.size();
    //for (TableType::const_iterator i = m_table.begin(), stop = m_table.end(); i != stop; i++ )    {
    //  SmartRef<KeyType> fromRef;
    //  fromRef = (KeyType*)(*i).first;
    //  s << fromRef(this);
    //  s << (*i).second(this);
    //}
    return s;
  }
  /// Serialize the object for reading
  virtual StreamBuffer& serialize( StreamBuffer& s )         {
    long siz;
    DataObject::serialize(s) >> siz;
    m_table.reserve(siz);
    //for ( long i = 0; i < siz; i++ )   {
    //  SmartRef<KeyType> fromRef;
    //  EntryType entry;
    //  s >> fromRef(this);
    //  s >> entry(this);
    //  insertMapElement( fromRef, entry);
    //}
    return s;
  }
};

template <class FROM, class TO> class RefTable1to1
: public RefTableBase< FROM , SmartRef<TO> >    {
public:
  /// Standard Constructor
  RefTable1to1 (const CLID& clid, int len=16) 
  : RefTableBase< FROM , SmartRef<TO> >(clid, len)
  {
  }
  /// Standard Destructor
  virtual ~RefTable1to1()   {
  }
	/// Retrieve reference to class definition structure
	virtual const CLID& clID() const  {
    return m_clid;
  }
  /// Insert new Entry into Reference container
  bool insert ( const FROM* from, TO* to )    {
    return insertMapElement(from, EntryType(to));
  }
  /// Insert new Entry into Reference container
  bool insert ( const FROM* from, const EntryType& to)    {
    // We MUST check the environment of the smart pointer!
    if ( 0 != to.data() || StreamBuffer::INVALID != to.hintID() )    {
      return insertMapElement(from, EntryType(to));
    }
    return false;
  }
  /// Find Reference from it's source entry
  TO* reference(const FROM* from)  {
    EntryType* e = i_reference(from);
    return (0 == e) ? 0 : (*e);
  }

  /// Find Reference from it's source entry (CONST)
  const TO* reference(const FROM* from)  const {
    const EntryType* e = i_reference(from);
    return (0 == e) ? 0 : (*e);
  }
  
  /// Check if two entries are associated to each other
  bool isReferenced(const FROM* from, const TO* to )   {
    const EntryType* e = i_reference(from);
    return (e == 0) ? false : ((*e) == to);
  }
  /// Check if two entries are Referenced to each other
  bool isReferenced(const FROM* from, const EntryType& to )   {
    const EntryType* e = i_reference(from);
    return (assoc!=0) ? ((*e)=!to) ? (e->target()==to.target()) : false : false;
  }
};

template <class FROM, class TO> class RefTable1toN
: public RefTableBase< FROM , SmartRefVector<TO> >    {
public:
  /// Standard Constructor
  RefTable1toN (const CLID& clid, int len=16) 
  : RefTableBase< FROM , SmartRefVector<TO> >(clid, len)    {
  }
  /// Standard Destructor
  virtual ~RefTable1toN()   {
  }
	/// Retrieve reference to class definition structure
	virtual const CLID& clID() const  { 
    return m_clid; 
  }
  /// Insert new Entry into Reference container
  bool insert ( const FROM* from, TO* to)    {
    EntryType* entry = i_reference(from);
    if ( 0 == entry )   {
      bool result = insertMapElement(from, EntryType());
      EntryType* newEntry = i_reference(from);
      if ( !( 0 == newEntry) )    {
        newEntry->push_back( SmartRef<TO>(to) );
        return true;
      }
      return false;
    }
    entry->push_back( SmartRef<TO>(to) );
    return true;
  }
  /// Insert new Entry into Reference container
  bool insert ( const FROM* from, const SmartRef<TO>& to)    {
    EntryType* entry = i_reference(from);
    if ( 0 == entry )   {
      bool result = insertMapElement(from, EntryType());
      EntryType* newEntry = i_reference(from);
      if ( !(0 == newEntry) )    {
        newEntry->push_back( to );
        return true;
      }
      return false;
    }
    entry->push_back( to );
    return true;
  }
  /// Insert new Entry into Reference container
  bool insert ( const FROM* from, const EntryType& to)    {
    return insertMapElement(from, const_cast<EntryType&>(to));
  }
  /// Find Reference from it's source entry
  EntryType& reference(const FROM* from)  {
    static EntryType empty;
    EntryType* e = i_reference(from);
    return (0 == e) ? empty : *e;
  }
  /// Find Reference from it's source entry (CONST)
  const EntryType& reference(const FROM* from)  const {
    static EntryType empty;
    EntryType* e = i_reference(from);
    return (0 == e) ? empty : (*e);
  }
  /// Check if two entries are Referenced to each other
  bool isReferenced(const FROM* from, const EntryType& to )   {
    const EntryType* e = i_reference(from);
    return (0 == e) ? false : (*e == to);
  }
  /// Check if two entries are Referenced to each other
  bool isReferenced(const FROM* from, const TO* to )   {
    return isReferenced(from, SmartRef<TO>(to));
  }
  /// Check if two entries are Referenced to each other
  bool isReferenced(const FROM* from, const SmartRef<TO>& to )   {
    const EntryType* e = i_reference(from);
    if ( 0 != assoc )   {
      SmartRefVector<TO>::const_iterator i = std::find(e->begin(), e->end(), to);
      return (i == e->end()) ? false : true;
    }
    return false;
  }
};


#endif // GAUDIKERNEL_REFTABLE_H

