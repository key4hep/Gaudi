/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Rust/AlgWrapper.h>
#include <GaudiKernel/IProperty.h>
#include <gaudi_rust_bindings_bridge/lib.h>
#include <string>
#include <string_view>
#include <type_traits>

namespace {
  class GenericProperty : public Gaudi::Details::PropertyBase {
  public:
    GenericProperty( std::string semantics, std::string name, std::string defaultValue, std::string doc )
        : PropertyBase( typeid( void ), std::move( name ), std::move( doc ), std::move( semantics ) )
        , m_value( std::move( defaultValue ) ) {}

    bool load( PropertyBase& dest ) const override { return dest.assign( *this ); }
    bool assign( const PropertyBase& source ) override {
      m_value = source.toString();
      return true;
    }
    std::string toString() const override { return m_value; }
    void        toStream( std::ostream& out ) const override { out << m_value; }
    StatusCode  fromString( const std::string& value ) override {
      m_value = value;
      return StatusCode::SUCCESS;
    }
    PropertyBase& declareReadHandler( std::function<void( PropertyBase& )> ) override {
      throw std::logic_error( "GrenericProperty::declareReadHandler not implemented" );
    }
    PropertyBase& declareUpdateHandler( std::function<void( PropertyBase& )> ) override {
      throw std::logic_error( "GrenericProperty::declareUpdateHandler not implemented" );
    }
    const std::function<void( PropertyBase& )> readCallBack() const override {
      throw std::logic_error( "GrenericProperty::readCallBack not implemented" );
    }
    const std::function<void( PropertyBase& )> updateCallBack() const override {
      throw std::logic_error( "GrenericProperty::updateCallBack not implemented" );
    }
    bool          useUpdateHandler() override { return true; }
    PropertyBase* clone() const override { throw std::logic_error( "GrenericProperty::clone not implemented" ); }

  private:
    std::string m_value;
  };
} // namespace

namespace Gaudi::Rust {
  AlgWrapper::AlgWrapper( std::string const& name, ISvcLocator* svcLoc, details::WrappedAlg* dyn_alg_ptr )
      : Algorithm( name, svcLoc ), m_dyn_alg_ptr( dyn_alg_ptr ) {
    if ( m_dyn_alg_ptr ) alg_bind_host( m_dyn_alg_ptr, *this );
  }

  AlgWrapper::~AlgWrapper() {
    if ( m_dyn_alg_ptr ) alg_drop( m_dyn_alg_ptr );
  }

  StatusCode AlgWrapper::initialize() {
    return Algorithm::initialize().andThen( [&]() { alg_initialize( m_dyn_alg_ptr, *this ); } );
  }
  StatusCode AlgWrapper::start() {
    return Algorithm::start().andThen( [&]() { alg_start( m_dyn_alg_ptr, *this ); } );
  }
  StatusCode AlgWrapper::execute( const EventContext& ctx ) const {
    alg_execute( m_dyn_alg_ptr, *this, ctx );
    return StatusCode::SUCCESS; // alg_execute(...) throws on error
  }
  StatusCode AlgWrapper::stop() {
    alg_stop( m_dyn_alg_ptr, *this );
    return Algorithm::stop();
  }
  StatusCode AlgWrapper::finalize() {
    alg_finalize( m_dyn_alg_ptr, *this );
    return Algorithm::finalize();
  }

  void AlgWrapper::addProperty( std::string const& semantics, std::string const& name, std::string const& defaultValue,
                                std::string const& doc ) const {
    auto mut_self = const_cast<AlgWrapper*>( this );
    mut_self->declareProperty( *mut_self->m_genProperties.emplace_back(
        std::make_unique<GenericProperty>( semantics, name, defaultValue, doc ) ) );
  }
  std::string AlgWrapper::getPropertyValue( std::string const& name ) const { return getProperty( name ).toString(); }

  void AlgWrapper::addInputHandle( std::string const& type_name, std::string const& name, std::string const& location,
                                   std::string const& doc ) const {
    auto mut_self = const_cast<AlgWrapper*>( this );
    mut_self->m_dataHandles.emplace(
        std::piecewise_construct, std::forward_as_tuple( name ),
        std::forward_as_tuple( mut_self, Gaudi::DataHandle::Reader, type_name, name, location, doc ) );
  }
  void AlgWrapper::addOutputHandle( std::string const& type_name, std::string const& name, std::string const& location,
                                    std::string const& doc ) const {
    auto mut_self = const_cast<AlgWrapper*>( this );
    mut_self->m_dataHandles.emplace(
        std::piecewise_construct, std::forward_as_tuple( name ),
        std::forward_as_tuple( mut_self, Gaudi::DataHandle::Writer, type_name, name, location, doc ) );
  }
} // namespace Gaudi::Rust
