/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Auditor.h>
#include <GaudiKernel/IHiveWhiteBoard.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/StatusCode.h>

#include <nvtx3/nvToolsExt.h>
#include <tbb/concurrent_hash_map.h>

#include <cstdint>
#include <format>
#include <string>

namespace {
  /// @brief Generate a color based on the given index.
  /// @param i The index to generate a color for.
  /// @return A 32-bit ARGB color value.
  /// @author Eric Cano
  static uint32_t cycle_color( uint64_t i ) {
    constexpr uint32_t alpha = 0xFFu << 24;
    constexpr uint8_t  cernR = 0;
    constexpr uint8_t  cernG = 51;
    constexpr uint8_t  cernB = 160;
    auto               r     = static_cast<uint8_t>( ( i * 23 ) + cernR );
    auto               g     = static_cast<uint8_t>( ( i * 47 ) + cernG );
    auto               b     = static_cast<uint8_t>( ( i * 71 ) + cernB );
    return alpha | ( static_cast<uint32_t>( r ) << 16 ) | ( static_cast<uint32_t>( g ) << 8 ) |
           static_cast<uint32_t>( b );
  }
} // namespace

/**
 * @class NVTXAuditor
 * @brief Auditor that uses NVTX to annotate ranges and incidents for profiling with NVIDIA tools.
 *
 * This auditor creates NVTX ranges around audited events and marks incidents with NVTX markers.
 * The ranges and markers are created in a custom "Gaudi" NVTX domain.
 * Ranges and markers related to events are annotated with event number in the payload, colored based on an event number
 * and put in categories based on event slot. Ranges and markers not related to events are put into the default category
 * with no payload and default color.
 * @author Mateusz Jakub Fila
 */
class NVTXAuditor : public extends<Gaudi::Auditor, IIncidentListener> {
public:
  using base_class::base_class;

private:
  // Domain used for all NVTX events emitted by this auditor. Written once during initialization and then only read.
  static inline nvtxDomainHandle_t s_domain{ nullptr };

  // Number of categories created (one per event slot). Written once during initialization and then only read; also
  // used to guard against double initialization.
  static inline uint32_t s_numCategories{ 0 };

  // The auditor uses "process ranges" and stores handles to them for in-flight events. "process ranges" are used
  // instead "thread ranges" because they can be started and stopped on different threads which might be not always
  // guaranteed with suspension.
  tbb::concurrent_hash_map<std::string, nvtxRangeId_t> m_ranges;

  /**
   * @brief Initialize the auditor and register for all available incidents if the Incident service exists.
   * @return StatusCode indicating success or failure.
   *
   * @note Creates the NVTX domain and categories, which must be done only once.
   */
  StatusCode initialize() override {

    return base_class::initialize()
        .andThen( [this]() {
          auto incidentSvc = service<IIncidentSvc>( "IncidentSvc" );
          if ( incidentSvc.isValid() ) {
            incidentSvc->addListener( this, "ALL" );
          } else {
            info() << "No IncidentSvc, NVTXAuditor will not be able to annotate incidents." << endmsg;
          }
        } )
        .andThen( [this]() {
          if ( s_numCategories > 0 ) {
            error() << "Attempting to initialize NVTX categories more than once." << endmsg;
            return StatusCode::FAILURE;
          }

          s_domain = nvtxDomainCreateA( "Gaudi" );

          auto hiveWhiteBoard = service<IHiveWhiteBoard>( "EventDataSvc" );
          if ( hiveWhiteBoard.isValid() ) {
            /// Name a category for each slot in the HiveWhiteBoard
            s_numCategories = hiveWhiteBoard->getNumberOfStores();
            for ( auto slot = 0u; slot < s_numCategories; ++slot ) {
              // slot + 1 for 1-indexing of categories in NVTX
              nvtxDomainNameCategoryA( s_domain, slot + 1, std::format( "Slot {}", slot ).c_str() );
            }
          } else {
            /// name a category for a single slot if no HiveWhiteBoard
            s_numCategories = 1;
            nvtxDomainNameCategoryA( s_domain, 1, "Slot 0" );
          }
          return StatusCode::SUCCESS;
        } );
  }

  /**
   * @brief Finalize the auditor, releasing the NVTX domain.
   * @return StatusCode indicating success or failure.
   */
  StatusCode finalize() override {
    if ( s_domain ) {
      nvtxDomainDestroy( s_domain );
      s_domain = nullptr;
    }
    return base_class::finalize();
  }

  /**
   * @brief Handle an incident by annotating it with an NVTX marker.
   * @param incident The incident to handle.
   *
   * Annotates the given incident with an NVTX marker. If the incident has a valid event context, the marker is
   * annotated with event number in the payload, colored based on event number and put in category based on event slot.
   * If the incident does not have a valid event context, the marker is created without payload, default color and
   * default category.
   */
  void handle( const Incident& incident ) override {
    auto eventAttrib          = nvtxEventAttributes_t{};
    eventAttrib.version       = NVTX_VERSION;
    eventAttrib.size          = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
    eventAttrib.messageType   = NVTX_MESSAGE_TYPE_ASCII;
    eventAttrib.message.ascii = incident.type().c_str();

    if ( !incident.context().valid() ) {
      nvtxDomainMarkEx( s_domain, &eventAttrib );
      info() << std::format( "Marked incident {} from {}", incident.type(), incident.source() ) << endmsg;
      return;
    }

    eventAttrib.colorType        = NVTX_COLOR_ARGB;
    eventAttrib.color            = ::cycle_color( incident.context().evt() );
    eventAttrib.category         = incident.context().slot() + 1;
    eventAttrib.payloadType      = NVTX_PAYLOAD_TYPE_UNSIGNED_INT64;
    eventAttrib.payload.ullValue = incident.context().evt();

    nvtxDomainMarkEx( s_domain, &eventAttrib );
    info() << std::format( "Marked incident {} from {} with context {}", incident.type(), incident.source(),
                           incident.context() )
           << endmsg;
  }

  /**
   * @brief Helper to create an identifier key for storing an associated range in the map.
   * @param event The name of the audited event.
   * @param caller The name of the caller of the audited event.
   * @param ctx The event context associated with the audited event.
   * @return A key for the event.
   */
  std::string make_key( std::string const& event, std::string const& caller, EventContext const& ctx ) const {
    if ( ctx.valid() ) { return std::format( "{}:{}:{}", caller, event, ctx.evt() ); }
    return std::format( "{}:{}", caller, event );
  }

  /**
   * @brief Start an NVTX range for the audited event.
   * @param label The label to attach to the range.
   * @param ctx The event context associated with the audited event.
   * @return The opaque handle for the started NVTX range.
   *
   * Starts an NVTX range for the audited event. If the context is valid, the range is annotated with event
   * number in the payload, colored based on event number and put in category based on event slot. If the context is
   * not valid, the range is created without payload, default color and default category.
   */
  nvtxRangeId_t start_range( std::string const& label, EventContext const& ctx ) {
    auto eventAttrib          = nvtxEventAttributes_t{};
    eventAttrib.version       = NVTX_VERSION;
    eventAttrib.size          = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
    eventAttrib.messageType   = NVTX_MESSAGE_TYPE_ASCII;
    eventAttrib.message.ascii = label.c_str();

    if ( !ctx.valid() ) {
      debug() << std::format( "Starting range for {}", label ) << endmsg;
      return nvtxDomainRangeStartEx( s_domain, &eventAttrib );
    }

    eventAttrib.colorType        = NVTX_COLOR_ARGB;
    eventAttrib.color            = ::cycle_color( ctx.evt() );
    eventAttrib.category         = ctx.slot() + 1;
    eventAttrib.payloadType      = NVTX_PAYLOAD_TYPE_UNSIGNED_INT64;
    eventAttrib.payload.ullValue = ctx.evt();

    debug() << std::format( "Starting range for {} with context {}", label, ctx ) << endmsg;
    return nvtxDomainRangeStartEx( s_domain, &eventAttrib );
  }

  /**
   *  @brief Handle 'before' audit for given event by starting an associated NVTX range.
   *  @param event The name of the audited event.
   *  @param caller The name of the caller of the audited event.
   *  @param ctx The EventContext associated with the audited event.
   *
   *  @note In case of a suspension event, the range for the execution is closed instead of creating a range for
   * suspension.
   *
   */
  void before( std::string const& event, std::string const& caller, EventContext const& ctx ) override {
    // Suspension pauses the execution, so close the currently active
    // execution range instead of creating a range for the suspension.
    if ( IAuditor::Suspension == event ) {
      auto execute_key = make_key( IAuditor::Execute, caller, ctx );
      auto acc         = decltype( m_ranges )::accessor{};
      if ( !m_ranges.find( acc, execute_key ) ) {
        error() << std::format( "Execution range for {} does not exist while suspending", execute_key ) << endmsg;
      } else {
        nvtxDomainRangeEnd( s_domain, acc->second );
        m_ranges.erase( acc );
        info() << std::format( "Suspended execution range for {}", execute_key ) << endmsg;
      }
      return;
    }

    auto key   = make_key( event, caller, ctx );
    auto label = std::format( "{}:{}", caller, event );
    auto id    = start_range( label, ctx );
    auto ret   = m_ranges.emplace( key, id );
    if ( !ret ) {
      error() << std::format( "Range for {} already exists", key ) << endmsg;
      nvtxDomainRangeEnd( s_domain, id ); // the newly started range would otherwise never be closed
    } else {
      info() << std::format( "Started range for {}", key ) << endmsg;
    }
  }

  /**
   *  @brief Handle 'after' audit for given event by ending the associated NVTX range.
   *  @param event The name of the audited event.
   *  @param caller The name of the caller of the audited event.
   *  @param ctx The EventContext associated with the audited event.
   *  @param status The status code of the audited event.
   *
   * @note In case of a suspension event, the range for the execution is created to indicate resumption rather than
   * creating a range for suspension.
   *
   */
  void after( std::string const& event, std::string const& caller, EventContext const& ctx,
              StatusCode const& = StatusCode::SUCCESS ) override {
    // Suspension has ended, so resume the execution by creating a new
    // execution range. Do not create a range for Suspend itself.
    if ( IAuditor::Suspension == event ) {
      auto execute_key = make_key( IAuditor::Execute, caller, ctx );
      auto label       = std::format( "{}:{}", caller, IAuditor::Execute );
      auto id          = start_range( label, ctx );
      auto ret         = m_ranges.emplace( execute_key, id );
      if ( !ret ) {
        error() << std::format( "Execution range for {} already exists while resuming", execute_key ) << endmsg;
        nvtxDomainRangeEnd( s_domain, id ); // the newly started range would otherwise never be closed
      } else {
        info() << std::format( "Resumed execution range for {}", execute_key ) << endmsg;
      }
      return;
    }

    auto key = make_key( event, caller, ctx );
    auto acc = decltype( m_ranges )::accessor{};
    if ( m_ranges.find( acc, key ) ) {
      nvtxDomainRangeEnd( s_domain, acc->second );
      m_ranges.erase( acc );
    }
    info() << std::format( "Finished range for {}", key ) << endmsg;
  }
};

DECLARE_COMPONENT( NVTXAuditor )
