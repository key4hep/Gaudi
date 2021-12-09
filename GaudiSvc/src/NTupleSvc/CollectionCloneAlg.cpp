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
//	====================================================================
//  CollectionCloneAlg.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/ISelectStatement.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/SmartIF.h"
#include <vector>

namespace {

  template <class T>
  static long upper( const INTupleItem* item ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    const NTuple::_Data<T>* it = dynamic_cast<const NTuple::_Data<T>*>( item );
    assert( it != nullptr );
    return it->range().upper();
#pragma GCC diagnostic pop
  }

  template <class TYP>
  static StatusCode createItem( MsgStream& log, INTuple* tuple, INTupleItem* src, const TYP& null ) {
    NTuple::_Data<TYP>* source = dynamic_cast<NTuple::_Data<TYP>*>( src );
    if ( !source ) return StatusCode::FAILURE;
    TYP                low    = source->range().lower();
    TYP                high   = source->range().upper();
    long               hasIdx = source->hasIndex();
    long               ndim   = source->ndim();
    const std::string& name   = source->name();
    std::string        idxName;
    long               dim[4], idxLen = 0;
    long               dim1 = 1, dim2 = 1;
    INTupleItem*       it = nullptr;
    for ( int i = 0; i < ndim; i++ ) dim[i] = source->dim( i );
    /// Type information of the item
    if ( hasIdx ) {
      const INTupleItem* index = source->indexItem();
      idxName                  = index->name();
      switch ( index->type() ) {
      case DataTypeInfo::UCHAR:
        idxLen = upper<unsigned char>( index );
        break;
      case DataTypeInfo::USHORT:
        idxLen = upper<unsigned short>( index );
        break;
      case DataTypeInfo::UINT:
        idxLen = upper<unsigned int>( index );
        break;
      case DataTypeInfo::ULONG:
        idxLen = upper<unsigned long>( index );
        break;
      case DataTypeInfo::CHAR:
        idxLen = upper<char>( index );
        break;
      case DataTypeInfo::SHORT:
        idxLen = upper<short>( index );
        break;
      case DataTypeInfo::INT:
        idxLen = upper<int>( index );
        break;
      case DataTypeInfo::LONG:
        idxLen = upper<long>( index );
        break;
      default:
        log << MSG::ERROR << "Column " << idxName << " is not a valid index column!" << endmsg;
        return StatusCode::FAILURE;
      }
    }
    switch ( ndim ) {
    case 0:
      it = NTuple::_Item<TYP>::create( tuple, name, typeid( TYP ), low, high, null );
      break;
    case 1:
      dim1 = ( hasIdx ) ? idxLen : dim[0];
      it   = NTuple::_Array<TYP>::create( tuple, name, typeid( TYP ), idxName, dim1, low, high, null );
      break;
    case 2:
      dim1 = ( hasIdx ) ? idxLen : dim[0];
      dim2 = ( hasIdx ) ? dim[0] : dim[1];
      it   = NTuple::_Matrix<TYP>::create( tuple, name, typeid( TYP ), idxName, dim1, dim2, low, high, null );
      break;
    default:
      return StatusCode::FAILURE;
    }
    return tuple->add( it );
  }
} // namespace

/**@class CollectionCloneAlg
 *
 * Small algorithm, which allows to merge N-tuples in
 * a generic way. In the options directory an python
 * interface is presented, which shows how to steer this
 * algorithm in a standaqlone program.
 *
 * @author:  M.Frank
 * @version: 1.0
 */
class CollectionCloneAlg : public Algorithm {

  Gaudi::Property<std::string> m_tupleSvc{ this, "EvtTupleSvc", "EvtTupleSvc", "name of the data provider service" };
  Gaudi::Property<std::vector<std::string>> m_inputs{ this, "Input", {}, "input specifications" };
  Gaudi::Property<std::string>              m_output{ this, "Output", {}, "output specification" };

  /// Reference to data provider service
  SmartIF<INTupleSvc> m_dataSvc;
  /// Name of the root leaf (obtained at initialize)
  std::string m_rootName;
  /// Output tuple name
  std::string m_outName;
  /// Selection criteria (if any)
  std::string m_criteria;
  /// Selector factory
  std::string m_selectorName;

public:
  /// Standard algorithm constructor
  using Algorithm::Algorithm;

  /// Initialize
  StatusCode initialize() override {
    MsgStream log( msgSvc(), name() );
    m_rootName     = "";
    m_outName      = "";
    m_criteria     = "";
    m_selectorName = "";
    m_dataSvc      = service( m_tupleSvc, true );
    if ( !m_dataSvc ) {
      log << MSG::ERROR << "Failed to access service \"" << m_tupleSvc << "\"." << endmsg;
      return StatusCode::FAILURE;
    }
    std::string fun;
    using Parser = Gaudi::Utils::AttribStringParser;
    for ( auto attrib : Parser( m_output ) ) {
      switch ( ::toupper( attrib.tag[0] ) ) {
      case 'D':
        m_outName = std::move( attrib.value );
        break;
      case 'S':
        m_criteria = std::move( attrib.value );
        break;
      case 'F':
        fun = std::move( attrib.value );
        break;
      default:
        break;
      }
    }
    if ( m_outName.empty() ) {
      log << MSG::ERROR << "Failed to analyze output specs:" << m_output << endmsg;
      return StatusCode::FAILURE;
    }
    if ( !fun.empty() || !m_criteria.empty() ) {
      if ( !m_criteria.empty() && fun.empty() ) fun = "NTuple::Selector";
      m_selectorName = fun;
    }
    return StatusCode::SUCCESS;
  }

  /// Finalize
  StatusCode finalize() override {
    m_dataSvc.reset();
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  StatusCode execute() override {
    StatusCode status = connect();
    return status.isSuccess() ? mergeInputTuples() : status;
  }

  /// Book the N-tuple according to the specification
  virtual StatusCode book( const NTuple::Tuple* nt ) {
    MsgStream      log( msgSvc(), name() );
    StatusCode     status = StatusCode::SUCCESS;
    NTuple::Tuple* tuple  = m_dataSvc->book( m_outName, nt->clID(), nt->title() );
    for ( const auto& i : nt->items() ) {
      switch ( i->type() ) {
      case DataTypeInfo::UCHAR:
        status = createItem<unsigned char>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::USHORT:
        status = createItem<unsigned short>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::UINT:
        status = createItem<unsigned int>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::ULONG:
        status = createItem<unsigned long>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::CHAR:
        status = createItem<char>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::SHORT:
        status = createItem<short>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::INT:
        status = createItem<int>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::LONG:
        status = createItem<long>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::BOOL:
        status = createItem( log, tuple, i, false );
        break;
      case DataTypeInfo::FLOAT:
        status = createItem<float>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::DOUBLE:
        status = createItem<double>( log, tuple, i, 0 );
        break;
      case DataTypeInfo::OBJECT_ADDR:
        status = createItem<IOpaqueAddress*>( log, tuple, i, nullptr );
        break;
      case DataTypeInfo::POINTER:
        status = createItem<void*>( log, tuple, i, nullptr );
        break;
      case DataTypeInfo::STRING:
      //        status = createItem(log, tuple, i, (std::string*)0);
      //        break;
      case DataTypeInfo::NTCHAR:
      //        status = createItem(log, tuple, i, (char*)0);
      //        break;
      case DataTypeInfo::UNKNOWN:
      default:
        status = StatusCode::FAILURE;
        break;
      }
    }
    return status;
  }

  // Perform some basic checks
  virtual StatusCode checkInput( const NTuple::Tuple* clone, const NTuple::Tuple* src ) {
    MsgStream log( msgSvc(), name() );
    if ( clone && src ) {
      const INTuple::ItemContainer& clone_items = clone->items();
      const std::string             clone_id    = clone->registry()->identifier();
      const std::string             src_id      = src->registry()->identifier();

      log << MSG::ERROR;
      for ( auto i = clone_items.begin(); i != clone_items.end(); ++i ) {
        const INTupleItem* itm     = *i;
        const std::string& nam     = itm->name();
        const INTupleItem* src_itm = src->find( nam );
        if ( !src_itm ) {
          log << "Tuple item " << nam << " not present in " << src_id << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->type() != src_itm->type() ) {
          log << "Tuple item " << nam << " are of different types in " << src_id << ":" << src_itm->typeName()
              << " <-> " << clone_id << ":" << itm->typeName() << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->ndim() != src_itm->ndim() ) {
          log << "Tuple item " << nam << " have different dimensions in " << src_id << ":" << src_itm->ndim() << " <-> "
              << clone_id << ":" << itm->ndim() << endmsg;
          return StatusCode::FAILURE;
        }
        for ( int j = 0; j < itm->ndim(); ++j ) {
          if ( src_itm->dim( j ) != itm->dim( j ) ) {
            log << "Tuple item " << nam << " have different dimensions in " << src_id << "[" << j
                << "]:" << src_itm->dim( j ) << " <-> " << clone_id << "[" << j << "]:" << itm->dim( j ) << endmsg;
            return StatusCode::FAILURE;
          }
        }
        if ( itm->hasIndex() != src_itm->hasIndex() ) {
          log << "Tuple item " << nam << " has different index colums " << src_id << ":" << src_itm->hasIndex()
              << " <-> " << clone_id << ":" << itm->hasIndex() << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->hasIndex() ) {
          if ( itm->index() != src_itm->index() ) {
            log << "Tuple item " << nam << " has different index colums " << src_id << ":" << src_itm->index()
                << " <-> " << clone_id << ":" << itm->index() << endmsg;
            return StatusCode::FAILURE;
          }
        }
      }
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  /// Merge the entries of a single input tuple into the output
  StatusCode mergeEntries( const std::string& input ) {
    MsgStream log( msgSvc(), name() );
    NTuplePtr out( m_dataSvc.get(), m_outName );
    if ( 0 != out ) {
      const INTuple::ItemContainer& clone_items = out->items();
      std::vector<GenericAddress>   addrVector( clone_items.size() );
      StatusCode                    status = StatusCode::SUCCESS;
      NTuplePtr                     nt( m_dataSvc.get(), input );
      size_t                        k = 0, nentry = 0;
      if ( 0 != nt ) {
        const INTuple::ItemContainer& source_items = nt->items();
        for ( k = 0; k < source_items.size(); ++k ) {
          if ( source_items[k]->type() == DataTypeInfo::OBJECT_ADDR ) {
            *(IOpaqueAddress**)source_items[k]->buffer() = &addrVector[k];
          }
        }
        while ( status.isSuccess() ) {
          status = m_dataSvc->readRecord( nt.ptr() );
          if ( status.isSuccess() ) {
            INTuple::ItemContainer::const_iterator i;
            nentry++;
            for ( k = 0, i = source_items.begin(); i != source_items.end(); ++i, ++k ) {
              const INTupleItem* src_itm = *i;
              const INTupleItem* out_itm = out->find( src_itm->name() );
              size_t             size    = 0;
              switch ( ( *i )->type() ) {
              case DataTypeInfo::UCHAR:
                size = sizeof( unsigned char );
                break;
              case DataTypeInfo::USHORT:
                size = sizeof( unsigned short );
                break;
              case DataTypeInfo::UINT:
                size = sizeof( unsigned int );
                break;
              case DataTypeInfo::ULONG:
                size = sizeof( unsigned long );
                break;
              case DataTypeInfo::CHAR:
                size = sizeof( char );
                break;
              case DataTypeInfo::SHORT:
                size = sizeof( short );
                break;
              case DataTypeInfo::INT:
                size = sizeof( int );
                break;
              case DataTypeInfo::LONG:
                size = sizeof( long );
                break;
              case DataTypeInfo::BOOL:
                size = sizeof( bool );
                break;
              case DataTypeInfo::FLOAT:
                size = sizeof( float );
                break;
              case DataTypeInfo::DOUBLE:
                size = sizeof( double );
                break;
              case DataTypeInfo::STRING:
                *(std::string*)out_itm->buffer() = *(std::string*)src_itm->buffer();
                size                             = 0;
                break;
              case DataTypeInfo::NTCHAR:
                size = ::strlen( (const char*)src_itm->buffer() ) + 1;
                break;
              case DataTypeInfo::POINTER: {
                *(void**)out_itm->buffer() = *(void**)src_itm->buffer();
                size                       = 0;
              } break;
              case DataTypeInfo::OBJECT_ADDR: {
                IOpaqueAddress*  ppA1 = &addrVector[k];
                IOpaqueAddress** ppA2 = (IOpaqueAddress**)( out_itm->buffer() );
                *ppA2                 = ppA1;
                size                  = 0;
              } break;
              case DataTypeInfo::UNKNOWN:
              default:
                size = 0;
                break;
              }
              if ( size > 0 ) {
                ::memcpy( const_cast<void*>( out_itm->buffer() ), src_itm->buffer(), size * src_itm->length() );
              }
            }
            status = m_dataSvc->writeRecord( out.ptr() );
            if ( !status.isSuccess() ) {
              log << MSG::ERROR << "Failed to write record " << nentry << " from " << input << " to " << m_outName
                  << endmsg;
            }
          }
        }
        log << MSG::INFO << "End of reading tuple " << input << " after " << nentry << " entries." << endmsg;

        if ( nentry > 0 || m_selectorName != "" ) { return StatusCode::SUCCESS; }
        return StatusCode::FAILURE;
      }
      log << MSG::ERROR << "Failed to access input: " << input << endmsg;
    }
    return StatusCode::FAILURE;
  }

  /// Connect input and output N-tuples
  StatusCode connect() {
    StatusCode status = StatusCode::SUCCESS;
    for ( size_t i = 0; i < m_inputs.size(); ++i ) {
      NTuplePtr nt( m_dataSvc.get(), m_inputs[i] );
      if ( !( 0 == nt ) ) {
        NTuplePtr out( m_dataSvc.get(), m_outName );
        if ( 0 == out ) {
          status = book( nt );
        } else {
          status = checkInput( out, nt );
        }
        if ( !status.isSuccess() ) {
          return status;
        } else if ( m_selectorName != "" ) {
          SmartIF<ISelectStatement> stmt( ObjFactory::create( m_selectorName, serviceLocator() ).release() );
          if ( stmt ) {
            if ( !m_criteria.empty() ) stmt->setCriteria( m_criteria );
            nt->attachSelector( stmt ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
          } else {
            MsgStream log( msgSvc(), name() );
            log << MSG::ERROR << "Failed to attach tuple selector to " << m_inputs[i] << endmsg;
            return StatusCode::FAILURE;
          }
        }
      } else {
        MsgStream log( msgSvc(), name() );
        log << MSG::ERROR << "Failed to access tuple: " << m_inputs[i] << endmsg;
        return StatusCode::FAILURE;
      }
    }
    return StatusCode::SUCCESS;
  }

  /// Merge all N-tuple entries
  StatusCode mergeInputTuples() {
    MsgStream log( msgSvc(), name() );
    for ( const auto& input : m_inputs ) {
      StatusCode sc = mergeEntries( input );
      if ( !sc.isSuccess() ) {
        log << MSG::ERROR << "Failed to merge tuple:" << input << endmsg;
        return sc;
      }
    }
    return StatusCode::SUCCESS;
  }
};
DECLARE_COMPONENT( CollectionCloneAlg )
