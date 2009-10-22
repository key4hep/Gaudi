// $Id: TupleTool.h,v 1.3 2006/11/30 10:16:13 mato Exp $
// ============================================================================
#ifndef GAUDIALG_TUPLETOOL_H 
#define GAUDIALG_TUPLETOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiAlg/GaudiTupleTool.h"
#include "GaudiAlg/ITupleTool.h"
// ============================================================================

/** @class TupleTool TupleTool.h components/TupleTool.h
 *  
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */
class TupleTool :         public GaudiTupleTool , 
                  virtual public     ITupleTool 
{
public:
  typedef Tuples::Tuple     Tuple   ;  
  typedef GaudiAlg::TupleID TupleID ;  
  
  /** Standard constructor
   *  @see GaudiTupleTool 
   *  @see GaudiHistoTool
   *  @see      GaudiTool
   *  @see        AlgTool 
   *  @param type   tool type (?) 
   *  @param name   tool name 
   *  @param parent pointer to parent component 
   */
  TupleTool( const std::string& type, 
             const std::string& name,
             const IInterface* parent);

  /// destructor: virtual and protected 
  virtual ~TupleTool() ;

public: 

  /** get N-tuple object ( book on-demand ) with unique identidier 
   *  
   *  @code
   *
   *  ITupleTool* tool = .. ;
   *  Tuple tuple = tool-> nTuple( 'My tuple' ) ;
   *  tuple->column( "A" , sin(0.1) );
   *  tuple->column( "B" , cos(0.1) );
   *  tuple->column( "C" , tan(0.1) );
   *  tuple->write()
   *
   *  @endcode 
   *
   *  NTuple will be booked with automatically assigned numerical ID 
   *
   *  @param title unique title for ntuple 
   *  @param clid  class identifier 
   *  @return ntuple object 
   */
  virtual Tuple  nTuple 
  ( const std::string& title                        , 
    const CLID&        clid  = CLID_ColumnWiseTuple ) const 
  { return GaudiTupleTool::nTuple (      title , clid ) ; }
  
  
  virtual Tuple  nTuple 
  ( const TupleID&     ID                           , 
    const std::string& title = ""                   , 
    const CLID&        clid  = CLID_ColumnWiseTuple ) const 
  { return GaudiTupleTool::nTuple ( ID , title , clid ) ; }
  
  virtual Tuple  evtCol 
  ( const std::string& title                        , 
    const CLID&        clid  = CLID_RowWiseTuple    ) const 
  { return GaudiTupleTool::evtCol (      title , clid ) ; }
  
  virtual Tuple  evtCol 
  ( const TupleID&     ID                           , 
    const std::string& title = ""                   , 
    const CLID&        clid  = CLID_RowWiseTuple    ) const 
  { return GaudiTupleTool::evtCol ( ID , title , clid ) ; }

private:

  TupleTool() ;
  TupleTool           ( const TupleTool& ) ;
  TupleTool& operator=( const TupleTool& ) ;

};
#endif // GAUDIALG_TUPLETOOL_H
