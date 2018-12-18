#ifndef GAUDIALG_TUPLEPUT_H
#define GAUDIALG_TUPLEPUT_H 1
// =============================================================================
// Include files
// =============================================================================
#include <memory>
// =============================================================================
// GaudiKernel
// =============================================================================
#include "GaudiKernel/System.h"
// =============================================================================
// GaudiAlg
// =============================================================================
#include "GaudiAlg/TupleObj.h"
// ============================================================================
// ROOT TClass
// ============================================================================
#include "TClass.h"
// =============================================================================
/** @file
 *  Implementation file for Tuple::TupleObj::put method
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-04-08
 */
// =============================================================================
namespace Tuples
{
  /** @class ItemStore TuplePut.h GaudiAlg/TuplePut.h
   *
   *  Simple class, which represents the local storage of N-tuple items
   *  of the given type. Essentially it is a restricted
   *  GaudiUtils::HashMap with the ownership of the newly created entries
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-04-08
   */
  template <class VALUE>
  class ItemStore final
  {
    friend class TupleObj;

  private:
    typedef GaudiUtils::HashMap<std::string, std::unique_ptr<NTuple::Item<VALUE>>> Store;

  public:
    /// constructor : create empty map
    ItemStore() = default;

  private:
    /// the only one method:
    inline NTuple::Item<VALUE>* getItem( const std::string& key, Tuples::TupleObj* tuple )
    {
      // find the item by name
      auto ifound = m_map.find( key );
      // existing item?
      if ( m_map.end() != ifound ) return ifound->second.get(); // RETURN
      // check the tuple for booking:
      if ( !tuple ) return nullptr;
      // check the existence of the name
      if ( !tuple->goodItem( key ) ) {
        tuple->Error( "ItemStore::getItem('" + key + "') item name is not unique" ).ignore();
        return nullptr; // RETURN
      }
      // get the underlying object
      NTuple::Tuple* tup = tuple->tuple();
      if ( !tup ) {
        tuple->Error( "ItemStore::getItem('" + key + "') invalid NTuple::Tuple*" ).ignore();
        return nullptr; // RETURN
      }
      // create new item:
      // add the newly created item into the store:
      auto stored = m_map.emplace( key, std::make_unique<NTuple::Item<VALUE>>() );
      if ( !stored.second ) {
        tuple->Warning( "ItemStore::getItem('" + key + "') item already exists, new one not inserted!" ).ignore();
        return nullptr;
      }
      auto& item = stored.first->second;
      // add it into N-tuple
      StatusCode sc = tup->addItem( key, *item ); // ATTENTION!
      if ( sc.isFailure() ) {
        tuple->Error( "ItemStore::getItem('" + key + "') cannot addItem", sc ).ignore();
        m_map.erase( stored.first );
        return nullptr; // RETURN
      }
      // check the name again
      if ( !tuple->addItem( key, System::typeinfoName( typeid( VALUE ) ) ) ) {
        tuple->Warning( "ItemStore::getItem('" + key + "') the item not unique " ).ignore();
        m_map.erase( stored.first );
        return nullptr;
      }
      //
      return item.get(); // RETURN
    }

    // delete copy constructor and assignment
    ItemStore( const ItemStore& ) = delete;
    ItemStore& operator=( const ItemStore& ) = delete;

  private:
    /// the underlying map
    Store m_map; ///< the underlying map
  };
} // end of namespace Tuples
// =============================================================================
/** The function allows to add almost arbitrary object into N-tuple
 *  @attention it requires POOL persistency
 *
 *  @param name column name
 *  @param obj  pointer to the object
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-04-08
 */
// =============================================================================
template <class TYPE>
inline StatusCode Tuples::TupleObj::put( const std::string& name, const TYPE* obj )
{
  if ( invalid() ) {
    return ErrorCodes::InvalidTuple;
  } // RETURN
  if ( !evtColType() ) {
    return ErrorCodes::InvalidOperation;
  } // RETURN

  // static block: The type description & the flag
  static bool    s_fail = false;   // STATIC
  static TClass* s_type = nullptr; // STATIC
  // check the status
  if ( s_fail ) {
    return ErrorCodes::InvalidItem;
  } // RETURN
  else if ( !s_type ) {
    s_type = TClass::GetClass( typeid( TYPE ) );
    if ( !s_type ) {
      s_fail = true;
      return Error( " put('" + name + "'," + System::typeinfoName( typeid( TYPE ) ) + ") :Invalid ROOT Type",
                    ErrorCodes::InvalidItem ); // RETURN
    }
  }
  // the local storage of items
  static Tuples::ItemStore<TYPE*> s_map;
  // get the variable by name:
  auto item = s_map.getItem( name, this );
  if ( !item ) {
    return Error( " put('" + name + "'): invalid item detected", ErrorCodes::InvalidItem );
  }
  // assign the item!
  ( *item ) = const_cast<TYPE*>( obj ); // THATS ALL!!
  //
  return StatusCode::SUCCESS; // RETURN
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_TUPLEPUT_H
