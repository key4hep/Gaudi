
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/RndmGenerators.h"

//------------------------------------------------------------------------------

/** @class ViewTester
 *
 *
 */
namespace Test
{

  class ViewTester : public GaudiAlgorithm
  {

  public:
    bool isClonable() const override { return true; }

    /// the execution of the algorithm
    StatusCode execute() override;
    /// Its initialization
    StatusCode initialize() override;
    /// the finalization of the algorithm
    StatusCode finalize() override;

    ViewTester( const std::string& name,   // the algorithm instance name
                ISvcLocator*       pSvc ); // the Service Locator

  private:
    /// the default constructor is disabled
    ViewTester(); // no default constructor
    /// the copy constructor is disabled
    ViewTester( const ViewTester& ); // no copy constructor
    /// the assignement operator is disabled
    ViewTester& operator=( const ViewTester& ); // no assignement

    Gaudi::Property<std::vector<std::string>>                  m_inpKeys{this, "inpKeys", {}, ""};
    Gaudi::Property<std::vector<std::string>>                  m_outKeys{this, "outKeys", {}, ""};
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputHandles;
    std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputHandles;

    // View config
    Gaudi::Property<std::string> m_baseViewName{this, "baseViewName", "view",
                                                "Views to be named this, plus a numerical index"};
    Gaudi::Property<unsigned int> m_viewNumber{this, "viewNumber", 0, "How many views to make"};
    Gaudi::Property<std::string>  m_viewNodeName{this, "viewNodeName", "viewNode", "Name of node to attach views to"};
  };
}
