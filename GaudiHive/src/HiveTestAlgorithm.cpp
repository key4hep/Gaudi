#include "GaudiHive/HiveTestAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

#include <atomic>

DECLARE_ALGORITHM_FACTORY(HiveTestAlgorithm)

using namespace std;

class MyObject : public DataObject {
  int m_data;
  static atomic<int> c_instances;
  static atomic<int> d_instances;
public:
  MyObject(int d) : m_data(d) {c_instances++;}
  MyObject(const MyObject& o) : DataObject(), m_data(o.m_data) {c_instances++;}
  ~MyObject() {d_instances++;}
  int getData() { return m_data; }
  static void dump() { cout << "MyObject (C/D): " << c_instances << "/" << d_instances <<endl;}
};

atomic<int> MyObject::c_instances;
atomic<int> MyObject::d_instances;

/**
 ** Constructor(s)
 **/
HiveTestAlgorithm::HiveTestAlgorithm(const std::string& name, ISvcLocator* pSvcLocator) :
  GaudiAlgorithm(name, pSvcLocator),
  m_total( 0 ),
  m_inputs( 0 ),
  m_outputs( 0 )
{
  declareProperty("Inputs", m_inputs, "List of required inputs");
  declareProperty("Outputs", m_outputs, "List of provided outputs");
}

/**
 ** Destructor
 **/
HiveTestAlgorithm::~HiveTestAlgorithm( )
{
}

StatusCode
HiveTestAlgorithm::initialize()
{
  info() << ":HiveTestAlgorithm::initialize " << endmsg;
  
  return StatusCode::SUCCESS;
}

StatusCode
HiveTestAlgorithm::execute()
{
  ++m_total;
  int evt = getContext()->m_evt_num;
  
  info() << ":HiveTestAlgorithm::getting inputs... " << evt << endmsg;
  
  for(vector<string>::iterator i = m_inputs.begin(); i != m_inputs.end(); i++) {
    MyObject* obj = get<MyObject>(*i);
    info() << "Got data " << *i << " with value " << obj->getData() << endmsg;
  }
  
  info() << ":HiveTestAlgorithm::registering outputs... " << evt << endmsg;

  for(vector<string>::iterator i = m_outputs.begin(); i != m_outputs.end(); i++) {
    put(new MyObject(1000+evt), *i);
  }

  return StatusCode::SUCCESS;
}

StatusCode
HiveTestAlgorithm::finalize()
{
  info() << name( ) << ":HiveTestAlgorithm::finalize - total events: " << m_total << endmsg;
  MyObject::dump();
  return StatusCode::SUCCESS;
}

const std::vector<std::string> HiveTestAlgorithm::get_inputs()
{
  return m_inputs;
}

const std::vector<std::string> HiveTestAlgorithm::get_outputs()
{
  return m_outputs;
}


