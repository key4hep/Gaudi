#ifndef GAUDIKERNEL_INTUPLESVC_H
#define GAUDIKERNEL_INTUPLESVC_H

// Framework include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/ClassID.h"


// Forward declarations
namespace NTuple    {
  class Tuple;
  class Directory;
}

/** @class INTupleSvc INTupleSvc.h GaudiKernel/INTupleSvc.h

    <P>The ntuple service interface allows to:
    <UL>
    <LI> Export the data provider's functionality.
    <LI> Extend the functionality in order
         to book abstract N tuples:
         <UL>
         <LI> Book COLUMN WISE N tuples.
         <LI> Book ROW WISE N tuples.
         <LI> Access a persistent N tuple within a file
         <LI> Save the N tuple after filling.
         <LI> Write individual records from to a N tuple.
         <LI> Read individual records from an existing N tuple.
         </UL>
         created N tuples are automatically put onto the
         N tuple data store.
    </UL>

   @author Markus Frank
   @version 1.0
*/
class GAUDI_API INTupleSvc: virtual public IDataProviderSvc {
public:
  /// InterfaceID
  DeclareInterfaceID(INTupleSvc,2,0);
  /// Create requested N tuple (Hide constructor)
  virtual StatusCode create(const CLID& typ, const std::string& title, NTuple::Tuple*& refpTuple) = 0;
  /** Book Ntuple and register it with the data store.
      Connects the object identified by its full path to the parent object
      identified by the base name of the full path.
      @param      fullPath    Full path to the node of the object.
      @param      type        Class ID of the N tuple: Column or row wise.
      @param      title       Title property of the N tuple.
      @param      refpTuple   Reference to pointer to the N tuple to be booled and registered.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Tuple* book (const std::string& fullPath, const CLID& type, const std::string& title) = 0;
  /** Book Ntuple and register it with the data store.
      Connects the object identified by its relative path to the parent object
      identified by the base name.
      @param      dirPath     Path to parent node of the object.
      @param      relPath     Relative path to the object with respect to the parent node.
                              The relative path is the identifier of the tuple.
      @param      title       Title property of the N tuple.
      @param      type        Class ID of the N tuple: Column or row wise.
      @param      refpTuple   Reference to pointer to the N tuple to be booled and registered.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Tuple* book (const std::string& dirPath, const std::string& relPath, const CLID& type, const std::string& title) = 0;
  /** Book Ntuple and register it with the data store.
      Connects the object identified by an identifier to the parent object
      identified by the base name.
      @param      dirPath     Path to parent node of the object.
      @param      id          Identifier of the tuple within the parent's directory.
      @param      type        Class ID of the N tuple: Column or row wise.
      @param      title       Title property of the N tuple.
      @param      refpTuple   Reference to pointer to the N tuple to be booled and registered.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Tuple* book (const std::string& dirPath, long id, const CLID& type, const std::string& title) = 0;
  /** Book Ntuple and register it with the data store.
      Connects the object identified by an identifier (id) to the parent object
      identified by the parent's pointer.
      @param      pParent     Pointer to parent object.
      @param      relPath     Relative path to the object with respect to the parent node.
                              The relative path is the identifier of the tuple.
      @param      type        Class ID of the N tuple: Column or row wise.
      @param      title       Title property of the N tuple.
      @param      refpTuple   Reference to pointer to the N tuple to be booled and registered.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Tuple* book (DataObject* pParent, const std::string& relPath, const CLID& type, const std::string& title) = 0;
  /** Book Ntuple and register it with the data store.
      Connects the object identified by its relative path to the parent object
      identified by the parent's pointer.
      @param      pParent     Pointer to parent object.
      @param      id          Identifier of the tuple within the parent's directory.
      @param      type        Class ID of the N tuple: Column or row wise.
      @param      title       Title property of the N tuple.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Tuple* book (DataObject* pParent, long id, const CLID& type, const std::string& title) = 0;
  /** Create Ntuple directory and register it with the data store.
      Connects the object identified by its relative path to the parent object
      identified by the parent's pointer.
      @param      pParent     Pointer to parent object.
      @param      title       Directory identifier.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Directory* createDirectory (DataObject* pParent, const std::string& title) = 0;
  /** Create Ntuple directory and register it with the data store.
      Connects the object identified by its relative path to the parent object
      identified by the parent's pointer.
      @param      pParent     Pointer to parent object.
      @param      id          Identifier of the tuple within the parent's directory.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Directory* createDirectory (DataObject* pParent, long id) = 0;
  /** Create Ntuple directory and register it with the data store.
      Connects the object identified by its relative path to the parent object
      identified by the parent's pointer.
      @param      dirPath     Full directory path to parent
      @param      id          Identifier of the tuple within the parent's directory.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Directory* createDirectory (const std::string& dirPath, long id) = 0;
  /** Create Ntuple directory and register it with the data store.
      Connects the object identified by its path to the parent object
      identified by the parent's path.
      @param      dirPath     Full directory path to parent
      @param      title       Directory identifier.
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Directory* createDirectory (const std::string& dirPath, const std::string& title) = 0;
  /** Create Ntuple directory and register it with the data store.
      @param      fullPath    Full directory path
      @return                 Status code indicating success or failure.
  */
  virtual NTuple::Directory* createDirectory (const std::string& fullPath) = 0;
  /** Access N tuple on disk.
      @param    fullPath    Full path to the N tuple within the transient store
      @param    filename    Name of the file the ntuple resides in.
      @return               Status code indicating success or failure.
  */
  virtual NTuple::Tuple* access(const std::string& fullPath, const std::string& filename) = 0;

  /** Save N tuple to disk. Must be called in order to close the ntuple file properly
      @param    fullPath    Full path to the N tuple in memory.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode save(const std::string& fullPath) = 0;

  /** Save N tuple to disk. Must be called in order to close the ntuple file properly
      @param    tuple       Pointer to the Ntuple in memory
      @return               Status code indicating success or failure.
  */
  virtual StatusCode save(NTuple::Tuple* tuple) = 0;

  /** Save N tuple to disk. Must be called in order to close the ntuple file properly
      @param    pParent     Parent object of the N tuple
      @param    relPath     Relative path to the N tuple in memory with respect to the
                            parent object.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode save(DataObject* pParent, const std::string& relPath) = 0;

  /** Write single record to N tuple.
      @param    tuple       Pointer to the Ntuple in memory
      @return               Status code indicating success or failure.
  */
  virtual StatusCode writeRecord( NTuple::Tuple* tuple) = 0;

  /** Write single record to N tuple.
      @param    fullPath    Full path to the N tuple in memory.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode writeRecord(const std::string& fullPath) = 0;

  /** Write single record to N tuple.
      @param    pParent     Parent object of the N tuple
      @param    relPath     Relative path to the N tuple in memory with respect to the
                            parent object.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode writeRecord( DataObject* pParent, const std::string& relPath) = 0;

  /** Read single record from N tuple.
      @param    tuple       Pointer to the Ntuple in memory
      @return               Status code indicating success or failure.
  */
  virtual StatusCode readRecord(NTuple::Tuple* tuple) = 0;

  /** Read single record from N tuple.
      @param    fullPath    Full path to the N tuple in memory.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode readRecord(const std::string& fullPath) = 0;

  /** Read single record from N tuple.
      @param    pParent     Parent object of the N tuple
      @param    relPath     Relative path to the N tuple in memory with respect to the
                            parent object.
      @return               Status code indicating success or failure.
  */
  virtual StatusCode readRecord(DataObject* pParent, const std::string& relPath) = 0;

};

#endif // INTERFACES_INTUPLESVC_H
