/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Required for inheritance
#include "OutputStream.h"

class IDataStoreLeaves;

/** Extension of OutputStream to copy the content of the main input file.
 *
 *  On top of the standard behavior of OutputStream, this class takes also all
 *  the entries that come from the same file as the root entry in the data
 *  service.
 *
 *  @author:  M. Frank
 *  @author:  P. Koppenburg
 *  @author:  M. Clemencic
 */
class InputCopyStream : public OutputStream {

public:
  /// Standard algorithm Constructor
  InputCopyStream( const std::string& name, ISvcLocator* pSvcLocator );

  /// Initialize the instance.
  StatusCode initialize() override;

  /// Finalize the instance.
  StatusCode finalize() override;

  /// Collect all objects to be written to the output stream
  StatusCode collectObjects() override;

private:
  /// Pointer to the (public) tool used to retrieve the objects in a file.
  IDataStoreLeaves* m_leavesTool = nullptr;

  Gaudi::Property<std::vector<std::string>> m_tesVetoList{ this, "TESVetoList", {}, "names of TES locations to Veto" };

protected:
  /// Overridden from the base class (InputCopyStream has always input).
  bool hasInput() const override { return true; }
};
