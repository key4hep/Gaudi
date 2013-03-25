#include "PythonConfig.h"

// boost includes
#include "boost/python.hpp"
using namespace boost::python;

//-----------------------------------------------------------------------------
StatusCode PythonConfig::evaluateConfig(const std::string& fileName, const std::string& pythonAction) {
  try{
   // Prepare a Python environment 
   Py_Initialize();
   object main_module((
   handle<>(borrowed(PyImport_AddModule("__main__")))));
   object main_namespace = main_module.attr("__dict__");
 
   // make the translator class known to Python 
   main_namespace["PythonAdaptor"] = class_<PythonAdaptor>("PythonAdaptor", boost::python::init<IJobOptionsSvc*>())
	 .def("addPropertyToJobOptions",&PythonAdaptor::addPropertyToJobOptions);
   
       // create an instance of it and pass it to Python
       PythonAdaptor adaptor(m_IJobOptionsSvc); 
       main_namespace["adaptor"] = ptr(&adaptor);      

       // some python helper
       std::string command("execfile('");
       command += fileName + "')\n";
       command += "from GaudiKernel.Configurable import expandvars\nfrom GaudiKernel.Proxy.Configurable import applyConfigurableUsers\napplyConfigurableUsers()\n";
       command += pythonAction; 
       command += "for n, c in Configurable.allConfigurables.iteritems():\n  for p, v in  c.getValuedProperties().items() :\n    v = expandvars(v)\n    if   type(v) == str : v = '\"%s\"' % v # need double quotes\n    elif type(v) == long: v = '%d'   % v # prevent pending 'L'\n    adaptor.addPropertyToJobOptions(n,p,str(v))";

       // Now fire off the translation
       handle<> ignored(( PyRun_String( command.c_str(),
                                        Py_file_input,
                                        main_namespace.ptr(),
                                        main_namespace.ptr() ) ));
  } catch( error_already_set ) {
       PyErr_Print();
       return StatusCode::FAILURE;
  }

return StatusCode::SUCCESS;
}
