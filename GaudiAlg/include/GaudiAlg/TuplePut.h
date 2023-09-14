/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIALG_TUPLEPUT_H
#define GAUDIALG_TUPLEPUT_H 1
#include "GaudiAlg/TupleObj.h"
#include "GaudiKernel/System.h"
#include "TClass.h"
#include <memory>
// =============================================================================
/** @file
 *  Implementation file for Tuple::TupleObj::put method
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-04-08
 */
// =============================================================================
namespace Tuples {
  /** @class ItemStore TuplePut.h GaudiAlg/TuplePut.h
   *
   *  Simple class, which represents the local storage of N-tuple items
   *  of the given type. Essentially it is a restricted
   *  std::unordered_map with ownership of the newly created entries
   *
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date   2007-04-08
   */
  template <class VALUE>
  class ItemStore final {
    friend class TupleObj;

  public:
    /// constructor : create empty map
    ItemStore() = default;

  private:
    /// the only one method:
    NTuple::Item<VALUE>* getItem( std::string_view key, Tuples::TupleObj* tuple ) {
      // find the item by name
      auto ifound = m_map.find( key );
      // existing item?
      if ( m_map.end() != ifound ) return &ifound->second.first; // RETURN

      // check the tuple for booking:
      if ( !tuple ) return nullptr;
      // check the existence of the name
      if ( !tuple->goodItem( key ) ) {
        tuple->Error( fmt::format( "ItemStore::getItem('{}') item name is not unique", key ) ).ignore();
        return nullptr; // RETURN
      }
      // get the underlying object
      NTuple::Tuple* tup = tuple->tuple();
      if ( !tup ) {
        tuple->Error( fmt::format( "ItemStore::getItem('{}') invalid NTuple::Tuple*", key ) ).ignore();
        return nullptr; // RETURN
      }
      // create new item:
      // add the newly created item into the store -- and point the key view into the mapped value...
      auto [iter, ok] = m_map.try_emplace( key, NTuple::Item<VALUE>{}, std::string{ key } );
      if ( ok ) {
        auto nh  = m_map.extract( iter );
        nh.key() = nh.mapped().second; // "re-point" key to the string contained value_type
        // std::tie( iter,ok,std::ignore)  = m_map.insert( std::move( nh ) );
        auto r = m_map.insert( std::move( nh ) );
        iter   = r.position;
        ok     = r.inserted;
      }
      if ( !ok ) {
        tuple->Warning( fmt::format( "ItemStore::getItem('{}') item already exists, new one not inserted!", key ) )
            .ignore();
        return nullptr;
      }
      auto& item = iter->second.first;
      // add it into N-tuple
      StatusCode sc = tup->addItem( iter->second.second, item ); // ATTENTION!
      if ( sc.isFailure() ) {
        tuple->Error( fmt::format( "ItemStore::getItem('{}') cannot addItem", key ), sc ).ignore();
        m_map.erase( iter );
        return nullptr; // RETURN
      }
      // check the name again
      if ( !tuple->addItem( iter->second.second, System::typeinfoName( typeid( VALUE ) ) ) ) {
        tuple->Warning( fmt::format( "ItemStore::getItem('{}') the item not unique ", key ) ).ignore();
        m_map.erase( iter );
        return nullptr;
      }
      //
      return &item; // RETURN
    }

    // delete copy constructor and assignment
    ItemStore( const ItemStore& )            = delete;
    ItemStore& operator=( const ItemStore& ) = delete;

  private:
    std::unordered_map<std::string_view, std::pair<NTuple::Item<VALUE>, std::string>> m_map; ///< the underlying map
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
StatusCode Tuples::TupleObj::put( std::string_view name, const TYPE* obj ) {
  if ( invalid() ) { return ErrorCodes::InvalidTuple; }         // RETURN
  if ( !evtColType() ) { return ErrorCodes::InvalidOperation; } // RETURN

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
      return Error( fmt::format( " put('{}',{}) :Invalid ROOT Type", name, System::typeinfoName( typeid( TYPE ) ) ),
                    ErrorCodes::InvalidItem ); // RETURN
    }
  }
  // the local storage of items
  static Tuples::ItemStore<TYPE*> s_map;
  // get the variable by name:
  auto item = s_map.getItem( name, this );
  if ( !item ) { return Error( fmt::format( " put('{}'): invalid item detected", name ), ErrorCodes::InvalidItem ); }
  // assign the item!
  *item = const_cast<TYPE*>( obj ); // THATS ALL!!
  //
  return StatusCode::SUCCESS; // RETURN
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_TUPLEPUT_H
