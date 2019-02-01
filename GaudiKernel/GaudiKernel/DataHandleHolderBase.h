#ifndef GAUDIKERNEL_DATAHANDLEHOLDERBASE
#define GAUDIKERNEL_DATAHANDLEHOLDERBASE 1

#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/Property.h"

#include <algorithm>
#include <unordered_set>

namespace {
  template <typename Container>
  std::vector<Gaudi::DataHandle*> handles( Container& c, Gaudi::DataHandle::Mode mode ) {
    std::vector<Gaudi::DataHandle*> h;
    std::copy_if( std::begin( c ), std::end( c ), std::back_inserter( h ),
                  [&]( const Gaudi::DataHandle* hndl ) -> bool { return hndl->mode() & mode; } );
    return h;
  }
} // namespace

template <class BASE>
class GAUDI_API DataHandleHolderBase : public extends<BASE, IDataHandleHolder> {
public:
  using extends<BASE, IDataHandleHolder>::extends;

  std::vector<Gaudi::DataHandle*> inputHandles() const override {
    return handles( m_handles, Gaudi::DataHandle::Reader );
  }
  std::vector<Gaudi::DataHandle*> outputHandles() const override {
    return handles( m_handles, Gaudi::DataHandle::Writer );
  }

  virtual const DataObjIDColl& extraInputDeps() const override { return m_extInputDataObjs; }
  virtual const DataObjIDColl& extraOutputDeps() const override { return m_extOutputDataObjs; }

  void declare( Gaudi::DataHandle& handle ) override {
    if ( !handle.owner() ) handle.setOwner( this );

    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to declare foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }

    m_handles.insert( &handle );
  }

  void renounce( Gaudi::DataHandle& handle ) override {
    if ( handle.owner() != this ) {
      throw GaudiException( "Attempt to renounce foreign handle with algorithm!", this->name(), StatusCode::FAILURE );
    }
    m_handles.erase( &handle );
  }

  const DataObjIDColl& inputDataObjs() const override { return m_inputDataObjs; }
  const DataObjIDColl& outputDataObjs() const override { return m_outputDataObjs; }

  void addDependency( const DataObjID& id, const Gaudi::DataHandle::Mode& mode ) override {
    if ( mode & Gaudi::DataHandle::Reader ) m_inputDataObjs.emplace( id );
    if ( mode & Gaudi::DataHandle::Writer ) m_outputDataObjs.emplace( id );
  }

private:
protected:
  /// initializes all handles - called by the sysInitialize method
  /// of any descendant of this
  void initDataHandleHolder() {
    for ( auto h : m_handles ) h->init();
  }

  DataObjIDColl m_inputDataObjs, m_outputDataObjs;

private:
  std::unordered_set<Gaudi::DataHandle*> m_handles;

  Gaudi::Property<DataObjIDColl> m_extInputDataObjs{this, "ExtraInputs", DataObjIDColl{}};
  Gaudi::Property<DataObjIDColl> m_extOutputDataObjs{this, "ExtraOutputs", DataObjIDColl{}};
};

#endif // !GAUDIKERNEL_DATAHANDLEHOLDERBASE
