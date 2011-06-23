// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiSvc/src/NTupleSvc/CollectionCloneAlg.cpp,v 1.7 2006/12/06 17:18:05 mato Exp $
//	====================================================================
//  CollectionCloneAlg.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ISelectStatement.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/NTuple.h"
#include <vector>

namespace {
  template <class T> static long upper(const INTupleItem* item)  {
    const NTuple::_Data<T>* it = dynamic_cast<const NTuple::_Data<T>*>(item);
    return long(it->range().upper());
  }

  template<class TYP> static
  StatusCode createItem (MsgStream& log,
                        INTuple* tuple,
                        INTupleItem* src,
                        const TYP& null)
  {
    NTuple::_Data<TYP>* source = dynamic_cast<NTuple::_Data<TYP>*>(src);
    TYP low                    = source->range().lower();
    TYP high                   = source->range().upper();
    long hasIdx                = source->hasIndex();
    long ndim                  = source->ndim();
    const std::string& name    = source->name();
    std::string idxName;
    long dim[4], idxLen = 0;
    long dim1 = 1, dim2 = 1;
    INTupleItem* it = 0;
    for ( int i = 0; i < ndim; i++ )
      dim[i] = source->dim(i);
    /// Type information of the item
    if ( hasIdx )  {
      const INTupleItem* index = source->indexItem();
      idxName = index->name();
      switch( index->type() )    {
      case DataTypeInfo::UCHAR:
        idxLen = upper<unsigned char>(index);
        break;
      case DataTypeInfo::USHORT:
        idxLen = upper<unsigned short>(index);
        break;
      case DataTypeInfo::UINT:
        idxLen = upper<unsigned int>(index);
        break;
      case DataTypeInfo::ULONG:
        idxLen = upper<unsigned long>(index);
        break;
      case DataTypeInfo::CHAR:
        idxLen = upper<char>(index);
        break;
      case DataTypeInfo::SHORT:
        idxLen = upper<short>(index);
        break;
      case DataTypeInfo::INT:
        idxLen = upper<int>(index);
        break;
      case DataTypeInfo::LONG:
        idxLen = upper<long>(index);
        break;
      default:
        log << MSG::ERROR << "Column " << idxName
            << " is not a valid index column!" << endmsg;
        return StatusCode::FAILURE;
      }
    }
    switch( ndim )  {
    case 0:
      it = NTuple::_Item<TYP>::create (tuple, name, typeid(TYP), low, high, null);
      break;
    case 1:
      dim1 = (hasIdx) ? idxLen : dim[0];
      it = NTuple::_Array<TYP>::create (tuple,
                                        name,
                                        typeid(TYP),
                                        idxName,
                                        dim1,
                                        low,
                                        high,
                                        null);
      break;
    case 2:
      dim1 = (hasIdx) ? idxLen : dim[0];
      dim2 = (hasIdx) ? dim[0] : dim[1];
      it = NTuple::_Matrix<TYP>::create ( tuple,
                                          name,
                                          typeid(TYP),
                                          idxName,
                                          dim1,
                                          dim2,
                                          low,
                                          high,
                                          null);
      break;
    default:
      return StatusCode::FAILURE;
    }
    return tuple->add(it);
  }
}

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
  /// Reference to data provider service
  INTupleSvc*               m_dataSvc;
  /// Name of the data provider service
  std::string               m_tupleSvc;
  /// Output specification
  std::string               m_output;
  /// input specifications
  std::vector<std::string>  m_inputs;
  /// Name of the root leaf (obtained at initialize)
  std::string               m_rootName;
  /// Output tuple name
  std::string               m_outName;
  /// Selection criteria (if any)
  std::string               m_criteria;
  /// Selector factory
  std::string               m_selectorName;
public:

  /// Standard algorithm constructor
  CollectionCloneAlg(const std::string& name, ISvcLocator* pSvcLocator)
  :	Algorithm(name, pSvcLocator),  m_dataSvc(0)
  {
    declareProperty("EvtTupleSvc", m_tupleSvc="EvtTupleSvc");
    declareProperty("Input",       m_inputs);
    declareProperty("Output",      m_output);
  }
  /// Standard Destructor
  virtual ~CollectionCloneAlg()     {
  }

  /// Initialize
  virtual StatusCode initialize()   {
    MsgStream log(msgSvc(), name());
    m_rootName = "";
    m_outName = "";
    m_criteria = "";
    m_selectorName = "";
    StatusCode sc = service(m_tupleSvc, m_dataSvc, true);
    if ( sc.isSuccess() )  {
      std::string fun;
      Tokenizer tok(true);
      tok.analyse(m_output, " ", "", "", "=", "'", "'");
      for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); i++ )   {
        const std::string& tag = (*i).tag();
        const std::string& val = (*i).value();
        switch( ::toupper(tag[0]) )    {
        case 'D':
          m_outName = val;
          break;
        case 'S':
          m_criteria = val;
          break;
        case 'F':
          fun = val;
          break ;
        default:
          break;
        }
      }
      if ( m_outName.empty() )  {
        log << MSG::ERROR << "Failed to analyze output specs:" << m_output << endmsg;
        return StatusCode::FAILURE;
      }
      if ( fun.length() > 0 || m_criteria.length() > 0 )   {
        if ( m_criteria.length() > 0 && fun.length() == 0 ) fun = "NTuple::Selector";
        m_selectorName = fun;
        return StatusCode::SUCCESS;
      }
      return sc;
    }
    log << MSG::ERROR << "Failed to access service \""
        << m_tupleSvc << "\"." << endmsg;
    return sc;
  }

  /// Finalize
  virtual StatusCode finalize() {
    if ( m_dataSvc ) m_dataSvc->release();
    m_dataSvc = 0;
    return StatusCode::SUCCESS;
  }

  /// Execute procedure
  virtual StatusCode execute()    {
    StatusCode status = connect();
    if ( status.isSuccess() )  {
      status = mergeInputTuples();
    }
    return status;
  }

  /// Book the N-tuple according to the specification
  virtual StatusCode book(const NTuple::Tuple* nt)  {
    MsgStream log(msgSvc(), name());
    const INTuple::ItemContainer& items = nt->items();
    StatusCode status = StatusCode::SUCCESS;
    INTuple::ItemContainer::const_iterator i;
    NTuple::Tuple* tuple = m_dataSvc->book(m_outName, nt->clID(), nt->title());
    for (i = items.begin(); i != items.end(); ++i)  {
      long type = (*i)->type();
      switch(type)  {
      case DataTypeInfo::UCHAR:
        status = createItem(log, tuple, *i, (unsigned char)0);
        break;
      case DataTypeInfo::USHORT:
        status = createItem(log, tuple, *i, (unsigned short)0);
        break;
      case DataTypeInfo::UINT:
        status = createItem(log, tuple, *i, (unsigned int)0);
        break;
      case DataTypeInfo::ULONG:
        status = createItem(log, tuple, *i, (unsigned long)0);
        break;
      case DataTypeInfo::CHAR:
        status = createItem(log, tuple, *i, char(0));
        break;
      case DataTypeInfo::SHORT:
        status = createItem(log, tuple, *i, short(0));
        break;
      case DataTypeInfo::INT:
        status = createItem(log, tuple, *i, int(0));
        break;
      case DataTypeInfo::LONG:
        status = createItem(log, tuple, *i, long(0));
        break;
      case DataTypeInfo::BOOL:
        status = createItem(log, tuple, *i, false);
        break;
      case DataTypeInfo::FLOAT:
        status = createItem(log, tuple, *i, float(0.0));
        break;
      case DataTypeInfo::DOUBLE:
        status = createItem(log, tuple, *i, double(0.0));
        break;
      case DataTypeInfo::OBJECT_ADDR:
        status = createItem(log, tuple, *i, (IOpaqueAddress*)0);
        break;
      case DataTypeInfo::POINTER:
        status = createItem(log, tuple, *i, (void*)0);
        break;
      case DataTypeInfo::STRING:
//        status = createItem(log, tuple, *i, (std::string*)0);
//        break;
      case DataTypeInfo::NTCHAR:
//        status = createItem(log, tuple, *i, (char*)0);
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
  virtual StatusCode checkInput(const NTuple::Tuple* clone, const NTuple::Tuple* src)  {
    MsgStream log(msgSvc(), name());
    if ( 0 != clone && 0 != src )   {
      const INTuple::ItemContainer& clone_items  = clone->items();
      const std::string clone_id = clone->registry()->identifier();
      const std::string src_id = src->registry()->identifier();

      INTuple::ItemContainer::const_iterator i;
      log << MSG::ERROR;
      for (i = clone_items.begin(); i != clone_items.end(); ++i)  {
        const INTupleItem* itm = *i;
        const std::string& nam = itm->name();
        const INTupleItem* src_itm = src->find(nam);
        if ( !src_itm )  {
          log << "Tuple item " << nam << " not present in " << src_id << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->type() != src_itm->type() )  {
          log << "Tuple item " << nam << " are of different types in "
            << src_id   << ":" << src_itm->typeName() << " <-> "
            << clone_id << ":" << itm->typeName() << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->ndim() != src_itm->ndim() )  {
          log << "Tuple item " << nam << " have different dimensions in "
            << src_id   << ":" << src_itm->ndim() << " <-> "
            << clone_id << ":" << itm->ndim() << endmsg;
          return StatusCode::FAILURE;
        }
        for (int j=0; j<itm->ndim(); ++j)  {
          if ( src_itm->dim(j) != itm->dim(j) )  {
            log << "Tuple item " << nam << " have different dimensions in "
              << src_id   << "[" << j << "]:" << src_itm->dim(j) << " <-> "
              << clone_id << "[" << j << "]:" << itm->dim(j) << endmsg;
            return StatusCode::FAILURE;
          }
        }
        if ( itm->hasIndex() != src_itm->hasIndex() )  {
          log << "Tuple item " << nam << " has different index colums "
            << src_id   << ":" << src_itm->hasIndex() << " <-> "
            << clone_id << ":" << itm->hasIndex() << endmsg;
          return StatusCode::FAILURE;
        }
        if ( itm->hasIndex() )  {
          if ( itm->index() != src_itm->index() )  {
            log << "Tuple item " << nam << " has different index colums "
              << src_id   << ":" << src_itm->index() << " <-> "
              << clone_id << ":" << itm->index() << endmsg;
            return StatusCode::FAILURE;
          }
        }
      }
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  /// Merge the entries of a single input tuple into the output
  StatusCode mergeEntries(const std::string& input)  {
    MsgStream log(msgSvc(), name());
    NTuplePtr out(m_dataSvc, m_outName);
    if ( 0 != out )  {
      const INTuple::ItemContainer& clone_items  = out->items();
      std::vector<GenericAddress> addrVector(clone_items.size());
      StatusCode status = StatusCode::SUCCESS;
      NTuplePtr nt(m_dataSvc, input);
      size_t k = 0, nentry = 0;
      if ( 0 != nt ) {
        const INTuple::ItemContainer& source_items = nt->items();
        for (k=0; k < source_items.size(); ++k )  {
          if ( source_items[k]->type() == DataTypeInfo::OBJECT_ADDR )  {
            *(IOpaqueAddress**)source_items[k]->buffer() = &addrVector[k];
          }
        }
        while ( status.isSuccess() ) {
          status = m_dataSvc->readRecord(nt.ptr());
          if ( status.isSuccess() )   {
            INTuple::ItemContainer::const_iterator i;
            nentry++;
            for (k=0,i = source_items.begin(); i != source_items.end(); ++i,++k)  {
              const INTupleItem* src_itm = *i;
              const INTupleItem* out_itm = out->find(src_itm->name());
              size_t size = 0;
              switch((*i)->type())  {
              case DataTypeInfo::UCHAR:
                size = sizeof(unsigned char);
                break;
              case DataTypeInfo::USHORT:
                size = sizeof(unsigned short);
                break;
              case DataTypeInfo::UINT:
                size = sizeof(unsigned int);
                break;
              case DataTypeInfo::ULONG:
                size = sizeof(unsigned long);
                break;
              case DataTypeInfo::CHAR:
                size = sizeof(char);
                break;
              case DataTypeInfo::SHORT:
                size = sizeof(short);
                break;
              case DataTypeInfo::INT:
                size = sizeof(int);
                break;
              case DataTypeInfo::LONG:
                size = sizeof(long);
                break;
              case DataTypeInfo::BOOL:
                size = sizeof(bool);
                break;
              case DataTypeInfo::FLOAT:
                size = sizeof(float);
                break;
              case DataTypeInfo::DOUBLE:
                size = sizeof(double);
                break;
              case DataTypeInfo::STRING:
                *(std::string*)out_itm->buffer() = *(std::string*)src_itm->buffer();
                size = 0;
                break;
              case DataTypeInfo::NTCHAR:
                size = ::strlen((const char*)src_itm->buffer())+1;
                break;
              case DataTypeInfo::POINTER:
                {
                  *(void**)out_itm->buffer() = *(void**)src_itm->buffer();
                  size = 0;
                }
                break;
              case DataTypeInfo::OBJECT_ADDR:
                {
                IOpaqueAddress* ppA1  = &addrVector[k];
                IOpaqueAddress** ppA2 = (IOpaqueAddress**)out_itm->buffer();
                *ppA2 = ppA1;
                size = 0;
                }
                break;
              case DataTypeInfo::UNKNOWN:
              default:
                size = 0;
                break;
              }
              if ( size > 0 )  {
                ::memcpy((void*)out_itm->buffer(), src_itm->buffer(), size*src_itm->length());
              }
            }
            status = m_dataSvc->writeRecord(out.ptr());
            if ( !status.isSuccess() )  {
              log << MSG::ERROR << "Failed to write record " << nentry
                  << " from " << input << " to " << m_outName << endmsg;
            }
          }
        }
        log << MSG::INFO << "End of reading tuple " << input
            << " after " << nentry << " entries." << endmsg;

        if ( nentry > 0 || m_selectorName != "" )  {
          return StatusCode::SUCCESS;
        }
        return StatusCode::FAILURE;
      }
      log << MSG::ERROR << "Failed to access input: " << input << endmsg;
    }
    return StatusCode::FAILURE;
  }

  /// Connect input and output N-tuples
  StatusCode connect()   {
    StatusCode status = StatusCode::SUCCESS;
    for (size_t i=0; i < m_inputs.size(); ++i)  {
      NTuplePtr nt(m_dataSvc, m_inputs[i]);
      if ( !(0 == nt) )    {
        NTuplePtr out(m_dataSvc, m_outName);
        if ( 0 == out )  {
          status = book(nt);
        }
        else  {
          status = checkInput(out, nt);
        }
        if ( !status.isSuccess() )  {
          return status;
        }
        else if ( m_selectorName != "" )   {
          SmartIF<ISelectStatement> stmt(ROOT::Reflex::PluginService::Create<IInterface*>(m_selectorName,serviceLocator()));
          if ( stmt.isValid( ) )    {
            if ( m_criteria.length() > 0 ) stmt->setCriteria(m_criteria);
            nt->attachSelector(stmt);
          }
          else  {
            MsgStream log(msgSvc(), name());
            log << MSG::ERROR << "Failed to attach tuple selector to " << m_inputs[i] << endmsg;
            return StatusCode::FAILURE;
          }
        }
      }
      else  {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "Failed to access tuple: " << m_inputs[i] << endmsg;
        return StatusCode::FAILURE;
      }
    }
    return StatusCode::SUCCESS;
  }

  /// Merge all N-tuple entries
  StatusCode mergeInputTuples()  {
    MsgStream log(msgSvc(), name());
    for (size_t inp=0; inp < m_inputs.size(); ++inp)  {
      StatusCode sc = mergeEntries(m_inputs[inp]);
      if ( !sc.isSuccess() )  {
        log << MSG::ERROR << "Failed to merge tuple:" << m_inputs[inp] << endmsg;
        return sc;
      }
    }
    return StatusCode::SUCCESS;
  }
};
DECLARE_ALGORITHM_FACTORY(CollectionCloneAlg)
