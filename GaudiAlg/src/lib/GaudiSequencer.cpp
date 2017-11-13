// Include files
#include <initializer_list>
#include <tuple>

// from Gaudi
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/ISequencerTimerTool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IJobOptionsSvc.h"

namespace
{

  bool isDefault( const std::string& s ) { return s.empty(); }

  template <typename Container>
  bool veto( const Container* props, const char* name )
  { // avoid changing properties explicitly present in the JOS...
    return props &&
           std::any_of( begin( *props ), end( *props ), [name]( const auto* prop ) { return prop->name() == name; } );
  }

  template <typename F, typename... Args>
  void for_each_arg( F&& f, Args&&... args )
  {
    // the std::initializer_list only exists to provide a 'context' in which to
    // expand the variadic pack
    (void)std::initializer_list<int>{( f( std::forward<Args>( args ) ), 0 )...};
  }

  // utility class to populate some properties in the job options service
  // for a given instance name in case those options are not explicitly
  // set a-priori (effectively inheriting their values from the GaudiSequencer)
  class populate_JobOptionsSvc_t
  {
    std::vector<std::string> m_props;
    IJobOptionsSvc* m_jos;
    std::string m_name;

    template <typename Properties, typename Key, typename Value>
    void addPropertyToCatalogue( const Properties* props, const std::tuple<Key, Value>& arg )
    {
      const auto& key   = std::get<0>( arg );
      const auto& value = std::get<1>( arg );
      if ( isDefault( value ) || veto( props, key ) ) return;
      m_jos->addPropertyToCatalogue( m_name, Gaudi::Property<std::decay_t<Value>>{key, value} ).ignore();
      m_props.push_back( key );
    }

  public:
    template <typename... Args>
    populate_JobOptionsSvc_t( std::string name, IJobOptionsSvc* jos, Args&&... args )
        : m_jos{jos}, m_name{std::move( name )}
    {
      const auto* props = m_jos->getProperties( m_name );
      for_each_arg( [&]( auto&& arg ) { this->addPropertyToCatalogue( props, std::forward<decltype( arg )>( arg ) ); },
                    std::forward<Args>( args )... );
    }
    ~populate_JobOptionsSvc_t()
    {
      std::for_each( begin( m_props ), end( m_props ),
                     [&]( const std::string& key ) { m_jos->removePropertyFromCatalogue( m_name, key ).ignore(); } );
    }
  };
}

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiSequencer
//
// 2004-05-13 : Olivier Callot
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GaudiSequencer::GaudiSequencer( const std::string& name, ISvcLocator* pSvcLocator )
    : GaudiAlgorithm( name, pSvcLocator )
{
  m_names.declareUpdateHandler( &GaudiSequencer::membershipHandler, this );
}
//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode GaudiSequencer::initialize()
{
  GaudiAlgorithm::initialize();

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialise" << endmsg;

  StatusCode status = decodeNames();
  if ( !status.isSuccess() ) return status;

  m_timerTool                                      = tool<ISequencerTimerTool>( "SequencerTimerTool" );
  if ( m_timerTool->globalTiming() ) m_measureTime = true;

  if ( m_measureTime ) {
    m_timer = m_timerTool->addTimer( name() );
    m_timerTool->increaseIndent();
  } else {
    release( m_timerTool );
    m_timerTool = nullptr;
  }

  //== Initialize the algorithms
  for ( auto& entry : m_entries ) {
    if ( m_measureTime ) {
      entry.setTimer( m_timerTool->addTimer( entry.algorithm()->name() ) );
    }

    status = entry.algorithm()->sysInitialize();
    if ( !status.isSuccess() ) {
      return Error( "Can not initialize " + entry.algorithm()->name(), status );
    }
  }
  if ( m_measureTime ) m_timerTool->decreaseIndent();

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode GaudiSequencer::execute()
{

  if ( m_measureTime ) m_timerTool->start( m_timer );

  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

  StatusCode result = StatusCode( StatusCode::SUCCESS, true );

  bool seqPass = !m_modeOR; //  for OR, result will be false, unless (at least) one is true
                            //  for AND, result will be true, unless (at least) one is false
                            //    also see comment below ....

  for ( auto& entry : m_entries ) {
    Algorithm* myAlg = entry.algorithm();
    if ( !myAlg->isEnabled() ) continue;
    if ( !myAlg->isExecuted() ) {

      if ( m_measureTime ) m_timerTool->start( entry.timer() );
      result = myAlg->sysExecute( getContext() );
      if ( m_measureTime ) m_timerTool->stop( entry.timer() );
      myAlg->setExecuted( true );
      if ( !result.isSuccess() ) break; //== Abort and return bad status
    }
    //== Check the returned status
    if ( !m_ignoreFilter ) {
      bool passed = myAlg->filterPassed();
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << "Algorithm " << myAlg->name() << " returned filter passed " << ( passed ? "true" : "false" )
                  << endmsg;
      if ( entry.reverse() ) passed = !passed;

      //== indicate our own result. For OR, exit as soon as true.
      //        If no more, will exit with false.
      //== for AND, exit as soon as false. Else, will be true (default)

      // if not short-circuiting, make sure we latch iPass to 'true' in
      // OR mode (i.e. it is sufficient for one item to be true in order
      // to be true at the end, and thus we start out at 'false'), and latch
      // to 'false' in AND mode (i.e. it is sufficient for one item to
      // be false to the false in the end, and thus we start out at 'true')
      // -- i.e. we should not just blindly return the 'last' passed status!

      // or to put it another way: in OR mode, we don't care about things
      // which are false, as they leave our current state alone (provided
      // we stared as 'false'!), and in AND mode, we keep our current
      // state until someone returns 'false' (provided we started as 'true')
      if ( m_modeOR ? passed : !passed ) {
        seqPass = passed;
        if ( msgLevel( MSG::VERBOSE ) ) verbose() << "SeqPass is now " << ( seqPass ? "true" : "false" ) << endmsg;
        if ( m_shortCircuit ) break;
      }
    }
  }
  if ( msgLevel( MSG::VERBOSE ) ) verbose() << "SeqPass is " << ( seqPass ? "true" : "false" ) << endmsg;
  if ( !m_ignoreFilter && !m_entries.empty() ) setFilterPassed( m_invert ? !seqPass : seqPass );
  setExecuted( true );

  if ( m_measureTime ) m_timerTool->stop( m_timer );

  return m_returnOK ? ( result.ignore(), StatusCode::SUCCESS ) : result;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode GaudiSequencer::finalize()
{
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;
  return GaudiAlgorithm::finalize();
}

//=========================================================================
//  Execute the beginRun of every algorithm
//=========================================================================
StatusCode GaudiSequencer::beginRun()
{
  if ( !isEnabled() ) return StatusCode::SUCCESS;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> beginRun" << endmsg;
  return StatusCode::SUCCESS;
}

//=========================================================================
//  Execute the endRun() of every algorithm
//=========================================================================
StatusCode GaudiSequencer::endRun()
{
  if ( !isEnabled() ) return StatusCode::SUCCESS;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "==> endRun" << endmsg;
  return StatusCode::SUCCESS;
}

//=========================================================================
//  Decode the input names and fills the m_algs vector.
//=========================================================================
StatusCode GaudiSequencer::decodeNames()
{
  StatusCode final = StatusCode::SUCCESS;
  m_entries.clear();

  //== Get the "Context" option if in the file...
  auto jos = service<IJobOptionsSvc>( "JobOptionsSvc" );

  //= Get the Application manager, to see if algorithm exist
  auto appMgr = service<IAlgManager>( "ApplicationMgr" );
  for ( const auto& item : m_names.value() ) {
    const Gaudi::Utils::TypeNameString typeName( item );
    const std::string& theName = typeName.name();
    const std::string& theType = typeName.type();

    //== Check wether the specified algorithm already exists. If not, create it
    StatusCode result          = StatusCode::SUCCESS;
    SmartIF<IAlgorithm> myIAlg = appMgr->algorithm( typeName, false ); // do not create it now
    if ( !myIAlg ) {
      // ensure some magic properties are set while we create the subalgorithm so
      // that it effectively inherites 'our' settings -- if they have non-default
      // values... and are not set explicitly already.
      populate_JobOptionsSvc_t populate_guard{theName, jos, std::forward_as_tuple( "Context", context() ),
                                              std::forward_as_tuple( "RootInTES", rootInTES() )};
      Algorithm* myAlg = nullptr;
      result           = createSubAlgorithm( theType, theName, myAlg );
      myIAlg           = myAlg; // ensure that myIAlg.isValid() from here onwards!
    } else {
      Algorithm* myAlg = dynamic_cast<Algorithm*>( myIAlg.get() );
      if ( myAlg ) {
        subAlgorithms()->push_back( myAlg );
        // when the algorithm is not created, the ref count is short by one, so we have to fix it.
        myAlg->addRef();
      }
    }

    // propagate the sub-algorithm into own state.
    if ( result.isSuccess() && Gaudi::StateMachine::INITIALIZED <= FSMState() && myIAlg &&
         Gaudi::StateMachine::INITIALIZED > myIAlg->FSMState() ) {
      StatusCode sc                = myIAlg->sysInitialize();
      if ( sc.isFailure() ) result = sc;
    }

    // propagate the sub-algorithm into own state.
    if ( result.isSuccess() && Gaudi::StateMachine::RUNNING <= FSMState() && myIAlg &&
         Gaudi::StateMachine::RUNNING > myIAlg->FSMState() ) {
      StatusCode sc                = myIAlg->sysStart();
      if ( sc.isFailure() ) result = sc;
    }

    //== Is it an Algorithm ?  Strange test...
    if ( result.isSuccess() ) {
      // TODO: (MCl) it is possible to avoid the dynamic_cast in most of the
      //             cases by keeping the result of createSubAlgorithm.
      Algorithm* myAlg = dynamic_cast<Algorithm*>( myIAlg.get() );
      if ( myAlg ) {
        // Note: The reference counting is kept by the system of sub-algorithms
        m_entries.emplace_back( myAlg );
        if ( msgLevel( MSG::DEBUG ) ) debug() << "Added algorithm " << theName << endmsg;
      } else {
        warning() << theName << " is not an Algorithm - failed dynamic_cast" << endmsg;
        final = StatusCode::FAILURE;
      }
    } else {
      warning() << "Unable to find or create " << theName << endmsg;
      final = result;
    }
  }

  //== Print the list of algorithms
  MsgStream& msg = info();
  if ( m_modeOR ) msg << "OR ";
  msg << "Member list: ";
  GaudiUtils::details::ostream_joiner( msg, m_entries, ", ",
                                       []( auto& os, const AlgorithmEntry& e ) -> decltype( auto ) {
                                         Algorithm* alg  = e.algorithm();
                                         std::string typ = System::typeinfoName( typeid( *alg ) );
                                         os << typ;
                                         if ( alg->name() != typ ) os << "/" << alg->name();
                                         return os;
                                       } );
  if ( !isDefault( context() ) ) msg << ", with context '" << context() << "'";
  if ( !isDefault( rootInTES() ) ) msg << ", with rootInTES '" << rootInTES() << "'";
  msg << endmsg;

  return final;
}

//=========================================================================
//  Interface for the Property manager
//=========================================================================
void GaudiSequencer::membershipHandler( Gaudi::Details::PropertyBase& /* p */ )
{
  // no action for not-yet initialized sequencer
  if ( Gaudi::StateMachine::INITIALIZED > FSMState() ) return; // RETURN

  decodeNames().ignore();

  if ( !m_measureTime ) return; // RETURN

  // add the entries into timer table:

  if ( !m_timerTool ) {
    m_timerTool = tool<ISequencerTimerTool>( "SequencerTimerTool" );
  }

  if ( m_timerTool->globalTiming() ) m_measureTime = true;

  m_timer = m_timerTool->addTimer( name() );
  m_timerTool->increaseIndent();

  for ( auto& entry : m_entries ) {
    entry.setTimer( m_timerTool->addTimer( entry.algorithm()->name() ) );
  }

  m_timerTool->decreaseIndent();
}

std::ostream& GaudiSequencer::toControlFlowExpression( std::ostream& os ) const
{
  if ( m_invert ) os << "~";
  // the default filterpass value for an empty sequencer depends on ModeOR
  if ( m_entries.empty() ) return os << ( ( !m_modeOR ) ? "CFTrue" : "CFFalse" );

  // if we have only one element, we do not need a name
  if ( m_entries.size() > 1 ) os << "seq(";

  const auto op    = m_modeOR ? " | " : " & ";
  const auto last  = end( m_entries );
  const auto first = begin( m_entries );
  for ( auto iterator = first; iterator != last; ++iterator ) {
    if ( iterator != first ) os << op;
    if ( iterator->reverse() ) os << "~";
    iterator->algorithm()->toControlFlowExpression( os );
  }

  if ( m_entries.size() > 1 ) os << ")";
  return os;
}
//=============================================================================
