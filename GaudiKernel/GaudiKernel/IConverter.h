#ifndef GAUDIKERNEL_ICONVERTER_H
#define GAUDIKERNEL_ICONVERTER_H

// Include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ClassID.h"

// Forward declarations
class IDataProviderSvc;
class IAddressCreator;
class IConversionSvc;
class IOpaqueAddress;
class DataObject;
template<class T> class SmartIF;

/** @class IConverter IConverter.h GaudiKernel/IConverter.h

    The data converters are responsible to translate data from one
    representation into another. Concrete examples are e.g. converters
    creating transient objects representing parts of an event from
    the persistent (and disk based) representations. Converters will
    have to deal with the technology both representations are based on:
  	in the upper example they have to know about the database internals
	  as well as the structure of the transient representations.
    The converters know about the mechanism to retrieve persistent objects
	  (ZEBRA, Objectivity, ) and only pass abstract instances of the converted
	  objects, hence shielding the calling service from internals.

    Data converters are meant to be light. This means there should
    not be all-in-one converters, which are able to convert the "world",
    but rather many converters. Each converter is then able to create a
    representation of a given type.

    In order to function a converter must be able to
    <UL>
    <LI>Answer (when asked) which kind of representation the converter
    is able to create.
    <LI>Retrieve the source object from the source store.
    <LI>Create the requested representation using the information contained
    in the source object.
    <LI>Inform the registry entry of  the created object that the object
    is now loaded.
    <LI>This registry entry is located in the data store which is supposed
    to manage the requested object.
    <LI>Register all leafs of the created object with the data store which
    is supposed to manage the requested object.
    Registering does not mean to create these representations,
    but rather to inform about the existence.
    </UL>

    The interface should cover the entry points of concrete
    converter instances in order to serve conversion requests.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API IConverter: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IConverter,3,0);

  /** Initialize the converter
      @return    Status code indicating success or failure
      @exception exception (STL standard) in case of fatal errors
  */
  virtual StatusCode initialize() = 0;

  /** Terminate the converter
      @return    Status code indicating success or failure
      @exception exception (STL standard) in case of fatal errors
  */
  virtual StatusCode finalize() = 0;

  /** Retrieve the class type of objects the converter produces.
      @return Class type of the created object.
  */
  virtual const CLID& objType() const = 0;

  /** Retrieve the class type of the data store the converter uses.
      @return Class type information about the source data store type the
              converter will use to retrieve the information.
   */
  virtual long repSvcType() const = 0;

  /** Set Data provider service
      @return    Status code indicating success or failure
      @param     pService   Pointer to data provider service
  */
  virtual StatusCode setDataProvider(IDataProviderSvc* pService) = 0;

  /** Get Data provider service
      @return    Pointer to data provider service
  */
  virtual SmartIF<IDataProviderSvc>& dataProvider() const = 0;

  /** Set conversion service the converter is connected to
      @return    Status code indicating success or failure
      @param     pService   Pointer to IConversionSvc interface
  */
  virtual StatusCode setConversionSvc(IConversionSvc* pService) = 0;

  /** Get conversion service the converter is connected to
      @return    Pointer to IConversionSvc interface
  */
  virtual SmartIF<IConversionSvc>& conversionSvc()    const = 0;

  /** Set address creator facility.
      @param      creator     Reference to address creator interface
      @return     Status code indicating success or failure.
  */
  virtual StatusCode setAddressCreator(IAddressCreator* creator) = 0;

  /** Get address creation interface needed to resolve links between objects
      @return    Pointer to IAddressCreator interface
  */
  virtual SmartIF<IAddressCreator>& addressCreator()    const = 0;

  /** Create the transient representation of an object.
      The transient representation is created by loading the
      persistent object using the source information
      contained in the address.
      @return    Status code indicating success or failure
      @param     pAddress   Opaque address information to retrieve the
                            requested object from the store in order to
			    produce the transient object.
      @param     refpObject Reference to location of pointer of the
                            created object.
   */
  virtual StatusCode createObj(IOpaqueAddress* pAddress, DataObject*& refpObject) = 0;

  /** Resolve the references of the created transient object.
      After the object creation references of the objects pointing
      to objects outside its scope will have to be filled. The actual
      objects will not be loaded, but the recipe to load them will be present.
      @return    Status code indicating success or failure
      @param     pAddress   Opaque address information to retrieve the
                            requested object from the store.
      @param     pObject    Pointer to location of the object
   */
  virtual StatusCode fillObjRefs(IOpaqueAddress* pAddress, DataObject* pObject) = 0;

  /** Update the transient object from the other representation.
      The transient representation will be updated by loading the
      persistent object using the source information
      contained in the address and then refill transient data.
      @return    Status code indicating success or failure
      @param     pAddress   Opaque address information to retrieve the
                            requested object from the store.
      @param     pObject    Pointer to the object to be updated.
   */
  virtual StatusCode updateObj(IOpaqueAddress* pAddress, DataObject* refpObject) = 0;

  /** Update the references of an updated transient object.
      After the object was updated also the references of the objects pointing
      to objects outside its scope will have to be filled. The actual pointers
      to objects will only be filled if already present. Otherwise
      only the recipe to load them will be present.
      @return    Status code indicating success or failure
      @param     pAddress   Opaque address information to retrieve the
                            requested object from the store.
      @param     pObject    Pointer to location of the object
   */
  virtual StatusCode updateObjRefs(IOpaqueAddress* pAddress, DataObject* pObject) = 0;

  /** Convert the transient object to the requested representation.
      e.g. conversion to persistent objects.
      @return    Status code indicating success or failure
      @param     pObject     Pointer to location of the object
      @param     refpAddress Reference to location of pointer with the
                             object address.
   */
  virtual StatusCode createRep(DataObject* pObject, IOpaqueAddress*& refpAddress) = 0;

  /** Resolve the references of the converted object.
      After the requested representation was created the references in this
      representation must be resolved.
      @return    Status code indicating success or failure
      @param     pObject    Pointer to location of the object
   */
  virtual StatusCode fillRepRefs(IOpaqueAddress* pAddress, DataObject* pObject) = 0;

  /** Update the converted representation of a transient object.
      @return    Status code indicating success or failure
      @param     pObject     Pointer to location of the object
      @param     refpAddress Reference to location of pointer with the
                             object address.
   */
  virtual StatusCode updateRep(IOpaqueAddress* pAddress, DataObject* pObject)  = 0;

  /** Update the references of an already converted object.
      The object must be retrieved before it can be updated.
      @return    Status code indicating success or failure
      @param     pAddress   Pointer to location of object address with the
                            store.
      @param     pObject    Pointer to location of the object
   */
  virtual StatusCode updateRepRefs(IOpaqueAddress* pAddress, DataObject* pObject) = 0;

  /// Virtual destructor
  virtual ~IConverter() {}

  /// Status code
  enum Status {
    /// Invalid address information
    INVALID_ADDRESS = IInterface::LAST_ERROR+1,
    /// Object to be converted is invalid
    INVALID_OBJECT,
    /// No more memory available
    NO_MEMORY,
    /// Invalid storage type
    BAD_STORAGE_TYPE,
    /// Error retrieving source data from source store
    NO_SOURCE_OBJECT,
    /// Last entry
    ICONVERSIONSVC_LAST_ERROR
  };
};

#endif // GAUDIKERNEL_ICONVERTER_H
