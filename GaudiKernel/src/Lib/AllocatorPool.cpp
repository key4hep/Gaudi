// ============================================================================
/** @file
 *  Implementation file for the class GaudiUtils::AllocatorPool.h
 *  This file is copied form Geant4 project
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
// ----------------------------------------------------------------------
// G4AllocatorPool
//
// Implementation file
//
// Author: G.Cosmo, November 2000
//
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AllocatorPool.h"
// ============================================================================

// ************************************************************
// G4AllocatorPool constructor
// ************************************************************
//
GaudiUtils::AllocatorPool::AllocatorPool( unsigned int sz )
    : esize( sz < sizeof( PoolLink ) ? sizeof( PoolLink ) : sz ), csize( sz < 1024 / 2 - 16 ? 1024 - 16 : sz * 10 - 16 )
{
}

// ************************************************************
// G4AllocatorPool copy constructor
// ************************************************************
//
GaudiUtils::AllocatorPool::AllocatorPool( const AllocatorPool& right ) : esize( right.esize ), csize( right.csize )
{
  *this = right;
}

// ************************************************************
// G4AllocatorPool operator=
// ************************************************************
//
GaudiUtils::AllocatorPool& GaudiUtils::AllocatorPool::operator=( const GaudiUtils::AllocatorPool& right )
{
  if ( &right == this ) {
    return *this;
  }
  chunks  = right.chunks;
  head    = right.head;
  nchunks = right.nchunks;
  return *this;
}

// ************************************************************
// G4AllocatorPool destructor
// ************************************************************
//
GaudiUtils::AllocatorPool::~AllocatorPool() { Reset(); }

// ************************************************************
// Reset
// ************************************************************
//
void GaudiUtils::AllocatorPool::Reset()
{
  // Free all chunks
  //
  PoolChunk* n = chunks;
  PoolChunk* p = nullptr;
  while ( n ) {
    p = n;
    n = n->next;
    delete p;
  }
  head    = nullptr;
  chunks  = nullptr;
  nchunks = 0;
}

// ************************************************************
// Grow
// ************************************************************
//
void GaudiUtils::AllocatorPool::Grow()
{
  // Allocate new chunk, organize it as a linked list of
  // elements of size 'esize'
  //
  auto n  = new PoolChunk( csize );
  n->next = chunks;
  chunks  = n;
  ++nchunks;

  const int nelem = csize / esize;
  char* start     = n->mem.get();
  char* last      = &start[( nelem - 1 ) * esize];
  for ( char* p = start; p < last; p += esize ) {
    reinterpret_cast<PoolLink*>( p )->next = reinterpret_cast<PoolLink*>( p + esize );
  }
  reinterpret_cast<PoolLink*>( last )->next = nullptr;
  head                                      = reinterpret_cast<PoolLink*>( start );
}

// ============================================================================
// The END
// ============================================================================
