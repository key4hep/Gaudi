#ifndef GAUDIALG_GAUDITUPLES_H
#define GAUDIALG_GAUDITUPLES_H 1
// ============================================================================
/* @file GaudiTuples.h
 *
 *  Header file for class : GaudiTuples
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-08-08
 */
// ============================================================================
// Include files
// ============================================================================
#include "boost/algorithm/string/replace.hpp"
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ClassID.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/Maps.h"
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleObj.h"
// ============================================================================
/** @class GaudiTuples GaudiTuples.h GaudiAlg/GaudiTuples.h
 *
 *  Templated base class providing common 'ntupling' methods
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-08-08
 */
// ============================================================================
template <class PBASE>
class GAUDI_API GaudiTuples : public PBASE {
public:
  // ==========================================================================
  /// the actual type for histogram identifier
  typedef GaudiAlg::HistoID HistoID;
  /// the actual type of the tuple
  typedef Tuples::Tuple Tuple;
  /// the actual type of N-tuple ID
  typedef GaudiAlg::TupleID TupleID;
  // ==========================================================================
public:
  // ==========================================================================
  /** Access an N-Tuple object (book on-demand) with unique identifier
   *
   *  @code
   *
   *  Tuple tuple = nTuple( "My N-Tuple" ) ;
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write();
   *
   *  @endcode
   *
   *  @attention
   *  The NTuple will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first NTuple booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given NTuple will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the NTuple are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given NTuple could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @param title   Unique title for N-Tuple
   *  @param clid    N-Tuple class identifier (row or column wise)
   *  @return ntuple The ntuple object
   */
  Tuple nTuple( const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const;

  /** Access an N-Tuple object (book on-demand) with forced identifier
   *
   *  @code
   *
   *  // Book Ntuple with a numeric ID
   *  Tuple tuple = nTuple(  1,  "My tuple" ) ;
   *  // ... or
   *  Tuple tuple = nTuple( "1", "My tuple" ) ;
   *
   *  // ... or, Book Ntuple with a literal ID
   *  Tuple tuple = nTuple( "mytuple", "My tuple" ) ;
   *
   *  // ... or, Book Ntuple with a literal ID in a sub-dir
   *  Tuple tuple = nTuple( "subdir/mytuple", "My tuple" ) ;
   *
   *  // Fill and write NTuple
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write();
   *
   *  @endcode
   *
   *  @attention
   *   If the N-Tuple with given ID is already booked
   *   through automatic assignment of N-Tuple ID,
   *   the error will not be detected.
   *   Therefore it is recommended
   *   to use a non-trivial N-Tuple ID offset  (property "NTupleOffSet")
   *   if one need to combine these techniques together
   *   It is still desirable to use the unique N-Tuple title
   *   to avoid a bad interference.
   *
   *  @param ID      The forced N-Tuple ID
   *  @param title   Unique title for N-Tuple
   *  @param clid    N-Tuple class identifier (row or column wise)
   *  @return ntuple The ntuple object
   */
  Tuple nTuple( const TupleID& ID, const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const;

  /** Access an Event Tag Collection object (book on-demand) with unique identifier
   *
   *  @code
   *
   *  Tuple tuple = evtCol( "My Tag Collection" ) ;
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write();
   *
   *  @endcode
   *
   *  The Event Tag Collection will get a unique identifier automatically assigned which by
   *  default will be equal to the histogram title. An option exists to instead
   *  use numerical IDs. In this case the first  Event Tag Collection booked will be ID=1 the
   *  next ID=2 and so on. Note though this scheme is not recommended as it does
   *  NOT guarantee predictability of the ID a given  Event Tag Collection will be given when
   *  filled under conditional statements, since in these circumstances the order
   *  in which the  Event Tag Collection are first filled, and thus booked, will depend on the
   *  nature of the first few events read. This is particularly problematic when
   *  users submit many parallel 'sub-jobs' and then attempt to merge the final
   *  output ROOT (or HBOOK) files, since a given  Event Tag Collection could have different IDs
   *  in each of the sub-jobs. Consequently it is strongly recommended that users do
   *  not use numerical automatic IDs unless they are sure they understand what they
   *  are doing.
   *
   *  @param title   Unique title for Event Tag Collection
   *  @param clid    N-Tuple class identifier (row or column wise)
   *  @return ntuple The Event Tag Collection object
   */
  Tuple evtCol( const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const;

  /** Access an Event Tag Collection object (book on-demand) with forced identifier
   *
   *  @code
   *
   *  // Book Ntuple with a numeric ID
   *  Tuple tuple = evtCol(  1,  "My Tag Collection" ) ;
   *  // ... or
   *  Tuple tuple = evtCol( "1", "My Tag Collection" ) ;
   *
   *  // ... or, Book Ntuple with a literal ID
   *  Tuple tuple = evtCol( "mytuple", "My Tag Collection" ) ;
   *
   *  // ... or, Book Ntuple with a literal ID in a sub-dir
   *  Tuple tuple = evtCol( "subdir/mytuple", "My Tag Collection" ) ;
   *
   *  // Fill and write
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write();
   *
   *  @endcode
   *
   *  @attention
   *   If the Event Tag Collection with given ID is already booked
   *   through automatic assignment of Event Tag Collection ID,
   *   the error will not be detected.
   *   Therefore it is recommended
   *   to use a non-trivial Event Tag Collection ID offset  (property "EvtColOffSet")
   *   if one need to combine these techniques together
   *   It is still desirable to use the unique Event Tag Collection title
   *   to avoid a bad interference.
   *
   *  @param ID      The forced Event Tag Collection ID
   *  @param title   Unique title for Event Tag Collection
   *  @param clid    N-Tuple class identifier (row or column wise)
   *  @return ntuple The Event Tag Collection object
   */
  Tuple evtCol( const TupleID& ID, const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const;
  // ==========================================================================
public: // trivial accessors
  // ==========================================================================
  /// get the flag for N-Tuple production (property "NTupleProduce")
  bool produceNTuples() const { return m_produceNTuples; }
  /// get the flag for N-Tuple path split (property "NTupleSplitDir")
  bool splitNTupleDir() const { return m_splitNTupleDir; }
  /// get the logical unit for N-Tuples   (property "NTupleLUN")
  const std::string& nTupleLUN() const { return m_nTupleLUN; }
  /// get the top-level N-Tuple directory (property "NTupleTopDir")
  const std::string& nTupleTopDir() const { return m_nTupleTopDir; }
  /// get the N-Tuple directory           (property "NTupleDir")
  const std::string& nTupleDir() const { return m_nTupleDir; }
  /// get the value for N-Tuple offset    (property "NTupleOffSet")
  TupleID::NumericID nTupleOffSet() const { return m_nTupleOffSet; }
  /// get the constructed N-Tuple path
  std::string nTuplePath() const;
  /// get the flag for Event Tag Collection production (property "EvtColsProduce")
  bool produceEvtCols() const { return m_produceEvtCols; }
  /// get the flag for Event Tag Collection path split (property "EvtColsSplitDir")
  bool splitEvtColDir() const { return m_splitEvtColDir; }
  /// get the logical unit for Event Tag Collections   (property "EvtColsLUN")
  const std::string& evtColLUN() const { return m_evtColLUN; }
  /// get the top-level Event Tag Collection directory (property "EvtColsTopDir")
  const std::string& evtColTopDir() const { return m_evtColTopDir; }
  /// get the Event Tag Collection directory           (property "EvtColsDir")
  const std::string& evtColDir() const { return m_evtColDir; }
  /// get the value for Event Tag Collection offset    (property "EvtColsOffSet")
  TupleID::NumericID evtColOffSet() const { return m_evtColOffSet; }
  /// get the constructed Event Tag Collection path
  std::string evtColPath() const;
  /// print tuples at finalization
  bool tuplesPrint() const { return m_tuplesPrint; }
  /// print event collections at finalization
  bool evtColsPrint() const { return m_evtColsPrint; }
  // ==========================================================================
public:
  // ==========================================================================
  /** perform the actual printout of N-tuples
   *  @return number of active N-Tuples
   */
  long printTuples() const;
  /** perform the actual printout of Event Tag Collections
   *  @return number of active Event Tag Collections
   */
  long printEvtCols() const;
  // ==========================================================================
  /// check the existence AND validity of the N-Tuple with the given ID
  bool nTupleExists( const TupleID& ID ) const;
  /// check the existence AND validity of the Event Tag Collection with the given ID
  bool evtColExists( const TupleID& ID ) const;
  // ==========================================================================
protected:
  // ==========================================================================
  /** create TupleObj
   *  @attention The method should never used directly by users
   *  @param name  name/title
   *  @param tuple the underlying ntuple implementation
   *  @param clid  unique classID for ntuple
   *  @return pointer to newly created TupleObj
   */
  virtual std::unique_ptr<Tuples::TupleObj> createNTuple( const std::string& name, NTuple::Tuple* tuple,
                                                          const CLID& clid ) const;
  /** create TupleObj for event tag collection
   *  @attention The method should never used directly by users
   *  @param name  name/title
   *  @param tuple the underlying ntuple implementation
   *  @param clid  unique classID for ntuple
   *  @return pointer to newly created TupelObj
   */
  virtual std::unique_ptr<Tuples::TupleObj> createEvtCol( const std::string& name, NTuple::Tuple* tuple,
                                                          const CLID& clid ) const;
  // ==========================================================================
public:
  // ==========================================================================
  /// Algorithm constructor - the SFINAE constraint below ensures that this is
  /// constructor is only defined if PBASE derives from GaudiHistoAlg
  template <typename U = PBASE, typename = std::enable_if_t<std::is_base_of_v<GaudiHistoAlg, PBASE>, U>>
  GaudiTuples( const std::string& name, ISvcLocator* pSvcLocator ) : PBASE( name, pSvcLocator ) {}
  /// Tool constructor - SFINAE-ed to insure this constructor is only defined
  /// if PBASE derives from AlgTool.
  template <typename U = PBASE, typename = std::enable_if_t<std::is_base_of_v<GaudiHistoTool, PBASE>, U>>
  GaudiTuples( const std::string& type, const std::string& name, const IInterface* parent )
      : PBASE( type, name, parent ) {}
  // ==========================================================================
protected:
  // ==========================================================================
  /** standard initialization method
   *  @return status code
   */
  StatusCode initialize() override
#ifdef __ICC
  {
    return i_gtInitialize();
  }
  StatusCode i_gtInitialize()
#endif
      ;
  /** standard finalization method
   *  @return status code
   */
  StatusCode finalize() override
#ifdef __ICC
  {
    return i_gtFinalize();
  }
  StatusCode i_gtFinalize()
#endif
      ;
  // ==========================================================================
private:
  // ==========================================================================
  Gaudi::Property<bool> m_produceNTuples{this, "NTupleProduce", true, "general switch to enable/disable N-tuples"};
  Gaudi::Property<bool> m_tuplesPrint{this, "NTuplePrint", true,
                                      [this]( auto& ) {
                                        // no action if not yet initialized
                                        if ( this->FSMState() >= Gaudi::StateMachine::INITIALIZED &&
                                             this->tuplesPrint() )
                                          this->printTuples();
                                      },
                                      "print N-tuple statistics"};
  Gaudi::Property<bool> m_splitNTupleDir{this, "NTupleSplitDir", false,
                                         "split long directory names into short pieces (suitable for HBOOK)"};
  Gaudi::Property<TupleID::NumericID> m_nTupleOffSet{this, "NTupleOffSet", 0, "offset for numerical N-tuple ID"};
  Gaudi::Property<std::string>        m_nTupleLUN{this, "NTupleLUN", "FILE1", "Logical File Unit for N-tuples"};
  Gaudi::Property<std::string>        m_nTupleTopDir{this, "NTupleTopDir", "", "top-level directory for N-Tuples"};
  Gaudi::Property<std::string>        m_nTupleDir{
      this, "NTupleDir", boost::algorithm::replace_all_copy( this->name(), ":", "_" ), "subdirectory for N-Tuples"};

  Gaudi::Property<bool> m_produceEvtCols{this, "EvtColsProduce", false,
                                         "general switch to enable/disable Event Tag Collections"};
  Gaudi::Property<bool> m_evtColsPrint{this, "EvtColsPrint", false,
                                       [this]( auto& ) {
                                         // no action if not yet initialized
                                         if ( this->FSMState() >= Gaudi::StateMachine::INITIALIZED &&
                                              this->evtColsPrint() )
                                           this->printEvtCols();
                                       },
                                       "print statistics for Event Tag Collections "};
  Gaudi::Property<bool> m_splitEvtColDir{this, "EvtColSplitDir", false, "split long directory names into short pieces"};
  Gaudi::Property<TupleID::NumericID> m_evtColOffSet{this, "EvtColOffSet", 0, "offset for numerical N-tuple ID"};
  Gaudi::Property<std::string> m_evtColLUN{this, "EvtColLUN", "EVTCOL", "Logical File Unit for Event Tag Collections"};
  Gaudi::Property<std::string> m_evtColTopDir{this, "EvtColTopDir", "",
                                              "Top-level directory for Event Tag Collections"};
  Gaudi::Property<std::string> m_evtColDir{this, "EvtColDir",
                                           boost::algorithm::replace_all_copy( this->name(), ":", "_" ),
                                           "Subdirectory for Event Tag Collections"};
  struct nTupleMapItem         final {
    std::string                       title;
    TupleID                           id;
    std::shared_ptr<Tuples::TupleObj> tuple;
  };
  struct title_t {};
  struct id_t {};
  struct order_t {};
  template <typename... Ts>
  using indexed_by = boost::multi_index::indexed_by<Ts...>;
  template <typename... Ts>
  using hashed_unique = boost::multi_index::hashed_unique<Ts...>;
  template <typename... Ts>
  using ordered_unique = boost::multi_index::ordered_unique<Ts...>;
  template <typename Obj, typename Type, Type Obj::*Member>
  using member = boost::multi_index::member<Obj, Type, Member>;
  template <typename T>
  using tag       = boost::multi_index::tag<T>;
  using nTupleMap = boost::multi_index_container<
      nTupleMapItem, indexed_by<hashed_unique<tag<title_t>, member<nTupleMapItem, std::string, &nTupleMapItem::title>>,
                                hashed_unique<tag<id_t>, member<nTupleMapItem, TupleID, &nTupleMapItem::id>>,
                                ordered_unique<tag<order_t>, member<nTupleMapItem, TupleID, &nTupleMapItem::id>>>>;

  // ==========================================================================
  /// the actual storage of ntuples by title and ID
  mutable nTupleMap m_nTupleMap;

  decltype( auto ) nTupleByID() const { return m_nTupleMap.template get<id_t>(); }
  decltype( auto ) nTupleByTitle() const { return m_nTupleMap.template get<title_t>(); }
  decltype( auto ) nTupleOrdered() const { return m_nTupleMap.template get<order_t>(); }

  // ==========================================================================
  /// the actual storage of event collections by title and ID
  mutable nTupleMap m_evtColMap;

  decltype( auto ) evtColByID() const { return m_evtColMap.template get<id_t>(); }
  decltype( auto ) evtColByTitle() const { return m_evtColMap.template get<title_t>(); }
  decltype( auto ) evtColOrdered() const { return m_evtColMap.template get<order_t>(); }

  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLES_H
