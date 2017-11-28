//====================================================================
//  Tag collection service definition
//--------------------------------------------------------------------
//
//  Package    : GaudiSvc ( The LHCb Offline System)
//  Author     : M.Frank
//====================================================================
#ifndef GAUDI_TAGCOLLECTIONSVC_H
#define GAUDI_TAGCOLLECTIONSVC_H 1

// Framework include files
#include "NTupleSvc.h"

/** Gaudi tag collection service definition

    Description:
    See the base class definition files for more detailed documentation
    of the implementing class.
    The basic functionality of the implementation is enhanced to
    allow for more general N-tuples, which can be used also
    as event tag collections.

    @Author  M.Frank
    @Version 1.0
*/
class TagCollectionSvc : public NTupleSvc
{
public:
  /// unhides some NTupleSvc methods
  using NTupleSvc::connect;

  /// Standard Constructor
  TagCollectionSvc( const std::string& name, ISvcLocator* svc );
  /// Add file to list I/O list
protected:
  StatusCode connect( const std::string& ident, std::string& logname ) override;
  /** Create conversion service.
      @param nam     Name of the service to be created
      @param svc     Service type if sepecified in the options
      @param props
  */
  virtual StatusCode createService( const std::string& nam, const std::string& typ, const std::vector<Prop>& props,
                                    IConversionSvc*& pSvc );
};

#endif // GAUDI_TAGCOLLECTIONSVC_H
