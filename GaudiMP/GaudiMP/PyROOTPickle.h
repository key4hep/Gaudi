// This file's extension implies that it's C, but it's really -*- C++ -*-.

/**
 * @file GaudiMP/PyROOTPickle.h
 * @author Wim Lavrijsen
 * @date Apr 2008
 * @brief Port pickling functionality while awaiting newer release.
 *
 * PyROOT does not implement direct pickling of ObjectProxy's until release
 * 5.20 of ROOT; this code brings it forward for use in ATLAS.
 *
 * The code here will make the following changes to PyROOT's behavior:
 *
 *  - Add a method "_ObjectProxy__expand__" to libPyROOT that is stored by
 *     name by the pickler used by the unpickler. The expand function takes
 *     a class name and a python string, which are turned into an object
 *     using TBufferFile.
 *
 *  - Add a "__reduce__" method to ObjectProxy, which turns ObjectProxy's
 *     into python strings through serialization done by TBufferFile. The
 *     python string is subsequently pickled.
 */


#ifndef GAUDIMP_PYROOTPICKLE_H
#define GAUDIMP_PYROOTPICKLE_H

#include "Python.h"

#include "GaudiKernel/Kernel.h"

namespace GaudiMP {


class GAUDI_API PyROOTPickle
{
public:
  /**
   * @brief Install the pickling of ObjectProxy's functionality.
   * @param libpyroot_pymodule The libPyROOT python module
   * @param objectproxy_pytype The ObjectProxy python type
   */
  static void Initialize (PyObject* libpyroot_pymodule,
                          PyObject* objectproxy_pytype);
};


} // namespace GaudiMP


#endif // not GAUDIMP_PYROOTPICKLE_H
