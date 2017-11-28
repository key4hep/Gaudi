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
class TupleTool : public GaudiTupleTool, virtual public ITupleTool
{
public:
  typedef Tuples::Tuple Tuple;
  typedef GaudiAlg::TupleID TupleID;

  /** Standard constructor
   *  @see GaudiTupleTool
   *  @see GaudiHistoTool
   *  @see      GaudiTool
   *  @see        AlgTool
   *  @param type   tool type (?)
   *  @param name   tool name
   *  @param parent pointer to parent component
   */
  TupleTool( const std::string& type, const std::string& name, const IInterface* parent );

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
  Tuple nTuple( const std::string& title, const CLID& clid = CLID_ColumnWiseTuple ) const override
  {
    return GaudiTupleTool::nTuple( title, clid );
  }

  Tuple nTuple( const TupleID& ID, const std::string& title = "",
                const CLID& clid = CLID_ColumnWiseTuple ) const override
  {
    return GaudiTupleTool::nTuple( ID, title, clid );
  }

  Tuple evtCol( const std::string& title, const CLID& clid = CLID_RowWiseTuple ) const override
  {
    return GaudiTupleTool::evtCol( title, clid );
  }

  Tuple evtCol( const TupleID& ID, const std::string& title = "", const CLID& clid = CLID_RowWiseTuple ) const override
  {
    return GaudiTupleTool::evtCol( ID, title, clid );
  }

  TupleTool()                   = delete;
  TupleTool( const TupleTool& ) = delete;
  TupleTool& operator=( const TupleTool& ) = delete;
};
#endif // GAUDIALG_TUPLETOOL_H
