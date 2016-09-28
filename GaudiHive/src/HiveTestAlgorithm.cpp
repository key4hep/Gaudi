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
  declareProperty("Input", m_inputs, "List of required inputs");
  declareProperty("Output", m_outputs, "List of provided outputs");
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

  int i=0;
  for (auto k: m_inputs) {
    debug() << "adding input key " << k << endmsg;
    m_inputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Reader, this ));
    declareProperty("dummy_in_" + std::to_string(i), *(m_inputHandles.back()) );
    i++;
  }

  i = 0;
  for (auto k: m_outputs) {
    debug() << "adding output key " << k << endmsg;
    m_outputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ));
    declareProperty("dummy_out_" + std::to_string(i), *(m_outputHandles.back()) );
    i++;
  }
  
  return StatusCode::SUCCESS;
}

StatusCode
HiveTestAlgorithm::execute()
{
  ++m_total;
  int evt = getContext()->evt();
  
  info() << ":HiveTestAlgorithm::getting inputs... " << evt << endmsg;
  
  for(auto& handle : m_inputHandles) {
    auto obj = dynamic_cast<MyObject*>(handle->get());
    info() << "Got data with value " << obj->getData() << endmsg;
  }
  
  info() << ":HiveTestAlgorithm::registering outputs... " << evt << endmsg;

  for (auto & outputHandle: m_outputHandles){ outputHandle->put(new MyObject(1000+evt)); }

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


