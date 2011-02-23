// $Id: Allocator.h,v 1.1 2006/02/14 15:01:12 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.1 $
// ============================================================================
/** @file
 *  Allocator
 *  The file is copied form Geant4 project
 *  @date 2006-02-10
 */
// ============================================================================

//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// ------------------------------------------------------------
// GEANT 4 class header file
//
// Class Description:
//
// A class for fast allocation of objects to the heap through a pool of
// chunks organised as linked list. It's meant to be used by associating
// it to the object to be allocated and defining for it new and delete
// operators via MallocSingle() and FreeSingle() methods.

//      ---------------- G4Allocator ----------------
//
// Author: G.Cosmo (CERN), November 2000
// ------------------------------------------------------------

// ============================================================================
#ifndef GAUDIKERNEL_Allocator_h
#define GAUDIKERNEL_Allocator_h 1
// ============================================================================
// Incldue files
// ============================================================================
// STD & STL
// ============================================================================
#include <cstddef>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AllocatorPool.h"
// ============================================================================


namespace GaudiUtils
{
  /** @class Allocator Allocator.h GaudiKernel/Allocator.h
   *  Allocator.
   *  The class is imported from Geant4 project
   *  @date 2006-02-10
   */
  template <class Type>
  class Allocator
  {
  public:  // with description

    /// Constructor
    Allocator() throw();
    /// destructor
    ~Allocator() throw();

    /** Malloc and Free methods to be used when overloading
     *  new and delete operators in the client &lt;Type&gt; object
     */
    inline Type* MallocSingle();
    inline void FreeSingle(Type* anElement);

    /** Returns allocated storage to the free store, resets
     *  allocator and page sizes.
     *  Note: contents in memory are lost using this call !
     */
    inline void ResetStorage();

    /// Returns the size of the total memory allocated
    inline size_t GetAllocatedSize() const;

  public:  // without description

    // This public section includes standard methods and types
    // required if the allocator is to be used as alternative
    // allocator for STL containers.
    // NOTE: the code below is a trivial implementation to make
    //       this class an STL compliant allocator.
    //       It is anyhow NOT recommended to use this class as
    //       alternative allocator for STL containers !

    typedef Type value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Type* pointer;
    typedef const Type* const_pointer;
    typedef Type& reference;
    typedef const Type& const_reference;

    /// Copy constructor
    template <class U> Allocator
    (const Allocator<U>& right) throw()
      : mem(right.mem) {}

    /// Returns the address of values
    pointer address(reference r) const { return &r; }
    const_pointer address(const_reference r) const { return &r; }

    /// Allocates space for n elements of type Type, but does not initialise
    pointer allocate(size_type n, void* hint = 0)
    {
      // Allocates space for n elements of type Type, but does not initialise
      //
      Type* mem_alloc = 0;
      if (n == 1)
        mem_alloc = MallocSingle();
      else
        mem_alloc = static_cast<Type*>(::operator new(n*sizeof(Type)));
      return mem_alloc;
    }

    /// Deallocates n elements of type Type, but doesn't destroy
    void deallocate(pointer p, size_type n)
    {
      // Deallocates n elements of type Type, but doesn't destroy
      //
      if (n == 1)
        FreeSingle(p);
      else
        ::operator delete((void*)p);
      return;
    }

    /// Initialises *p by val
    void construct(pointer p, const Type& val) { new((void*)p) Type(val); }
    /// Destroy *p but doesn't deallocate
    void destroy(pointer p) { p->~Type(); }

    /// Returns the maximum number of elements that can be allocated
    size_type max_size() const throw()
    {
      // Returns the maximum number of elements that can be allocated
      //
      return 2147483647/sizeof(Type);
    }

    // Rebind allocator to type U
    template <class U>
    struct rebind { typedef Allocator<U> other; };

  private:

    GaudiUtils::AllocatorPool mem;
    // Pool of elements of sizeof(Type)
  };

} // end of the namespace GaudiUtils

// ------------------------------------------------------------
// Inline implementation
// ------------------------------------------------------------

// Initialization of the static pool
//
// template <class Type> G4AllocatorPool G4Allocator<Type>::mem(sizeof(Type));

// ************************************************************
// G4Allocator constructor
// ************************************************************
//
template <class Type>
GaudiUtils::Allocator<Type>::Allocator() throw()
  : mem(sizeof(Type))
{
}

// ************************************************************
// G4Allocator destructor
// ************************************************************
//
template <class Type>
GaudiUtils::Allocator<Type>::~Allocator() throw()
{
}

// ************************************************************
// MallocSingle
// ************************************************************
//
template <class Type>
Type* GaudiUtils::Allocator<Type>::MallocSingle()
{
  return static_cast<Type*>(mem.Alloc());
}

// ************************************************************
// FreeSingle
// ************************************************************
//
template <class Type>
void GaudiUtils::Allocator<Type>::FreeSingle(Type* anElement)
{
  mem.Free(anElement);
  return;
}

// ************************************************************
// ResetStorage
// ************************************************************
//
template <class Type>
void GaudiUtils::Allocator<Type>::ResetStorage()
{
  // Clear all allocated storage and return it to the free store
  //
  mem.Reset();
  return;
}

// ************************************************************
// GetAllocatedSize
// ************************************************************
//
template <class Type>
size_t GaudiUtils::Allocator<Type>::GetAllocatedSize() const
{
  return mem.Size();
}

// ************************************************************
// operator==
// ************************************************************
//
template <class T1, class T2>
bool operator==
(const GaudiUtils::Allocator<T1>&,
 const GaudiUtils::Allocator<T2>&) throw()
{
  return true;
}

// ************************************************************
// operator!=
// ************************************************************
//
template <class T1, class T2>
bool operator!=
(const GaudiUtils::Allocator<T1>&,
 const GaudiUtils::Allocator<T2>&) throw()
{
  return false;
}

// ============================================================================
// The END
// ============================================================================
#endif
// ============================================================================

