#include <Gaudi/Examples/Conditions/ConditionAccessorHolder.h>
#include <GaudiKernel/System.h>

namespace Gaudi::Examples::Conditions::details {
  ConditionsStore& ConditionsStore::instance() {
    static ConditionsStore store{};
    return store;
  }

  void ConditionsStore::declareKey( const ConditionKey& key ) {
    if ( std::get<1>( m_requestedKeys.emplace( key ) ) ) {
      std::cout << "ConditionsStore: requested condition " << key << '\n';
    }
  }

  const ConditionsStore::ConditionContext& ConditionsStore::getConditionContext( const EventContext& ctx ) const {
    static ConditionContext current_iov = 0;

    current_iov = ctx.evt() % 2;
    return current_iov;
  }

  const std::any* ConditionsStore::getConditionAny( const ConditionKey& key, const ConditionContext& ctx ) const {
    std::lock_guard<std::mutex> lg{m_storageMtx};
    if ( m_storage.find( ctx ) == end( m_storage ) ) {
      std::cout << "ConditionsStore: first access to store " << ctx << '\n';
      auto& store = m_storage[ctx];
      for ( const auto& key : m_requestedKeys ) {
        std::cout << "ConditionsStore: prepare condition " << key << '\n';
        store.emplace( key, ctx );
      }
    } else {
      std::cout << "ConditionsStore: using existing store " << ctx << '\n';
    }
    return &m_storage[ctx][key];
  }
} // namespace Gaudi::Examples::Conditions::details
