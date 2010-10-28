#define PYCONF_VALIDATOR_MODULE
#include "GaudiKernel/PyConfValidators.h"

BOOST_PYTHON_MODULE(test_validator)
{
  using namespace boost::python;
  def("SimpleProperty<int,BoundedVerifier<int> >",
      check<int,BoundedVerifier<int> >);
  def("SimpleProperty<bool,BoundedVerifier<bool> >",
      check<bool,BoundedVerifier<bool> >);
  def("SimplePropertyRef<std::vector<std::string,std::allocator<std::string> >,NullVerifier<std::vector<std::string,std::allocator<std::string> > > >",
      check<std::vector<std::string,std::allocator<std::string> >,NullVerifier<std::vector<std::string,std::allocator<std::string> > > >);
  def("SimplePropertyRef<std::string,NullVerifier<std::string> >",
      check<std::string,NullVerifier<std::string> >);
  def("SimplePropertyRef<std::vector<long,std::allocator<long> >,NullVerifier<std::vector<long,std::allocator<long> > > >",
      check<std::vector<long,std::allocator<long> >,NullVerifier<std::vector<long,std::allocator<long> > > >);
}
