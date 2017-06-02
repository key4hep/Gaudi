
// The following two classes are the interfaces to facilitate reading POOL formatted ROOT files:
#include <string>
#include <utility>
#include <vector>
#include "GaudiKernel/Kernel.h" // GAUDI_API

/*
 *   POOL namespace declaration
 */
namespace pool  {
  class Guid {          // size is 16
  public:
    unsigned int  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
  };
  /** @class Token
   *
   *  Shadow class to mimik POOL tokens.
   *
   */
  class Token {
  public:
    int                   m_refCount;    //! transient (a streamer is used to read it)
    int                   m_technology;  //! transient (a streamer is used to read it)
    std::string           m_dbID;        //! transient (a streamer is used to read it)
    std::string           m_cntID;       //! transient (a streamer is used to read it)
    Guid                  m_classID;     //! Object global identifier
    std::pair<int, int> m_oid;           //! POOL OID data member: transient (a streamer is used to read it)
    int m_type;                          //! transient (a streamer is used to read it)
  public:
    Token() = default;
    virtual ~Token() = default;
    bool operator==(const Token& t) const { return m_oid.first==t.m_oid.first && m_oid.second==t.m_oid.second;}
  };
}

  /** @class UCharDbArray
   *
   *  Shadow class to mimik POOL blobs.
   *
   */
struct UCharDbArray {
  public:
  /// Size of buffer
  int    m_size = 0;
  /// Buffer with object content
  unsigned char *m_buffer = nullptr;//[m_size]
  /// Default constructor
  UCharDbArray() = default;
  /// Standard destructor
  virtual ~UCharDbArray() {
    if ( m_buffer ) delete [] m_buffer;
    m_buffer = nullptr;
  }
};

/** @class PoolDbTokenWrap PoolDbTokenWrap.h PoolDb/PoolDbTokenWrap.h
  *
  * Description:
  *
  * @author  M.Frank
  * @version 1.0
  */
class PoolDbTokenWrap {
 public:
  /// Aggregated token object
  pool::Token token;
  /// Standard constructor
  PoolDbTokenWrap() = default;
  /// Copy constructor
  PoolDbTokenWrap(const PoolDbTokenWrap& wrp)  {    token = wrp.token;  }
  /// Standard destructor
  virtual ~PoolDbTokenWrap() = default;
  /// Equality operator
  bool operator==(const PoolDbTokenWrap& c) const {    return token == c.token;  }
  /// Assignment operator
  PoolDbTokenWrap& operator=(const PoolDbTokenWrap& wrp)  {
    if ( this != &wrp )  token = wrp.token;
    return *this;
  }
};

/** @class PoolDbLinkManager PoolDbLinkManager.h GaudiPoolDb/PoolDbLinkManager.h
  *
  * Description:
  * PoolDbLinkManager class implementation definition.
  *
  * @author Markus Frank
  * @version 1.0
  */
class GAUDI_API PoolDbLinkManager {
protected:
  /// Directory links
  std::vector<pool::Token*> m_refs;
  /// Logical links
  std::vector<std::string>  m_links;

public:
  /// Standard constructor
  PoolDbLinkManager() = default;
  /// Standard destructor
  virtual ~PoolDbLinkManager() {
    for(auto& i : m_refs ) delete i;
  }
  /// Access to token array
  std::vector<pool::Token*>& references()  {    return m_refs;   }
  /// Access to link array
  std::vector<std::string>& links()        {    return m_links;  }
};
