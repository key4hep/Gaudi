/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/details/BranchReadHelper.h>
#include <TClass.h>
#include <TDataType.h>
#include <TInterpreter.h>
#include <TInterpreterValue.h>
#include <sstream>

namespace Gaudi::details {

  BranchReadHelper::BranchReadHelper( gsl::not_null<TBranch*> b ) : m_branch( b ) {
    TClass*   clptr    = nullptr;
    EDataType datatype = kNoType_t;
    b->GetExpectedType( clptr, datatype );
    // 64 bits of storage are enough for reading from the TBranch:
    // for simple types, we can just read the value directly into
    // this storage, and for complex types, we can read the address
    // of the object into this storage
    b->SetAddress( &m_store );

    // Here we compile a function that converts the value hold in m_store
    // into a DataObject. This is done by directly returning the pointer
    // if the type inherits from DataObject, or by creating a new AnyDataWrapper.
    std::ostringstream code;
    code << "new std::function<DataObject*(void*)>([] (void* store) -> DataObject* {";
    if ( clptr ) {
      // complex type case: store points to an instance of the object
      code << "auto ptr = reinterpret_cast<" << clptr->GetName() << "*>(store);";
      if ( clptr->InheritsFrom( TClass::GetClass<DataObject>() ) ) {
        // for DataObject, just return the pointer
        code << "return ptr;";
      } else {
        // for other complex types, create a new AnyDataWrapper from the object
        code << "return new AnyDataWrapper(std::move(*ptr));";
      }
    } else {
      // simple type case: store is the actual value
      code << TDataType::GetTypeName( datatype ) << " value;";
      code << "std::memcpy(&value, &store, sizeof(value));";
      code << "return new AnyDataWrapper(std::move(value));";
    }
    code << "})";
    gInterpreter->Declare( "#include <cstring>\n#include <functional>\n#include <GaudiKernel/AnyDataWrapper.h>\n" );
    auto val = gInterpreter->MakeInterpreterValue();
    if ( gInterpreter->Evaluate( code.str().c_str(), *val ) == 0 ) {
      throw std::runtime_error( "Failed to compile code" );
    }
    m_extractor = std::unique_ptr<std::function<DataObject*( void* )>>(
        reinterpret_cast<std::function<DataObject*( void* )>*>( val->GetAsPointer() ) );
  }

  BranchReadHelper::BranchReadHelper( BranchReadHelper&& other ) noexcept {
    m_branch    = other.m_branch;
    m_store     = other.m_store;
    m_extractor = std::move( other.m_extractor );

    other.m_branch = nullptr;
    other.m_store  = nullptr;
  }

  BranchReadHelper& BranchReadHelper::operator=( BranchReadHelper&& other ) noexcept {
    if ( this == &other ) { return *this; }

    m_branch    = other.m_branch;
    m_store     = other.m_store;
    m_extractor = std::move( other.m_extractor );

    other.m_branch = nullptr;
    other.m_store  = nullptr;

    return *this;
  }

  BranchReadHelper::~BranchReadHelper() { m_branch->SetAddress( nullptr ); }

  std::unique_ptr<DataObject> BranchReadHelper::adoptValue() {
    std::unique_ptr<DataObject> out( ( *m_extractor )( m_store ) );
    m_store = nullptr;
    return out;
  }

} // namespace Gaudi::details
