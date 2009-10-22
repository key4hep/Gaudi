// $Id: AllocatorPool.h,v 1.3 2007/05/24 14:39:11 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.3 $
// ============================================================================
/** @file
 *  Allocator pool.
 *  Class is imported from Geant4 project
 *  @date 2006-02-14
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
// -------------------------------------------------------------------
//      GEANT 4 class header file
//
// Class description:
//
// Class implementing a memory pool for fast allocation and deallocation
// of memory chunks.  The size of the chunks for small allocated objects
// is fixed to 1Kb and takes into account of memory alignment; for large
// objects it is set to 10 times the object's size.
// The implementation is derived from: B.Stroustrup, The C++ Programming
// Language, Third Edition.

//           -------------- G4AllocatorPool ----------------
//
// Author: G.Cosmo (CERN), November 2000
// -------------------------------------------------------------------

#ifndef GAUDIKERNEL_AllocatorPool_h
#define GAUDIKERNEL_AllocatorPool_h 1

#include "GaudiKernel/Kernel.h"

namespace GaudiUtils
{
  /** @class AllocatorPool AllocatorPool.h GaudiKernel/AllocatorPool.h
   *
   *  Allocator pool.
   *  Class is imported from Geant4 project
   *
   *  @date 2006-02-14
   */
  class GAUDI_API AllocatorPool
  {
  public:

    /// Create a pool of elements of size n
    explicit AllocatorPool( unsigned int n=0 );
    /// Destructor. Return storage to the free store
    ~AllocatorPool();
    /// Copy constructor
    AllocatorPool(const AllocatorPool& right);
    /// Allocate one element
    inline void* Alloc();
    /// Return an element back to the pool
    inline void  Free( void* b );
    /// Return storage size
    inline unsigned int  Size() const;
    /// Return storage to the free store
    void  Reset();

  private:

    /// Private equality operator
    AllocatorPool& operator= (const AllocatorPool& right);

  private:

    struct PoolLink
    {
      PoolLink* next;
    };
    class PoolChunk
    {
    public:
      explicit PoolChunk(unsigned int sz)
        : size(sz), mem(new char[size]), next(0) {;}
      ~PoolChunk() { delete [] mem; }
      const unsigned int size;
      char* mem;
      PoolChunk* next;
    };

    /// Make pool larger
    void Grow();

  private:

    const unsigned int esize;
    const unsigned int csize;
    PoolChunk* chunks;
    PoolLink* head;
    int nchunks;
  };

} // end of namespace GaudiUtils



// ************************************************************
// Alloc
// ************************************************************
//
inline void*
GaudiUtils::AllocatorPool::Alloc()
{
  if ( head==0 ) { Grow(); }
  PoolLink* p = head;  // return first element
  head = p->next;
  return p;
}

// ************************************************************
// Free
// ************************************************************
//
inline void
GaudiUtils::AllocatorPool::Free( void* b )
{
  PoolLink* p = static_cast<PoolLink*>(b);
  p->next = head;        // put b back as first element
  head = p;
}

// ************************************************************
// Size
// ************************************************************
//
inline unsigned int
GaudiUtils::AllocatorPool::Size() const
{
  return nchunks*csize;
}


// ============================================================================
// The END
// ============================================================================
#endif
// ============================================================================

