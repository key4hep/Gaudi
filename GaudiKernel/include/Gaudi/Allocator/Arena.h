/***********************************************************************************\
* (c) Copyright 2019-20 CERN for the benefit of the LHCb and ATLAS collaborations   *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <cstddef>
#include <functional>
#include <type_traits>
namespace Gaudi::Allocator {
  /** @class Arena
   *  @brief Custom allocator holding a pointer to a generic memory resource.
   *
   *  Custom allocator holding a pointer to a memory resource ("arena").
   *  Allocation and deallocation requests are passed through to the arena.
   *  Note that the typedefs propagate_on_container_{swap,copy_assignment,move_assignment}
   *  do not have their default values.
   *
   *  The, optional, third template parameter (DefaultResource) may be used to specify
   *  a default arena, in which case the allocator is default-constructible. Otherwise,
   *  a pointer to an arena must be given.
   */
  template <typename Resource, typename T, typename DefaultResource = void>
  struct Arena {
    using value_type                             = T;
    using propagate_on_container_swap            = std::true_type;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;

    /** Construct an allocator using the given memory resource, which must be valid.
     */
    constexpr Arena( Resource* resource ) noexcept : m_resource{ resource } {}

    /** Construct an allocator using the resource provided by DefaultResource.
     *  This constructor is only enabled if an instance of DefaultResource can be invoked
     *  with no arguments and yields Resource*.
     */
    template <typename D = void, typename = std::enable_if_t<std::is_invocable_r_v<Resource*, DefaultResource>, D>>
    Arena() : Arena( std::invoke( DefaultResource{} ) ) {}

    /** Converting copy constructor, rebinding U -> T.
     */
    template <typename U>
    constexpr Arena( Arena<Resource, U, DefaultResource> const& other ) noexcept : m_resource{ other.m_resource } {}

    /** Allocate storage for n objects.
     */
    [[nodiscard]] T* allocate( std::size_t n ) {
      return reinterpret_cast<T*>( m_resource->template allocate<alignof( T )>( n * sizeof( T ) ) );
    }

    /** Deallocate storage for n objects.
     */
    void deallocate( T* p, std::size_t n ) noexcept {
      m_resource->deallocate( reinterpret_cast<std::byte*>( p ), n * sizeof( T ) );
    }

    /** Return a pointer to the memory resource.
     */
    [[nodiscard]] Resource* resource() const noexcept { return m_resource; }

    template <typename U>
    friend constexpr bool operator==( Arena const& lhs, Arena<Resource, U, DefaultResource> const& rhs ) {
      return lhs.m_resource == rhs.m_resource;
    }

    template <typename U>
    struct rebind {
      using other = Arena<Resource, U, DefaultResource>;
    };

  private:
    // Required for the Arena<Resource, U, DefaultResource> converting copy constructor
    template <typename, typename, typename>
    friend struct Arena;

    Resource* m_resource{ nullptr };
  };

  template <typename Resource, typename T, typename U, typename DefaultResource>
  inline constexpr bool operator!=( Arena<Resource, T, DefaultResource> const& lhs,
                                    Arena<Resource, U, DefaultResource> const& rhs ) {
    return !( lhs == rhs );
  }
} // namespace Gaudi::Allocator