// $Id: HistogramPersistencySvc.h,v 1.5 2008/10/09 13:40:18 marcocle Exp $ 
//	===========================================================================
//
//	HistogramPersistencySvc.h
//	------------------------------------------------------------
//
//	Package   : PersistencySvc
//
//	Author    : Markus Frank
//
//	===========================================================
#ifndef PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_H
#define PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_H 1
// ============================================================================
// Incldue files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
#include <vector>
#include <set>
// ============================================================================
// local 
// ============================================================================
#include "PersistencySvc.h"
// ============================================================================
/** HistogramPersistencySvc class implementation definition.
 * 
 * <P> System:  The LHCb Offline System
 * <P> Package: HistogramPersistencySvc
 * 
 * Dependencies:
 * <UL>
 *  <LI> PersistencySvc definition:  "Kernel/Interfaces/PersistencySvc.h"
 *   </UL>
 *
 * History:
 * <PRE>
 * +---------+----------------------------------------------+---------+
 * |    Date |                 Comment                      | Who     |
 * +---------+----------------------------------------------+---------+
 * | 3/11/98 | Initial version                              | M.Frank |
 * +---------+----------------------------------------------+---------+
 * </PRE>
 * @author Markus Frank
 * @version 1.0
 */
class HistogramPersistencySvc  : virtual public PersistencySvc	
{
public:
  friend class SvcFactory<HistogramPersistencySvc>;
  friend class Factory<HistogramPersistencySvc,IService* (std::string,ISvcLocator *)>;
public:
  /**@name PersistencySvc overrides    */
  //@{
  /// Initialize the service.
  virtual StatusCode initialize();
  /// Reinitialize the service.
  virtual StatusCode reinitialize();
  /// Finalize the service.
  virtual StatusCode finalize();
  /// Implementation of IConverter: Convert the transient object to the requested representation.
  virtual StatusCode createRep(DataObject* pObject, IOpaqueAddress*& refpAddress);
  //@}
  
  /**@name: Object implementation  */
  //@{
  /// Standard Constructor
  HistogramPersistencySvc(const std::string& name, ISvcLocator* svc);
  
  /// Standard Destructor
  virtual ~HistogramPersistencySvc();
  //@}
public:
  // ==========================================================================
  /// the actual type for the vector of strings  
  typedef std::vector<std::string>  Strings ; // the vector of strings  
  /// for report: unconverted histograms  
  typedef std::set<std::string>     Set     ; // unconverted histograms  
  // ==========================================================================
protected:
  /// Name of the Hist Pers type
  std::string       m_histPersName; // Name of the Hist Pers type
  /// Name of the outputFile
  std::string       m_outputFile  ; // Name of the outputFile
  /// the list of patterns to be converted 
  Strings m_convert ; // the list of patterns to be converted 
  /// the list of patterns to be excludes 
  Strings m_exclude ; // the list of patterns to be excludes
  /// for the final report: the list of converted histograms 
  Set  m_converted ;
  /// for the final report: the list of excluded histograms 
  Set  m_excluded  ;
  /// Flag to disable warning messages when using external input
  bool              m_warnings;
  // ==========================================================================
};
// ============================================================================
// The END 
// ============================================================================
#endif // PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_H
// ============================================================================
