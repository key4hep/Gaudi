/** Examples and tests of use of IGenericThreadSafeTool interface.
 */
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiAlg/IGenericTool.h"

namespace GenericToolTests {
  struct MyGenericTool: public extends<AlgTool, IGenericTool> {
    using extends::extends;

    void execute() override {
      info() << "MyGenericTool::execute()" << endmsg;
    }
  };
  DECLARE_COMPONENT(MyGenericTool)

  struct MyGenericTSTool: public extends<AlgTool, IGenericThreadSafeTool> {
    using extends::extends;
    using IGenericThreadSafeTool::execute;

    void execute() const override {
      info() << "MyGenericTSTool::execute() const" << endmsg;
    }
  };
  DECLARE_COMPONENT(MyGenericTSTool)

  struct Algorithm: public ::Algorithm {
    using ::Algorithm::Algorithm;

    StatusCode initialize() override {
      auto sc = ::Algorithm::initialize();
      if ( !sc ) return sc;
      info() << "getting the tools..." << endmsg;
      sc = m_gt1.retrieve() &&
           m_gt2a.retrieve() &&
           m_gt2b.retrieve();
      return sc;
    }
    StatusCode execute() override {
      info() << "executing..." << endmsg;
      m_gt1->execute();
      m_gt2a->execute();
      m_gt2b->execute();
      i_constMethod();
      info() << "...done" << endmsg;
      return StatusCode::SUCCESS;
    }
    StatusCode finalize() override {
      m_gt1.release().ignore();
      m_gt2a.release().ignore();
      m_gt2b.release().ignore();
      return ::Algorithm::finalize();
    }
  private:
    ToolHandle<IGenericTool> m_gt1{"GenericToolTests::MyGenericTool/Generic", this};
    ToolHandle<IGenericTool> m_gt2a{"GenericToolTests::MyGenericTSTool/GenericTS", this};
    ToolHandle<IGenericThreadSafeTool> m_gt2b{"GenericToolTests::MyGenericTSTool/GenericTS", this};

    void i_constMethod() const {
      info() << "from const method..." << endmsg;
      // m_gt1->execute(); // not allowed
      // m_gt2a->execute(); // not allowed
      m_gt2b->execute(); // not allowed
    }
  };
  DECLARE_COMPONENT(Algorithm)
}
