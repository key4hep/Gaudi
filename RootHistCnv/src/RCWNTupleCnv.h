#ifndef ROOTHISTCNV_RCWNTCNV_H
#define ROOTHISTCNV_RCWNTCNV_H 1

// Include files
#include "GaudiKernel/Converter.h"
#include "RNTupleCnv.h"
#include <string>

#include "TFile.h"
#include "TTree.h"

namespace RootHistCnv
{

  /** @class RootHistCnv::RCWNTupleCnv RCWNTupleCnv.h
   *
   * Converter of Column-wise NTuple into ROOT format
   * @author Charles Leggett
   */

  class RCWNTupleCnv : public RNTupleCnv
  {
  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_ColumnWiseTuple; }
    /// Standard constructor
    RCWNTupleCnv( ISvcLocator* svc ) : RNTupleCnv( svc, classID() ) {}
    /// Standard destructor
    ~RCWNTupleCnv() override {}

  protected:
    /// Create the transient representation of an object.
    StatusCode load( TTree* tree, INTuple*& refpObject ) override;
    /// Book the N tuple
    StatusCode book( const std::string& desc, INTuple* pObject, TTree*& tree ) override;
    /// Write N tuple data
    StatusCode writeData( TTree* rtree, INTuple* pObject ) override;
    /// Read N tuple data
    StatusCode readData( TTree* rtree, INTuple* pObject, long ievt ) override;

    template <class T>
    size_t saveItem( char* target, const T* src, size_t len )
    {
      long* tar = (long*)target;
      for ( size_t i = 0; i < len; i++ ) {
        *( tar++ ) = long( *( src++ ) );
      }
      return sizeof( long ) * len;
    }
    template <typename T>
    size_t saveItem( char* target, T* src, size_t len )
    {
      memcpy( target, src, sizeof( T ) * len );
      return sizeof( T ) * len;
    }

    template <class T>
    size_t loadItem( char* src, T* tar, size_t len )
    {
      long* s = (long*)src;
      for ( size_t i = 0; i < len; i++ ) {
        *( tar++ ) = T( *( s++ ) );
      }
      return sizeof( long ) * len;
    }
    size_t loadItem( char* src, bool* tar, size_t len )
    {
      long* s = (long*)src;
      for ( size_t i = 0; i < len; i++ ) {
        *( tar++ ) = ( *( s++ ) ) ? true : false;
      }
      return sizeof( long ) * len;
    }
    size_t loadItem( char* src, float* target, size_t len )
    {
      memcpy( target, src, sizeof( float ) * len );
      return sizeof( float ) * len;
    }
    size_t loadItem( char* src, double* target, size_t len )
    {
      memcpy( target, src, sizeof( double ) * len );
      return sizeof( double ) * len;
    }
    size_t loadItem( char* src, long* target, size_t len )
    {
      memcpy( target, src, sizeof( long ) * len );
      return sizeof( long ) * len;
    }
    size_t loadItem( char* src, unsigned long* target, size_t len )
    {
      memcpy( target, src, sizeof( unsigned long ) * len );
      return sizeof( unsigned long ) * len;
    }
  };
} // namespace RootHistCnv

#endif // ROOTHISTCNV_RCWNTCNV_H
