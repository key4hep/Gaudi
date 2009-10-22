// $Id: ITupleTool.h,v 1.2 2004/10/18 08:18:00 mato Exp $
// ============================================================================
#ifndef GAUDIALG_ITUPLETOOL_H
#define GAUDIALG_ITUPLETOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/ClassID.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/TupleID.h"
#include "GaudiAlg/Tuple.h"
// ============================================================================


// ============================================================================
/** @class ITupleTool ITupleTool.h GaudiTools/ITupleTool.h
 *
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-06-28
 */
// ============================================================================

class GAUDI_API ITupleTool: virtual public IAlgTool
{
public:
  /// InterfaceID
  DeclareInterfaceID(ITupleTool,2,0);

  typedef Tuples::TupleID   TupleID ;
  typedef Tuples::Tuple     Tuple   ;

  /** get N-tuple object ( book on-demand ) with unique identifier
   *
   *  @code
   *
   *  ITupleTool* tool = .. ;
   *  Tuple tuple = tool->nTuple( 'My tuple' ) ;
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
    const CLID&        clid  = CLID_ColumnWiseTuple ) const = 0 ;

  virtual Tuple  nTuple
  ( const TupleID&     ID                           ,
    const std::string& title = ""                   ,
    const CLID&        clid  = CLID_ColumnWiseTuple ) const = 0 ;

  virtual Tuple  evtCol
  ( const std::string& title                        ,
    const CLID&        clid  = CLID_RowWiseTuple    ) const = 0 ;

  virtual Tuple  evtCol
  ( const TupleID&     ID                           ,
    const std::string& title = ""                   ,
    const CLID&        clid  = CLID_RowWiseTuple    ) const = 0 ;

protected:

  virtual ~ITupleTool( ); ///< Destructor

};

// ============================================================================
#endif // GAUDIALG_ITUPLETOOL_H
// ============================================================================
