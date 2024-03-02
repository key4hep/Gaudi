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
/*
 * PythonModule.cpp
 *
 *  Created on: Sep 1, 2010
 *      Author: marcocle
 */

/** @file PythonModule.cpp
 *
 *  Small Python module example based on Boost.Python tutorial
 *
 */

#include "Functions.h"

#include <boost/python.hpp>

BOOST_PYTHON_MODULE( PyExample ) {
  using namespace boost::python;
  def( "greet", greet );
}
