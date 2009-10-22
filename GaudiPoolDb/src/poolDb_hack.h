#ifndef POOLDB_HACK_H_
#define POOLDB_HACK_H_
#include "StorageSvc/poolDb.h"
#include "StorageSvc/DbInstanceCount.h"
namespace poolDb { void genMD5(const std::string& s, void* code); }
namespace pool {
  using poolDb::INVALID;
  using poolDb::AccessMode;
  using poolDb::DbInstanceCount;
  using poolDb::releasePtr;
  using poolDb::deletePtr;
  using poolDb::Success;
  using poolDb::Error;
  using poolDb::debugBreak;
  using poolDb::READ;
  using poolDb::RECREATE;
  using poolDb::CREATE;
  using poolDb::UPDATE;
  using poolDb::accessMode;
  using poolDb::NOT_OPEN;
  inline void genMD5(const std::string& s, void* code) {
    poolDb::genMD5(s,code);
  }

}
#endif /*POOLDB_HACK_H_*/
