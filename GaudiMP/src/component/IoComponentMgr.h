///////////////////////// -*- C++ -*- /////////////////////////////
// IoComponentMgr.h 
// Header file for class IoComponentMgr
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef GAUDIMP_IOCOMPONENTMGR_H
#define GAUDIMP_IOCOMPONENTMGR_H 1

// Python includes
#include <Python.h>

// STL includes
#include <string>
#include <map>
#include <list>

// FrameWork includes
#include "GaudiKernel/Service.h"

// GaudiKernel
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IIncidentListener.h"

// Forward declaration
class ISvcLocator;
template <class TYPE> class SvcFactory;


class IoComponentMgr: public extends2<Service, IIoComponentMgr,
		      IIncidentListener> {

  friend class SvcFactory<IoComponentMgr>;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  IoComponentMgr( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~IoComponentMgr(); 

  // Assignment operator: 
  //IoComponentMgr &operator=(const IoComponentMgr &alg); 

  /// Gaudi Service Implementation
  //@{
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  //@}

  void handle(const Incident&);

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** @brief: check if the registry contains a given @c IIoComponent
   */
  virtual
  bool io_hasitem (IIoComponent* iocomponent) const;

  /** @brief: check if the registry contains a given @c IIoComponent and
   *          that component had @param `fname` as a filename
   */
  virtual
  bool io_contains (IIoComponent* iocomponent,
		    const std::string& fname) const;

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent);

  /** @brief: allow a @c IIoComponent to register itself with this
   *          manager so appropriate actions can be taken when e.g.
   *          a @c fork(2) has been issued (this is usually handled
   *          by calling @c IIoComponent::io_reinit on every registered
   *          component)
   */
  virtual
  StatusCode io_register (IIoComponent* iocomponent,
			  IIoComponentMgr::IoMode::Type iomode,
			  const std::string& fname,
			  const std::string& pfn);

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new file name
   */
  virtual
  StatusCode io_update (IIoComponent* iocomponent,
			const std::string& old_fname,
			const std::string& new_fname);

  /** @brief: allow a @c IIoComponent to update the contents of the
   *          registry with a new work directory
   */
  virtual
  StatusCode io_update (IIoComponent* iocomponent,
			const std::string& work_dir);

  // VT. new method
  /** @brief: Update all @c IIoComponents with a new work directory
   */
  virtual
  StatusCode io_update_all (const std::string& work_dir);

  /** @brief: retrieve the new filename for a given @c IIoComponent and
   *          @param `fname` filename
   */
  virtual
  StatusCode io_retrieve (IIoComponent* iocomponent,
			  std::string& fname);

  /** @brief: reinitialize the I/O subsystem.
   *  This effectively calls @c IIoComponent::io_reinit on all the registered
   *  @c IIoComponent.
   */
  virtual
  StatusCode io_reinitialize ();

  /** @brief: finalize the I/O subsystem.
   *  Hook to allow to e.g. give a chance to I/O subsystems to merge output
   *  files. Not sure how to do this correctly though...
   */
  virtual
  StatusCode io_finalize ();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  struct IoComponentEntry {
    std::string m_oldfname;
    std::string m_oldabspath; // VT. store absolute path
    std::string m_newfname;
    IIoComponentMgr::IoMode::Type m_iomode;

    IoComponentEntry():m_oldfname(""),m_oldabspath(""),m_newfname(""), // VT. changes
		       m_iomode(IIoComponentMgr::IoMode::INVALID) {}
    IoComponentEntry(const std::string& f, const std::string& p,       // VT. changes
		     const IIoComponentMgr::IoMode::Type& t)
      : m_oldfname(f), m_oldabspath(p),m_newfname(""), m_iomode(t){}   // VT. changes
    IoComponentEntry(const IoComponentEntry& rhs):m_oldfname(rhs.m_oldfname),
						  m_oldabspath(rhs.m_oldabspath), // VT. changes
						  m_newfname(rhs.m_newfname),
						  m_iomode(rhs.m_iomode){}
    bool operator < (IoComponentEntry const &rhs) const {
      if (m_oldfname == rhs.m_oldfname) {
	return (m_iomode < rhs.m_iomode);
      } else {
	return (m_oldfname < rhs.m_oldfname);
      }
    }

    friend std::ostream& operator<< ( std::ostream& os, const IoComponentEntry& c) {
      os << "old: \"" << c.m_oldfname 
	 << "\"  absolute path: \"" << c.m_oldabspath
	 << "\"  new: \"" << c.m_newfname
	 << "\"  m: " << ( (c.m_iomode == IIoComponentMgr::IoMode::READ) ? 
			 "R" : "W" );
      return os;
    }

  };



  /// Default constructor: 
  IoComponentMgr();

  mutable MsgStream m_log;

  typedef std::map<std::string, IIoComponent*> IoRegistry_t;
  /// Registry of @c IIoComponents
  IoRegistry_t m_ioregistry;

  typedef std::list<IIoComponent*> IoStack_t;
  /// Stack of @c IIoComponents to properly handle order of registration
  IoStack_t m_iostack;


  // This is the registry
  typedef std::multimap<IIoComponent*, IoComponentEntry > IoDict_t;
  typedef IoDict_t::const_iterator iodITR;

  IoDict_t m_cdict;

  /// location of the python dictionary
  std::string m_dict_location;

  bool findComp(IIoComponent*, const std::string&, iodITR& ) const;
  bool findComp(IIoComponent*, std::pair<iodITR,iodITR>& ) const;
  bool findComp(const std::string&, std::pair<iodITR,iodITR>& ) const;

  std::string list() const;


}; 


#endif //> !GAUDIMP_IOCOMPONENTMGR_H
