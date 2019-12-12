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
#pragma once

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/Property.h>
#include <GaudiKernel/StateMachine.h>
#include <GaudiKernel/StatusCode.h>
#include <any>
#include <mutex>
#include <string>

namespace Gaudi::Examples::Conditions {
  namespace details {
    /// Example class to implement an actual backend for Gaudi::Examples::Conditions::ConditionAccessorHolder
    class ConditionsStore {
    public:
      using ConditionContext = int;
      using ConditionKey     = std::string;

      static ConditionsStore& instance();

      void declareKey( const ConditionKey& key );

      const ConditionContext& getConditionContext( const EventContext& ctx ) const;

      template <typename T>
      const T& getCondition( const ConditionKey& key, const ConditionContext& ctx ) const {
        return *std::any_cast<T>( getConditionAny( key, ctx ) );
      }

    private:
      ConditionsStore() = default;
      const std::any* getConditionAny( const ConditionKey& key, const ConditionContext& ctx ) const;

      using IOVSlot  = std::map<ConditionKey, std::any>;
      using IOVSlots = std::map<ConditionContext, IOVSlot>;

      mutable std::mutex     m_storageMtx;
      mutable IOVSlots       m_storage;
      std::set<ConditionKey> m_requestedKeys;
    };
  } // namespace details

  /// Example of a ConditionAccessorHolder implementation.
  ///
  /// Requirements are:
  /// - inherit from the template argument (forwarding the constructor)
  /// - provide the following types (could be type aliases):
  ///   - `base_class` (this type)
  ///   - `ConditionAccessor<T>`
  ///   - `ConditionContext`
  ///   - `ConditionKey`
  /// - provide the method
  ///   - `const ConditionContext& getConditionContext( const EventContext& ) const`
  /// - provide a method to handle registration of ConditionAccessor instances for
  ///   internal bookkeeping (here represented by `void registerConditionAccessor<T>( ConditionAccessor<T>& accessor )`)
  ///
  /// Component classes that require conditions have to inherit from this class,
  /// templated with the component base class. E.g.
  /// \code
  /// class MyConditionAlgorithm final : ConditionAccessorHolder<Gaudi::Algorithm> {
  ///   // ...
  /// };
  /// \endcode
  ///
  /// See Gaudi::Examples::Conditions::UserAlg for example.
  template <typename Base>
  class ConditionAccessorHolder : public Base {
  public:
    /// Forward to base class constructor
    using Base::Base;

    /// Helper to expose this class to specializations without having to spell
    /// the whole name (see Gaudi::Examples::Conditions::UserAlg)
    using base_class = ConditionAccessorHolder<Base>;

    /// Forward backend specific ConditionContext to specializations.
    ///
    /// It is used to refer to a specific conditions slot.
    using ConditionContext = details::ConditionsStore::ConditionContext;

    /// Forward backend specific ConditionKey to specializations.
    ///
    /// Used to identify a specific condition in a condition slot.
    using ConditionKey = details::ConditionsStore::ConditionKey;

    /// Class wrapping the communication between condition users and conditions backend.
    ///
    /// Users must use instances of this class as data members, passing `this` to the
    /// data member constructor.
    ///
    /// See Gaudi::Examples::Conditions::UserAlg::m_cond
    template <typename T>
    class ConditionAccessor {
    public:
      /// Constructor a ConditionAccessor instance registering it to the current holder.
      ConditionAccessor( base_class* owner, const std::string& keyName, const ConditionKey& keyDefault,
                         const std::string& keyDoc = "" )
          : m_key{owner, keyName, keyDefault, keyDoc} {
        owner->registerConditionAccessor( *this );
      }

      /// Helper to access the key of the current accessor.
      ///
      /// @note can only be modified via the associated property
      const ConditionKey& key() const { return m_key; }

      /// Get the condition value for the requested condition slot.
      const T& get( const ConditionContext& ctx ) const {
        return details::ConditionsStore::instance().getCondition<T>( key(), ctx );
      }

    private:
      /// Internal property object for the condition key.
      Gaudi::Property<ConditionKey> m_key;
    };

    StatusCode initialize() override {
      auto sc = Base::initialize();
      if ( !sc ) return sc;

      // backend-specific initialization

      // trigger delayed registrations
      for ( auto& action : m_delayedRegistrations ) { action(); }
      m_delayedRegistrations.clear();

      return sc;
    }

    /// Helper to map the event being processed to the corresponding conditions slot.
    const ConditionContext& getConditionContext( const EventContext& ctx ) const {
      return details::ConditionsStore::instance().getConditionContext( ctx );
    }

  private:
    template <typename T>
    friend class ConditionAccessor;

    /// Register a ConditionAccessor instance to internal bookkeeping.
    template <typename T>
    void registerConditionAccessor( ConditionAccessor<T>& accessor ) {
      if ( this->targetFSMState() == Gaudi::StateMachine::INITIALIZED ) {
        details::ConditionsStore::instance().declareKey( accessor.key() );
      } else {
        m_delayedRegistrations.push_back( [this, &accessor]() { registerConditionAccessor( accessor ); } );
      }
    }

    /// Helper to postpone the registration to the backend.
    ///
    /// In Gaudi properties are set during initialize, so, in this example implementation,
    /// the registration has to be delayed from construction time to initialization time.
    std::list<std::function<void()>> m_delayedRegistrations;
  };
} // namespace Gaudi::Examples::Conditions
