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
// ============================================================================
// STD:
// ============================================================================
#include <iostream>
#include <memory>
// ============================================================================
// BOOST:
// ============================================================================
#include <boost/format.hpp>
// ============================================================================
#include "Analyzer.h"
#include "Catalog.h"
#include "IncludedFiles.h"
#include "Messages.h"
#include "Node.h"
#include "Parser.h"
#include "PragmaOptions.h"
#include "PropertyName.h"
#include "PropertyValue.h"
#include "Units.h"
// ============================================================================
#include "GaudiKernel/Environment.h"
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
static bool IncludeNode( gp::Node* node, const std::string& search_path, gp::IncludedFiles* included,
                         gp::Messages* messages ) {
  gp::Node include_root;
  bool     status = gp::Parse( node->position, node->value, search_path, included, messages, &include_root );
  if ( !status ) return false;
  node->value = include_root.value; // Save absolute file path
  node->children.reserve( node->children.size() + include_root.children.size() );
  std::copy( std::begin( include_root.children ), std::end( include_root.children ),
             std::back_inserter( node->children ) );
  return true;
}
// ============================================================================
static bool UnitsNode( gp::Node* node, const std::string& search_path, gp::IncludedFiles* included,
                       gp::Messages* messages ) {
  gp::Node units_root;
  bool     status = gp::ParseUnits( node->position, node->value, search_path, included, messages, &units_root );
  if ( !status ) return false;
  node->value = units_root.value; // Save absolute file path
  node->children.reserve( node->children.size() + units_root.children.size() );
  std::copy( std::begin( units_root.children ), std::end( units_root.children ), std::back_inserter( node->children ) );
  return true;
}
// ============================================================================
static std::unique_ptr<gp::PropertyName> GetPropertyName( const gp::Node* node ) {
  if ( node->children.size() == 1 ) {
    return std::make_unique<gp::PropertyName>( node->children[0].value, node->position );
  }
  std::string delim;
  std::string client;
  for ( unsigned int i = 0; i < ( node->children.size() - 1 ); ++i ) {
    client += delim + node->children[i].value;
    delim = '.';
  }
  return std::make_unique<gp::PropertyName>( client, node->children[node->children.size() - 1].value, node->position );
}
// ============================================================================
static std::unique_ptr<gp::PropertyValue> GetPropertyValue( const gp::Node* node, gp::Catalog* catalog,
                                                            gp::Units* units ) {
  std::unique_ptr<gp::PropertyValue> value;
  switch ( node->type ) {
  // ------------------------------------------------------------------------
  case gp::Node::kReal: {
    //  Example:
    //  <real value="10" line="3" column="7">
    //     <identifier value="m" line="3" column="10"/>
    //  </real>
    //
    if ( node->children.size() == 1 ) {
      // Unit is presented
      std::string unit_name  = node->children[0].value;
      double      unit_value = 0;
      if ( units->Find( unit_name, unit_value ) ) {
        // We have found a unit
        double val = std::stod( node->value );
        value      = std::make_unique<gp::PropertyValue>( std::to_string( val * unit_value ) );
      } else {
        // Unit not found
        throw gp::PositionalPropertyValueException::CouldNotFindUnit( node->children[0].position, unit_name );
      }
    } else {
      value = std::make_unique<gp::PropertyValue>( node->value );
    }
    break;
  }
  // ------------------------------------------------------------------------
  case gp::Node::kString: {
    std::stringstream ss;
    ss << std::quoted( node->value );
    value = std::make_unique<gp::PropertyValue>( ss.str() );
    break;
  }
  // ------------------------------------------------------------------------
  case gp::Node::kBool: {
    value = std::make_unique<gp::PropertyValue>( node->value );
    break;
  }
  // ------------------------------------------------------------------------
  case gp::Node::kVector: {
    std::vector<std::string> result;
    result.reserve( node->children.size() );
    std::transform( std::begin( node->children ), std::end( node->children ), std::back_inserter( result ),
                    [&]( const gp::Node& child ) { return GetPropertyValue( &child, catalog, units )->ToString(); } );
    value = std::make_unique<gp::PropertyValue>( std::move( result ) );
    break;
  }
  // ------------------------------------------------------------------------
  case gp::Node::kMap: {
    std::map<std::string, std::string> result;
    for ( const auto& child : node->children ) {
      auto kvalue = GetPropertyValue( &child.children[0], catalog, units );
      auto vvalue = GetPropertyValue( &child.children[1], catalog, units );
      result.emplace( kvalue->ToString(), vvalue->ToString() );
    }
    value = std::make_unique<gp::PropertyValue>( std::move( result ) );
    break;
  }
  // ------------------------------------------------------------------------
  case gp::Node::kProperty: {
    auto          property = GetPropertyName( node );
    gp::Property* exists   = catalog->Find( property->client(), property->property() );
    if ( exists ) {
      value = std::make_unique<gp::PropertyValue>( exists->property_value() );
    } else {
      throw gp::PositionalPropertyValueException::CouldNotFindProperty( node->position, property->ToString() );
    }
    break;
  }
  case gp::Node::kPropertyRef: {
    auto property = GetPropertyName( node );
    // Save a property reference as vector [clientname, property]
    std::vector<std::string> reference;
    reference.push_back( property->client() );
    reference.push_back( property->property() );

    value = std::make_unique<gp::PropertyValue>( std::move( reference ), property->position(), true );
    break;
  }
  // ------------------------------------------------------------------------
  default: {
    assert( false );
    break;
  }
  }
  return value;
}

// ============================================================================
static std::string SignString( gp::Node::NodeType type ) {
  switch ( type ) {
  case gp::Node::kEqual: {
    return "=";
  }

  case gp::Node::kPlusEqual: {
    return "+=";
  }

  case gp::Node::kMinusEqual: {
    return "-=";
  }
  default: {
    assert( false );
    break;
  }
  }
  return "unknown";
}
// ============================================================================
static bool AssignNode( const gp::Node* node, gp::Messages* messages, gp::Catalog* catalog, gp::Units* units,
                        bool is_print ) {
  // ----------------------------------------------------------------------------
  std::unique_ptr<gp::PropertyValue> value;
  // ----------------------------------------------------------------------------
  auto property = GetPropertyName( &node->children[0] );
  try {
    value = GetPropertyValue( &node->children[2], catalog, units );
  } catch ( const gp::PositionalPropertyValueException& ex ) {
    messages->AddError( ex.position(), ex.what() );
    return false;
  }
  // ------------------------------------------------------------------------
  bool          reassign = false;
  gp::Property* exists   = catalog->Find( property->client(), property->property() );
  // ----------------------------------------------------------------------------
  if ( exists ) {
    // ----------------------------------------------------------------------
    // If property already exists:
    // ----------------------------------------------------------------------
    try {
      if ( node->children[1].type == gp::Node::kEqual ) {
        std::string message = str( boost::format( "Reassignment of option '%1%' ." ) % property->FullName() );
        if ( exists->HasDefinedPosition() ) {
          message += " Previously defined at " + exists->DefinedPosition().ToString() + ".";
        }
        reassign = true;
        // INFO: we don't need this warning
        // messages->AddWarning(node->position, message);
      } else if ( node->children[1].type == gp::Node::kPlusEqual ) {
        *exists += *value;
      } else if ( node->children[1].type == gp::Node::kMinusEqual ) {
        *exists -= *value;
      }
    } catch ( const gp::PropertyValueException& ex ) {
      std::string message = ex.what();
      if ( exists->HasDefinedPosition() ) {
        message += " Previously defined at " + exists->DefinedPosition().ToString() + ".";
      }
      messages->AddError( node->position, message );
      return false;
    }
  }
  // ----------------------------------------------------------------------------
  bool result = true;
  if ( !exists || reassign ) { result = catalog->Add( new gp::Property( *property, *value ) ); }

  if ( result && is_print ) { /*;%|72t|%2% %3%*/
    std::string message = str( boost::format( "%1% %2% %3%" ) % property->FullName() %
                               SignString( node->children[1].type ) % value->ToString() );
    messages->AddInfo( node->position, message );
  }
  return result;
}
// ============================================================================
static bool UnitNode( const gp::Node* node, gp::Messages* messages, gp::Units* units, bool is_print ) {
  // --------------------------------------------------------------------------
  double      left  = std::stod( node->children[0].value );
  std::string name  = node->children[1].value;
  double      right = std::stod( node->children[2].value );
  // --------------------------------------------------------------------------
  gp::Units::Container::mapped_type exists;
  if ( units->Find( name, exists ) ) {
    std::string message = str( boost::format( "Unit '%1%' already defined" ) % name );
    if ( exists.second.Exists() ) { message += " at " + exists.second.ToString(); }
    messages->AddError( node->children[1].position, message );
    return false;
  }
  // --------------------------------------------------------------------------
  bool result = units->Add( name, right / left, node->children[1].position );
  if ( result && is_print ) {
    std::string message = str( boost::format( "%1% %2% = %3%" ) % left % name % right );
    messages->AddInfo( node->position, message );
  }
  return result;
}
// ============================================================================
static bool ConditionNode( gp::Node* node, gp::Catalog* catalog, gp::Node** next ) {
  // ----------------------------------------------------------------------------
  auto property_name = GetPropertyName( &node->children[0] );
  // --------------------------------------------------------------------------
  bool is_defined = ( nullptr != catalog->Find( property_name->client(), property_name->property() ) );
  // --------------------------------------------------------------------------
  if ( ( is_defined && ( node->children[1].type == gp::Node::kIfdef ) ) ||
       ( !is_defined && ( node->children[1].type == gp::Node::kIfndef ) ) ) {
    *next = &node->children[1];
  } else if ( node->children.size() > 2 ) {
    *next = &node->children[2];
  } else {
    *next = nullptr;
  }
  return true;
}
// ============================================================================
static bool Analyze( gp::Node* node, const std::string& search_path, gp::IncludedFiles* included,
                     gp::Messages* messages, gp::Catalog* catalog, gp::Units* units, gp::PragmaOptions* pragma ) {
  // ----------------------------------------------------------------------------
  bool      result       = true;
  bool      local_result = true;
  bool      skip_childs  = true;
  gp::Node* next_root    = node;
  // ------------------------------------------------------------------------
  switch ( node->type ) {
  // ------------------------------------------------------------------------
  case gp::Node::kRoot: {
    skip_childs = false;
    break;
  }
  // ----------------------------------------------------------------------
  case gp::Node::kInclude: {
    local_result = IncludeNode( node, search_path, included, messages );
    skip_childs  = false;
    break;
  }
  // ----------------------------------------------------------------------
  case gp::Node::kUnits: {
    local_result = UnitsNode( node, search_path, included, messages );
    skip_childs  = false;
    break;
  }
  // ----------------------------------------------------------------------
  case gp::Node::kAssign: {
    local_result = AssignNode( node, messages, catalog, units, pragma->is_print() );
    break;
  }
  // ----------------------------------------------------------------------
  case gp::Node::kUnit: {
    local_result = UnitNode( node, messages, units, pragma->is_print() );
    break;
  }
  // ----------------------------------------------------------------------
  case gp::Node::kCondition: {
    local_result = ConditionNode( node, catalog, &next_root );
    skip_childs  = false;
    break;
  }

  case gp::Node::kPrintOptions: {
    pragma->setIsPrintOptions( true );
    break;
  }

  case gp::Node::kPrintOn: {
    pragma->setIsPrint( true );
    break;
  }

  case gp::Node::kPrintOff: {
    pragma->setIsPrint( false );
    break;
  }

  case gp::Node::kPrintTree: {
    pragma->setIsPrintTree( true );
    break;
  }

  case gp::Node::kDumpFile: {
    std::string file = "";
    if ( System::resolveEnv( node->value, file ) ) {
      pragma->setDumpFile( file );
    } else {
      pragma->setDumpFile( node->value );
    }
    break;
  }
  // ----------------------------------------------------------------------
  default: {
    break;
  }
  }
  if ( result ) result = local_result;

  if ( !skip_childs && next_root ) {
    for ( auto& child : next_root->children ) {
      local_result = Analyze( &child, search_path, included, messages, catalog, units, pragma );
      if ( result ) result = local_result;
    }
  }
  return result;
}

bool Unreference( gp::Catalog& catalog, gp::Messages* messages ) {
  bool unreference_result = true;
  for ( auto& client : catalog ) {
    for ( auto& current : client.second ) {
      if ( current.IsReference() ) {
        gp::PropertyValue&              value    = current.property_value();
        const std::vector<std::string>& names    = value.Vector();
        gp::Property*                   property = catalog.Find( names[0], names[1] );
        if ( !property ) {
          messages->AddError( value.position(), "Could not unreference " + current.ValueAsString() );
          unreference_result = false;
        } else {
          value = property->property_value();
        }
      }
    }
  }
  return unreference_result;
}

// ============================================================================
bool gp::ReadOptions( const std::string& filename, const std::string& search_path, Messages* messages, Catalog* catalog,
                      Units* units, PragmaOptions* pragma, Node* root ) {
  // Extract Path
  IncludedFiles included;
  bool          result = Parse( filename, search_path, &included, messages, root );
  if ( !result ) return false;

  bool result1 = Analyze( root, search_path, &included, messages, catalog, units, pragma );
  bool result2 = Unreference( *catalog, messages );
  return result1 && result2;
}

// ============================================================================
