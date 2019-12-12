/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
#include <set>
#include <string>
#include <vector>
// ============================================================================
// local
// ============================================================================
#include "../PersistencySvc/PersistencySvc.h"
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
class HistogramPersistencySvc : virtual public PersistencySvc {
public:
  /**@name PersistencySvc overrides    */
  //@{
  /// Initialize the service.
  StatusCode initialize() override;
  /// Reinitialize the service.
  StatusCode reinitialize() override;
  /// Finalize the service.
  StatusCode finalize() override;
  /// Implementation of IConverter: Convert the transient object to the requested representation.
  StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;
  //@}

  /**@name: Object implementation  */
  //@{
  /// Standard Constructor
  HistogramPersistencySvc( const std::string& name, ISvcLocator* svc );

  //@}
public:
  // ==========================================================================
  /// for report: unconverted histograms
  typedef std::set<std::string> Set; // unconverted histograms
  // ==========================================================================
protected:
  Gaudi::Property<std::string> m_histPersName{this, "HistogramPersistency", "", "name of the Hist Pers type"};
  Gaudi::Property<std::string> m_outputFile{this, "OutputFile", "", "name of the output file"};
  Gaudi::Property<std::vector<std::string>> m_convert{
      this, "ConvertHistos", {}, "The list of patterns to be accepted for conversion"};
  Gaudi::Property<std::vector<std::string>> m_exclude{
      this, "ExcludeHistos", {}, "The list of patterns to be excluded from conversion"};
  Gaudi::Property<bool> m_warnings{this, "Warnings", true, "Set this property to false to suppress warning messages"};

  /// for the final report: the list of converted histograms
  Set m_converted;
  /// for the final report: the list of excluded histograms
  Set m_excluded;
  // ==========================================================================
};
// ============================================================================
// The END
// ============================================================================
#endif // PERSISTENCYSVC_HISTOGRAMPERSISTENCYSVC_H
