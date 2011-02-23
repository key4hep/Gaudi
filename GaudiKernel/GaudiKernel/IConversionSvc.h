// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IConversionSvc.h,v 1.8 2006/01/26 09:11:14 mato Exp $
#ifndef GAUDIKERNEL_ICONVERSIONSVC_H
#define GAUDIKERNEL_ICONVERSIONSVC_H

// Include files
#include <string>

#include "GaudiKernel/IConverter.h"

// Forward declarations
class ICnvFactory;

/** @class IConversionSvc IConversionSvc.h GaudiKernel/IConversionSvc.h

    <P> The conversion service interface allows to:
    <UL>
    <LI> Add, get and remove data converters from the sercvice.
    <LI> Create objects using converters: e.g. create the transient
         representation of a persistent object.
    <LI> convert objects - the opposite of create: e.g. convert transient
         objects into the persistent representation of these objects.
         The objects to be converted are defined by a data selector object.
    <LI> update objects which already exist: e.g. update the existing
         persistent representation of objects which were read from the
	       persistent store and should be modified.
	       The objects to be converted are defined by a data selector object.
    <LI> Class specific aspects of the creation/update/conversion mechanism
         must be handled by the converters, the common aspects will be handled
         by the service.
    </UL>

   @author Markus Frank
   @version 1.2

  - Version 1.2 Remove createReps, updateReps - replaced by interface IConverter

*/
class GAUDI_API IConversionSvc: virtual public IConverter {
public:
  /// InterfaceID
  DeclareInterfaceID(IConversionSvc,4,0);

  /** Add converter object to conversion service.
   *  @param      pConverter Pointer to converter object
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode addConverter(IConverter* pConverter) = 0;

  /** Add converter object to conversion service.
   *  @param      clid   Class ID of the converter needed
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode addConverter(const CLID& clid) = 0;

  /** Remove converter object from conversion service (if present).
   *  The converter is defined by the class type of the objects created.
   *  @param      clid   Class ID of the converter
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode removeConverter(const CLID& clid) = 0;

  /** Retrieve converter from list
   *  @param clid the clid of the converter
   *  @return the converter corresponding to clid or 0 if non wasa found
   */
  virtual IConverter* converter(const CLID& clid) = 0;

  /** Connect the output file to the service.
   *  @param      outputFile  String containig output file
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode connectOutput(const std::string& outputFile) = 0;

  /** Connect the output file to the service with open mode.
   *  @param      outputFile  String containig output file
   *  @param      openMode    String containig opening mode of the output file
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode connectOutput(const std::string& outputFile,
                                   const std::string& openMode) = 0;

  /** Commit pending output.
   *  @param      outputFile  String containig output file
   *  @param      do_commit   if true commit the output and flush
   *                          eventually pending items to the database
   *                          if false, discard pending buffers.
   *                          Note: The possibility to commit or rollback
   *                          depends on the database technology used!
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode commitOutput(const std::string& outputFile,
                                  bool do_commit) = 0;

  /// Status code definitions
  enum Status  {
   /// Success
   CONVERSIONSVC_NO_ERROR = ICONVERSIONSVC_LAST_ERROR+1,
   /// No proper converter is availible to the service
   NO_CONVERTER
  };
};

#endif // GAUDIKERNEL_ICONVERSIONSVC_H
