#include "GaudiKernel/Kernel.h"
extern "C" GAUDI_IMPORT int GaudiMain(int argc, char* argv[]);

int main(int argc, char* argv[] )   {
  return GaudiMain(argc, argv);
}
