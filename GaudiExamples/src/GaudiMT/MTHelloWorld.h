#include "GaudiKernel/Algorithm.h"

/////////////////////////////////////////////////////////////////////////////

class MTHelloWorld:public Algorithm {
public:
  using Algorithm::Algorithm;
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:

  IntegerProperty  m_myInt {this, "MyInt",  0};
  BooleanProperty  m_myBool {this, "MyBool",  0;
  DoubleProperty  m_myDouble {this, "MyDouble",  0};

  StringArrayProperty  m_myStringVec {this, "MyStringVec",  {}};
};
