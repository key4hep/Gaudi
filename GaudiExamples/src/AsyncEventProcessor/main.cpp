/** @file Example of implementation of a Gaudi executable that uses a non default application steering, in this case
 *  Gaudi::Examples::AsyncApplication.
 */

#include <Gaudi/Application.h>

int main() {
  std::string_view appType{"Gaudi::Examples::AsyncApplication"};

  Gaudi::Application::Options opts{
      {"ApplicationMgr.JobOptionsType", "\"NONE\""},
      {"ApplicationMgr.EventLoop", "\"Gaudi::Examples::AsyncEventLoopMgr/AsyncEventLoopMgr\""},
      {"ApplicationMgr.OutputLevel", "3"},
      {"ApplicationMgr.TopAlg", "['GaudiTesting::SleepyAlg/Alg1']"},
      {"Alg1.SleepTime", "1"},
      {"AsyncEventLoopMgr.OutputLevel", "2"}};

  auto app = Gaudi::Application::create( appType, std::move( opts ) );

  if ( !app ) {
    std::cerr << "error: could not instantiate " << appType << '\n';
    return 1;
  }
  return app->run();
}
