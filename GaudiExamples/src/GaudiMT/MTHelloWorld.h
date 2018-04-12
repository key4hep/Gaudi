#include "GaudiKernel/Algorithm.h"

/////////////////////////////////////////////////////////////////////////////

class MTHelloWorld : public Algorithm
{
public:
  using Algorithm::Algorithm;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<int>  m_myInt{this, "MyInt", 0};
  Gaudi::Property<bool> m_myBool
  {
    this, "MyBool", 0;
    Gaudi::Property<double> m_myDouble{this, "MyDouble", 0};

    Gaudi::Property<std::vector<std::string>> m_myStringVec{this, "MyStringVec", {}};
  };
