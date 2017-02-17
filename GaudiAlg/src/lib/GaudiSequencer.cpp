// Include files

// from Gaudi
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/ISequencerTimerTool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IJobOptionsSvc.h"

namespace
{

  // TODO: this  adds C++14 'make_unique'... remove once we move to C++14...
  template <typename T, typename... Args>
  std::unique_ptr<T> make_unique_( Args&&... args )
  {
    return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
  }

  bool isDefault(const std::string& s) { return s.empty(); }

  // utility class to populate some properties in the job options service
  // for a given instance name in case those options are not explicitly
  // set a-priori (effectively inheriting their values from the GaudiSequencer)
  class populate_JobOptionsSvc_t
  {
    std::vector<std::unique_ptr<Gaudi::Details::PropertyBase>> m_props;
    IJobOptionsSvc* m_jos;
    std::string m_name;

    template <typename T>
    void process( T&& t )
    {
      static_assert( std::tuple_size<T>::value == 2, "Expecting an std::tuple key-value pair" );
      using type   = typename std::decay<typename std::tuple_element<1, T>::type>::type;
      using prop_t = Gaudi::Property<type>;
      if ( !isDefault( std::get<1>( t ) ) )
        m_props.push_back( make_unique_<prop_t>( std::get<0>( t ), std::get<1>( t ) ) );
    }
    template <typename T, typename... Args>
    void process( T&& t, Args&&... args )
    {
      process( std::forward<T>( t ) );
      process( std::forward<Args>( args )... );
    }
    void check_veto()
    { // avoid changing properties expliclty present in the JOS...
      const auto* props = m_jos->getProperties( m_name );
      if ( !props ) return;
      for ( const auto& i : *props ) {
        auto j = std::find_if(
            std::begin( m_props ), std::end( m_props ),
            [&i]( const std::unique_ptr<Gaudi::Details::PropertyBase>& prop ) { return prop->name() == i->name(); } );
        if ( j == std::end( m_props ) ) continue;
        m_props.erase( j );
        if ( m_props.empty() ) break; // done!
      }
    }

  public:
    template <typename... Args>
    populate_JobOptionsSvc_t( std::string name, IJobOptionsSvc* jos, Args&&... args )
        : m_jos{jos}, m_name{std::move( name )}
    {
      process( std::forward<Args>( args )... );
      if ( !m_props.empty() ) check_veto();
      std::for_each( std::begin( m_props ), std::end( m_props ),
                     [&]( const std::unique_ptr<Gaudi::Details::PropertyBase>& i ) {
                       m_jos->addPropertyToCatalogue( m_name, *i ).ignore();
                     } );
    }
    ~populate_JobOptionsSvc_t()
    {
      std::for_each( std::begin( m_props ), std::end( m_props ),
                     [&]( const std::unique_ptr<Gaudi::Details::PropertyBase>& i ) {
                       m_jos->removePropertyFromCatalogue( m_name, i->name() ).ignore();
                     } );
    }
  };

  template <typename Stream, typename Container, typename Separator, typename Transform>
  Stream& ostream_joiner( Stream& os, const Container& c, Separator sep, Transform trans )
  {
    auto first = std::begin( c );
    auto last  = std::end( c );
    if ( first != last ) {
      os << trans( *first );
      ++first;
    }
    for ( ; first != last; ++first ) os << sep << trans( *first );
    return os;
  }
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
    if ( ! myAlg->isEnabled() ) continue;
    if ( ! myAlg->isExecuted() ) {

      if ( m_measureTime ) m_timerTool->start( entry.timer() );
      result = myAlg->sysExecute(getContext());
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
      StatusCode sc = myIAlg->sysInitialize();
      if ( sc.isFailure() ) {
        result = sc;
      }
    }

    // propagate the sub-algorithm into own state.
    if ( result.isSuccess() && Gaudi::StateMachine::RUNNING <= FSMState() && myIAlg &&
         Gaudi::StateMachine::RUNNING > myIAlg->FSMState() ) {
      StatusCode sc = myIAlg->sysStart();
      if ( sc.isFailure() ) {
        result = sc;
      }
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
  ostream_joiner( msg, m_entries, ", ", []( const AlgorithmEntry& e ) {
    Algorithm* alg  = e.algorithm();
    std::string typ = System::typeinfoName( typeid( *alg ) );
    return ( alg->name() == typ ) ? alg->name() : ( typ + "/" + alg->name() );
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
  if ( Gaudi::StateMachine::INITIALIZED > FSMState() ) {
    return;
  } // RETURN

  decodeNames().ignore();

  if ( !m_measureTime ) {
    return;
  } // RETURN

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

std::ostream& GaudiSequencer::toControlFlowExpression(std::ostream& os) const {
  if (m_invert) os << "~";
  // the default filterpass value for an empty sequencer depends on ModeOR
  if (m_entries.empty()) return os << ((!m_modeOR) ? "CFTrue" : "CFFalse");

  // if we have only one element, we do not need a name
  if (m_entries.size() > 1) os << "seq(";

  const auto op = m_modeOR ? " | " : " & ";
  const auto first = begin(m_entries);
  const auto last = end(m_entries);
  auto iterator = first;
  while (iterator != last) {
    if (iterator != first) os << op;
    if (iterator->reverse()) os << "~";
    iterator->algorithm()->toControlFlowExpression(os);
    ++iterator;
  }

  if (m_entries.size() > 1) os << ")";
  return os;
}
//=============================================================================
