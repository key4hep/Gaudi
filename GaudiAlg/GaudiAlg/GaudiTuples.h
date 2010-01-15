// $Id: GaudiTuples.h,v 1.7 2008/10/27 19:22:20 marcocle Exp $
// ============================================================================
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
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ClassID.h"
// ============================================================================
// GaudiAlg
// ============================================================================
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
class GAUDI_API GaudiTuples: public PBASE
{
public:
  // ==========================================================================
  /// the actual type for histogram identifier
  typedef GaudiAlg::HistoID         HistoID;
  /// the actual type of the tuple
  typedef Tuples::Tuple             Tuple         ;
  /// the actual type of N-tuple ID
  typedef GaudiAlg::TupleID         TupleID       ;
  /// the actual type of (title) -> (tuple) mappping
  typedef GaudiAlg::TupleMapTitle   TupleMapTitle ;
  /// the actual type of    (Numeric ID) -> (tuple) mappping
  typedef GaudiAlg::TupleMapID      TupleMapID ;
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
  Tuple  nTuple ( const std::string& title                        ,
                  const CLID&        clid  = CLID_ColumnWiseTuple ) const ;

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
  Tuple  nTuple ( const TupleID&     ID                           ,
                  const std::string& title                        ,
                  const CLID&        clid  = CLID_ColumnWiseTuple ) const ;

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
  Tuple  evtCol ( const std::string& title                        ,
                  const CLID&        clid  = CLID_ColumnWiseTuple ) const ;

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
  Tuple  evtCol ( const TupleID&     ID                           ,
                  const std::string& title                        ,
                  const CLID&        clid  = CLID_ColumnWiseTuple ) const ;
  // ==========================================================================
public:  // trivial accessors
  // ==========================================================================
  /// get the flag for N-Tuple production (property "NTupleProduce")
  bool               produceNTuples () const { return m_produceNTuples ; }
  /// get the flag for N-Tuple path split (property "NTupleSplitDir")
  bool               splitNTupleDir () const { return m_splitNTupleDir ; }
  /// get the logical unit for N-Tuples   (property "NTupleLUN")
  const std::string& nTupleLUN      () const { return m_nTupleLUN      ; }
  /// get the top-level N-Tuple directory (property "NTupleTopDir")
  const std::string& nTupleTopDir   () const { return m_nTupleTopDir   ; }
  /// get the N-Tuple directory           (property "NTupleDir")
  const std::string& nTupleDir      () const { return m_nTupleDir      ; }
  /// get the value for N-Tuple offset    (property "NTupleOffSet")
  TupleID::NumericID nTupleOffSet   () const { return m_nTupleOffSet   ; }
  /// get the constructed N-Tuple path
  std::string        nTuplePath     () const
  {
    const std::string path = nTupleLUN() + "/" + nTupleTopDir() + nTupleDir();
    return ( splitNTupleDir() ? dirHbookName( path ) : path ) ;
  }
  /// get the flag for Event Tag Collection production (property "EvtColsProduce")
  bool               produceEvtCols () const { return m_produceEvtCols ; }
  /// get the flag for Event Tag Collection path split (property "EvtColsSplitDir")
  bool               splitEvtColDir () const { return m_splitEvtColDir ; }
  /// get the logical unit for Event Tag Collections   (property "EvtColsLUN")
  const std::string& evtColLUN      () const { return m_evtColLUN      ; }
  /// get the top-level Event Tag Collection directory (property "EvtColsTopDir")
  const std::string& evtColTopDir   () const { return m_evtColTopDir   ; }
  /// get the Event Tag Collection directory           (property "EvtColsDir")
  const std::string& evtColDir      () const { return m_evtColDir      ; }
  /// get the value for Event Tag Collection offset    (property "EvtColsOffSet")
  TupleID::NumericID evtColOffSet   () const { return m_evtColOffSet   ; }
  /// get the constructed Event Tag Collection path
  std::string        evtColPath     () const
  {
    std::string path = evtColLUN() + "/" + evtColTopDir() + evtColDir();
    return ( splitEvtColDir() ? dirHbookName( path ) : path );
  }
  /// print tuples at finalization
  bool tuplesPrint  () const { return m_tuplesPrint  ; }
  /// print event collections at finalization
  bool evtColsPrint () const { return m_evtColsPrint ; }
  // ==========================================================================
public :
  // ==========================================================================
  /** perform the actual printout of N-tuples
   *  @return number of active N-Tuples
   */
  long printTuples  () const ;
  /** perform the actual printout of Event Tag Collections
   *  @return number of active Event Tag Collections
   */
  long printEvtCols () const ;
  // ==========================================================================
public :
  // ==========================================================================
  /// check the existence AND validity of the N-Tuple with the given ID
  bool nTupleExists ( const TupleID& ID ) const;
  /// check the existence AND validity of the Event Tag Collection with the given ID
  bool evtColExists ( const TupleID& ID ) const;
  // ==========================================================================
protected:
  // ==========================================================================
  /// access to the all ntuples by title
  const TupleMapTitle& nTupleMapTitle () const { return m_nTupleMapTitle ; }
  /// access to the all evet tag collections by title
  const TupleMapTitle& evtColMapTitle () const { return m_evtColMapTitle ; }
  /// access to the all ntuples by numeric ID
  const TupleMapID&    nTupleMapID    () const { return m_nTupleMapID    ; }
  /// access to the all evet tag collections by numeric ID
  const TupleMapID&    evtColMapID    () const { return m_evtColMapID    ; }
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
  virtual Tuples::TupleObj*
  createNTuple ( const std::string& name  ,
                 NTuple::Tuple*     tuple ,
                 const CLID&        clid  ) const ;
  /** create TupleObj for event tag collection
   *  @attention The method should never used directly by users
   *  @param name  name/title
   *  @param tuple the underlying ntuple implementation
   *  @param clid  unique classID for ntuple
   *  @return pointer to newly created TupelObj
   */
  virtual Tuples::TupleObj*
  createEvtCol ( const std::string& name  ,
                 NTuple::Tuple*     tuple ,
                 const CLID&        clid  ) const ;
  // ==========================================================================
public:
  // ==========================================================================
  /// Algorithm constructor
  GaudiTuples ( const std::string & name,
                ISvcLocator * pSvcLocator );
  /// Tool constructor
  GaudiTuples ( const std::string& type   ,
                const std::string& name   ,
                const IInterface*  parent );
  /// Destructor
  virtual ~GaudiTuples();
  // ==========================================================================
protected:
  // ==========================================================================
  /** standard initialization method
   *  @return status code
   */
  virtual StatusCode initialize()
#ifdef __ICC
    { return i_gtInitialize(); }
  StatusCode i_gtInitialize()
#endif
  ;
  /** standard finalization method
   *  @return status code
   */
  virtual StatusCode finalize()
#ifdef __ICC
    { return i_gtFinalize(); }
  StatusCode i_gtFinalize()
#endif
  ;
  // ==========================================================================
private:
  // ==========================================================================
  /// Constructor initialization and job options
  inline void initGaudiTuplesConstructor()
  {
    m_produceNTuples = true ;     // Switch ON/OFF ntuple production
    m_splitNTupleDir = false ;    // for HBOOK it is better to use 'true'
    m_nTupleLUN      = "FILE1" ;  // logical unit for ntuples
    m_nTupleTopDir   = "" ;       // top level ntuple directory
    m_nTupleDir      = this->name() ;   // ntuple directory
    m_nTupleOffSet   = 0  ;       // offset for ntuples
    //
    m_produceEvtCols = false ;    // Switch ON/OFF ntupel production
    m_splitEvtColDir = false ;    // for HBOOK it is better to use 'true'
    m_evtColLUN      = "EVTCOL" ; // logical unit for ntuples
    m_evtColTopDir   = ""    ;    // top level ntuple directory
    m_evtColDir      = this->name() ;   // ntuple directory
    m_evtColOffSet   = 0   ;      // offset for ntuples
    //
    m_tuplesPrint    = true  ;    // print tuples at end of job
    m_evtColsPrint   = false  ;   // print event collections at end of job
    //
    this -> declareProperty
      ( "NTupleProduce"  , m_produceNTuples         ,
        "General switch to enable/disable N-tuples" ) ;
    this -> declareProperty
      ( "NTuplePrint"    , m_tuplesPrint    ,
        "Print N-tuple statistics"        )
      -> declareUpdateHandler ( &GaudiTuples<PBASE>::printNTupleHandler , this ) ;
    this -> declareProperty
      ( "NTupleSplitDir" , m_splitNTupleDir ,
        "Split long directory names into short pieces (suitable for HBOOK)" ) ;
    this -> declareProperty
      ( "NTupleOffSet"   , m_nTupleOffSet   ,
        "Offset for numerical N-tuple ID" ) ;
    this -> declareProperty
      ( "NTupleLUN"      , m_nTupleLUN      ,
        "Logical File Unit for N-tuples"  ) ;
    this -> declareProperty
      ( "NTupleTopDir"   , m_nTupleTopDir   ,
        "Top-level directory for N-Tuples") ;
    this -> declareProperty
      ( "NTupleDir"      , m_nTupleDir      ,
        "Subdirectory for N-Tuples"       ) ;
    // ========================================================================
    this -> declareProperty
      ( "EvtColsProduce" , m_produceEvtCols ,
        "General switch to enable/disable Event Tag Collections" ) ;
    this -> declareProperty
      ( "EvtColsPrint"   , m_evtColsPrint   ,
        "Print statistics for Event Tag Collections " )
      -> declareUpdateHandler ( &GaudiTuples<PBASE>::printEvtColHandler , this ) ;
    this -> declareProperty
      ( "EvtColSplitDir" , m_splitEvtColDir ,
        "Split long directory names into short pieces" ) ;
    this -> declareProperty
      ( "EvtColOffSet"   , m_evtColOffSet   ,
        "Offset for numerical N-tuple ID" ) ;
    this -> declareProperty
      ( "EvtColLUN"      , m_evtColLUN      ,
        "Logical File Unit for Event Tag Collections"   ) ;
    this -> declareProperty
      ( "EvtColTopDir"   , m_evtColTopDir   ,
        "Top-level directory for Event Tag Collections" ) ;
    this -> declareProperty
      ( "EvtColDir"      , m_evtColDir      ,
        "Subdirectory for Event Tag Collections"        ) ;
    // ========================================================================
  }
  // ==========================================================================
private:
  // ==========================================================================
  /// handler for "NTuplePrint" property
  void printNTupleHandler  ( Property& /* theProp */ ) ; //       "NTuplePrint"
  /// handler for "EvtColsPrint" property
  void printEvtColHandler  ( Property& /* theProp */ ) ; //      "EvtcolsPrint"
  // ==========================================================================
private:
  // ==========================================================================
  /// flag to switch ON/OFF the ntuple filling and booking
  bool        m_produceNTuples ;
  /// flag to indicate splitting of tuple directories (useful for HBOOK)
  bool        m_splitNTupleDir ;
  /// name of logical unit for tuple directory
  std::string m_nTupleLUN      ;
  /// top level tuple directory
  std::string m_nTupleTopDir   ;
  /// local tuple directory
  std::string m_nTupleDir      ;
  /// the offset for ntuple numerical ID
  TupleID::NumericID     m_nTupleOffSet   ;
  // ==========================================================================
  /// flag to switch ON/OFF the ntuple filling and booking
  bool        m_produceEvtCols ;
  /// flag to indicate splitting of tuple directories (useful for HBOOK)
  bool        m_splitEvtColDir ;
  /// name of Logical Unit for tuple directory
  std::string m_evtColLUN      ;
  /// top level tuple directory
  std::string m_evtColTopDir   ;
  /// local tuple directory
  std::string m_evtColDir      ;
  /// the offset for ntuple numerical ID
  TupleID::NumericID     m_evtColOffSet   ;
  // ==========================================================================
  /// print tuples at finalization?
  bool m_tuplesPrint    ;                      // print tuples at finalization?
  /// print event collections at finalization
  bool m_evtColsPrint   ;            // print event collections at finalization
  // ==========================================================================
  /// the actual storage of ntuples by title
  mutable TupleMapTitle  m_nTupleMapTitle ;
  /// the actual storage of ntuples by ID
  mutable TupleMapID     m_nTupleMapID    ;
  // ==========================================================================
  /// the actual storage of event collections by title
  mutable TupleMapTitle  m_evtColMapTitle ;
  /// the actual storage of event collections by ID
  mutable TupleMapID     m_evtColMapID    ;
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLES_H
// ============================================================================
