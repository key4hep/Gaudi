#ifndef GAUDIKERNEL_DATAHANDLE
#define GAUDIKERNEL_DATAHANDLE 1

#include "GaudiKernel/DataHandleConfigurable.h"
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Property.h"
#include <string>
#include <type_traits>

//---------------------------------------------------------------------------

/** DataHandle.h  GaudiKernel/DataHandle.h
 *
 * Base class for Handles to access data in Event Store, shared between
 * Gaudi and Atlas
 *
 * Objects are identified via a DataObjID
 *
 * Once object is created, Mode is not modifiable
 *
 * @author Charles Leggett
 * @date   2015-09-01
 */

//---------------------------------------------------------------------------

class IDataHandleHolder;

namespace Gaudi
{

  class DataHandle
  {
  public:
    enum Mode { Reader = 1 << 2, Writer = 1 << 4, Updater = Reader | Writer };

    DataHandle( const DataObjID& k, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( k ), m_owner( owner ), m_mode( a ){};

    DataHandle( const DataObjID& k, const bool& isCond, Mode a = Reader, IDataHandleHolder* owner = nullptr )
        : m_key( k ), m_owner( owner ), m_mode( a ), m_isCond( isCond ){};

    virtual ~DataHandle() = default;

    virtual void setOwner( IDataHandleHolder* o ) { m_owner = o; }
    virtual IDataHandleHolder*                owner() const { return m_owner; }

    virtual Mode mode() const { return m_mode; }

    virtual void setKey( const DataObjID& key ) const { m_key = key; }
    virtual void updateKey( const std::string& key ) const { m_key.updateKey( key ); }

    virtual const std::string& objKey() const { return m_key.key(); }
    virtual const DataObjID&   fullKey() const { return m_key; }

    virtual void reset( bool ){};

    virtual std::string pythonRepr() const;
    virtual bool        init() { return true; }

    // is this a ConditionHandle?
    virtual bool isCondition() const { return m_isCond; }

  protected:
    /**
     * The key of the object behind this DataHandle
     * Although it may look strange to have it mutable, this can actually
     * change in case the object had alternative names, and it should not
     * be visible to the end user, for which the Handle is still the same
     */
    mutable DataObjID  m_key   = {"NONE"};
    IDataHandleHolder* m_owner = nullptr;

  private:
    Mode m_mode   = Reader;
    bool m_isCond = false;
  };

  /// Work-in-progress rewrite of the DataHandle infrastructure
  namespace experimental
  {
    /// Base class to all new-style data handles
    class DataHandle
    {
    public:
      // DataHandles are used for the purpose of accounting component data
      // dependencies, and should thus never be copied.
      DataHandle( const DataHandle& ) = delete;
      DataHandle& operator=( const DataHandle& ) = delete;

      // DataHandles pass a pointer to themselves to the host Algorithm during
      // registration, which happens at construction time. They should thus
      // not be moved, as that would invalidate said pointer.
      //
      // The only reason why we don't delete the DataHandle move constructor
      // as well is that it would also forbid some safe and useful forms of
      // in-place construction.
      //
      DataHandle( DataHandle&& ) = default;
      DataHandle& operator=( DataHandle&& ) = delete;

      /// (Configurable) ID of the data being accessed via this handle
      ///
      /// The current proposal is to only support one ID per handle. If
      /// experiments need to read from multiple whiteboard keys (LHCb
      /// alternate paths) or to write an object that's accessible via
      /// multiple whiteboard keys (ATLAS), they would need to implement the
      /// underlying support on their own.
      ///
      /// For LHCb alternate paths, this can be handled at the DataLoader
      /// level (try linking from alternate paths if original one is empty),
      /// or via good old DataOnDemand for the sequential case. See
      /// https://gitlab.cern.ch/gaudi/Gaudi/merge_requests/422 .
      ///
      /// FIXME: For aliased writes, how would that work with the Scheduler?
      ///
      const DataObjID& targetKey() const { return m_property.targetKey(); }

      /// Change the ID of the target data
      void setTargetKey( const DataObjID& id ) { m_property.setTargetKey( id ); }

      /// Initialize the data handle
      ///
      /// This must be done by the owner once it has sysInitialized itself.
      /// It is the stage at which the handle performs deferred framework
      /// operations such as acquiring access to the data stores.
      ///
      virtual void initialize( const IDataHandleHolder& owner ) = 0;

      /// Truth that owner setup has been performed
      ///
      /// Subclasses of DataHandle which allow for lazy owner initialization are
      /// strongly encouraged to check if said initialization has been performed
      /// at the time where a DataHandle is used.
      ///
      bool ownerSetupDone() const { return m_ownerSetupDone; }

      /// Setup the owner of this DataHandle
      ///
      /// You should only perform this operation when you used the owner-less
      /// constructor. And you should only perform it only once. Failures to
      /// do so will be detected and result in a run-time error.
      ///
      template <typename Owner, std::enable_if_t<std::is_base_of<IDataHandleHolder, Owner>::value>* = nullptr>
      void setOwner( Owner& owner )
      {
        if ( m_ownerSetupDone ) {
          throw std::runtime_error( "Attempted to set DataHandle owner twice" );
        }
        m_property.setOwner( &owner );
        m_ownerSetupDone = true;
      }

    protected:
      /// Handles are constructed like a Gaudi property (and effectively
      /// behave as one, which sets the associated data object identifier)
      template <typename Owner, typename T                                          = DataObjID,
                std::enable_if_t<std::is_base_of<IDataHandleHolder, Owner>::value>* = nullptr>
      DataHandle( Owner& owner, const std::string& propertyName, T&& defaultID, const std::string& docString,
                  const IDataHandleMetadata& metadata )
          : m_property{&owner, propertyName, {metadata, std::forward<T>( defaultID )}, docString}
          , m_ownerSetupDone{true}
      {
      }

      /// The above constructor is easier to use correctly, and therefore
      /// preferred. But some legacy use cases may require setting a
      /// DataHandle's owner lazily after construction. In that case, this
      /// constructor must be used.
      ///
      /// If you opt for this lazy construction method, then you _must_ set the
      /// owner Alg or AlgTool before said owner's initialize() procedure has
      /// finished. See setOwnerImpl() for more information on this.
      ///
      template <typename T = DataObjID>
      explicit DataHandle( const std::string& propertyName, T&& defaultID, const std::string& docString,
                           const IDataHandleMetadata& metadata )
          : m_property{propertyName, {metadata, std::forward<T>( defaultID )}, docString}, m_ownerSetupDone{false}
      {
      }

    private:
      /// Configurable property associated with a DataHandle
      Gaudi::Property<DataHandleConfigurable> m_property;

      /// Truth that owner setup has been performed
      bool m_ownerSetupDone;
    };
  }
}

#endif
