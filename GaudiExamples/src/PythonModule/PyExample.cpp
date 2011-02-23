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

BOOST_PYTHON_MODULE(PyExample)
{
    using namespace boost::python;
    def("greet", greet);
}
