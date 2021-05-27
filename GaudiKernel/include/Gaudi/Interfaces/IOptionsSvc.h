/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/TaggedBool.h>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace Gaudi {
  namespace Details {
    class PropertyBase;
  }
  namespace Interfaces {
    /** Interface for a component that manages application configuration options.

    Options are to be recorded as pairs of strings: option id and value.
    The value must be the string representation of the a value, as understood by
    `Gaudi::Property<T>::fromString()`.

    For example, if an Algorithm defines two properties as in
    \code
    class MyAlg: public Algorithm {
      Gaudi::Property<std::string> m_hname{this, "HistoName", "none"};
      Gaudi::Property<float> m_threshold{this, "Threshold", 1.0};
    };
    \endcode

    The IOptionsSvc instance should be populated with something like
    \code
      auto& optsSvc = sericeLocator()->getOptsSvc();
      optsSvc.set("AlgInstanceName.HistoName", "'my_histogram'");
      optsSvc.set("AlgInstanceName.Threshold", "2.345");
    \endcode
    */
    struct IOptionsSvc : virtual public IInterface {
      /// InterfaceID declaration.
      DeclareInterfaceID( IOptionsSvc, 1, 0 );

      /// Set the value of an option, overriding the old value, if any.
      virtual void set( const std::string& key, const std::string& value ) = 0;
      /// Get the value of an options, returning the specified _default_ value if not found.
      virtual std::string get( const std::string& key, const std::string& default_ = {} ) const = 0;
      /// Get the value of an options, removing it from the storage, returning the specified _default_ value if not
      /// found.
      virtual std::string pop( const std::string& key, const std::string& default_ = {} ) = 0;
      /// %Test if an option key is available in the catalog.
      virtual bool has( const std::string& key ) const = 0;
      /// %Test if an option key was explicitly set or not.
      virtual bool isSet( const std::string& key ) const = 0;

      /// @name Methods to query options.
      /// These methods allow to extract the list of know options with their values.
      /// @{
      /** Return all known options with their values. */
      virtual std::vector<std::tuple<std::string, std::string>> items() const = 0;
      /** Return all known options with their values for which `predicate` evaluates to true. */
      template <class UnaryPredicate>
      std::vector<std::tuple<std::string, std::string>> items( UnaryPredicate predicate ) const {
        auto v = this->items();
        v.erase( std::remove_if( begin( v ), end( v ),
                                 [&predicate]( const auto& element ) { return !predicate( element ); } ),
                 v.end() );
        return v;
      }
      /** Return all known options with their values for which the key matches the given regular expression. */
      std::vector<std::tuple<std::string, std::string>> items( const std::regex& filter ) const {
        std::smatch match;
        return items(
            [&filter, &match]( const auto& element ) { return regex_match( std::get<0>( element ), match, filter ); } );
      }
      /// @}

      /// Register a Gaudi::Property instance to the option service.
      ///
      /// The option will be bound to the property with name `<prefix>.<property-name>`, and immediately
      /// assigned the value set in the options service, if any.
      ///
      /// After the binding, setting a property value via IOptionsSvc::set will automatically set the value
      /// of the Gaudi::Property instance, and IOptionsSvc::get will return the string representation of the
      /// value of the Gaudi::Property instance.
      virtual void bind( const std::string& prefix, Gaudi::Details::PropertyBase* property ) = 0;

      using OnlyDefaults = Gaudi::tagged_bool<class OnlyDefaults_tag>;

      /// Broadcast version of IOptionsSvc::set.
      ///
      /// With IOptionsSvc::broadcast it is possible to assign one value to all known properties matching a
      /// regular expression.  By default, only default values are overridden, but explicitly assigned values
      /// can also be overridden passing `OnlyDefaults{false}` as third argument to the method.
      virtual void broadcast( const std::regex& filter, const std::string& value,
                              OnlyDefaults defaults = OnlyDefaults{true} ) = 0;

      /// look for file 'file' into search path 'path'
      /// and read it to update the options
      virtual StatusCode readOptions( const std::string& file, const std::string& path = "" ) = 0;

    protected:
      virtual ~IOptionsSvc() = default;
    };
  } // namespace Interfaces
} // namespace Gaudi
