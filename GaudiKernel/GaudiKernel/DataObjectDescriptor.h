#ifndef GAUDIKERNEL_DATAOBJECTDESCRIPTOR_H
#define GAUDIKERNEL_DATAOBJECTDESCRIPTOR_H

#include <GaudiKernel/MinimalDataObjectHandle.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Property.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IAlgTool.h>

#include <map>
#include <memory>
#include <functional>

#include <boost/iterator/transform_iterator.hpp>

template <class T>
class DataObjectHandle;

class GAUDI_API DataObjectDescriptor{

public:
  static const std::string NULL_;

public:

  DataObjectDescriptor(const std::string& tag,
      const std::string& address,
      const bool optional = false,
      const MinimalDataObjectHandle::AccessType accessType = MinimalDataObjectHandle::READ)
: m_tag(tag), m_address(address), m_optional(optional), m_accessType(accessType) {};

  DataObjectDescriptor(const std::string& tag,
      const std::vector<std::string>& addresses,
      const bool optional = false,
      const MinimalDataObjectHandle::AccessType accessType = MinimalDataObjectHandle::READ)
  : m_tag(tag), m_address(addresses[0]), m_optional(optional), m_accessType(accessType) {

    setAltAddresses(addresses, true); //
  };

private:
  DataObjectDescriptor(const DataObjectDescriptor & value);

  DataObjectDescriptor & operator=(const DataObjectDescriptor& value);

public:

  MinimalDataObjectHandle::AccessType accessType() const;
  void setAccessType(MinimalDataObjectHandle::AccessType accessType);

  bool optional() const;
  void setOptional(bool optional);

  const std::string& address() const;
  const std::vector<std::string> & alternativeAddresses() const;

  //sets the address of the descriptor and updates the handle if present
  void setAddress(const std::string& address);
  void setAddresses(const std::vector<std::string>& address);
  void setAltAddresses(const std::vector<std::string> & addresses){
    setAltAddresses(addresses, false);
  }

  const std::string& tag() const;
  void setTag(const std::string& tag);

  static std::string tag(const std::string & s);

  const std::string toString() const;
  void fromString(const std::string & s);

  const std::string pythonRepr() const;

  bool valid() const {
    return address() != NULL_;
  }

private:
  void setAltAddresses(const std::vector<std::string> & addresses, bool skipFirst);

private:
  std::string m_tag;
  std::string m_address;
  std::vector<std::string> m_altAddresses;
  bool m_optional;
  MinimalDataObjectHandle::AccessType m_accessType;

};

class GAUDI_API DataObjectDescriptorCollection {
public:
  using map_type = std::map<std::string, MinimalDataObjectHandle *>;

  struct get_key: std::unary_function<const map_type::value_type &, const map_type::key_type&> {
    inline result_type operator()(argument_type p) const {
      return p.first;
    }
  };

  using iterator = boost::transform_iterator<get_key, map_type::iterator>;
  using const_iterator = boost::transform_iterator<get_key, map_type::const_iterator>;

  DataObjectDescriptorCollection() { }
  DataObjectDescriptorCollection(const DataObjectDescriptorCollection & other);
  DataObjectDescriptorCollection & operator=(const DataObjectDescriptorCollection& other);

  bool contains(const std::string & o) const;
  bool contains(const MinimalDataObjectHandle * o) const;

  bool insert(const std::string& tag, MinimalDataObjectHandle * descriptor);

  bool insert(MinimalDataObjectHandle * item);
  bool update(MinimalDataObjectHandle * item);

  bool update(const std::string & item);

  inline iterator begin() {
    return iterator(m_dataItems.begin(), get_key());
  }

  inline iterator end() {
    return iterator(m_dataItems.end(), get_key());
  }

  inline const_iterator begin() const {
    return const_iterator(m_dataItems.begin(), get_key());
  }

  inline const_iterator end() const {
    return const_iterator(m_dataItems.end(), get_key());
  }

  const MinimalDataObjectHandle & operator[](const std::string & tag) const;
  MinimalDataObjectHandle & operator[](const std::string & tag);

  const std::string toString() const;
  const std::string pythonRepr() const;

private:
  map_type m_dataItems;
};


class GAUDI_API DataObjectDescriptorProperty: public Property {
public:
  DataObjectDescriptorProperty(const std::string &name, DataObjectDescriptor & item);

  DataObjectDescriptorProperty(DataObjectDescriptorProperty & other);

  DataObjectDescriptorProperty(const DataObjectDescriptorProperty & other);

  DataObjectDescriptorProperty& operator=(DataObjectDescriptorProperty& value);

  DataObjectDescriptorProperty& operator=(const DataObjectDescriptorProperty& value);

  virtual DataObjectDescriptorProperty* clone() const;

  virtual bool load(Property& destination) const;

  virtual bool assign(const Property& source);

  virtual std::string toString() const;

  virtual void toStream(std::ostream& out) const;

  virtual StatusCode fromString(const std::string& s);

  virtual const DataObjectDescriptor& value() const;

  virtual bool setValue(DataObjectDescriptor& value );

private:
  DataObjectDescriptor * m_item;
};

class GAUDI_API DataObjectDescriptorCollectionProperty : public Property {
public:

  DataObjectDescriptorCollectionProperty( const std::string& name, DataObjectDescriptorCollection& items );

  DataObjectDescriptorCollectionProperty(DataObjectDescriptorCollectionProperty & other);

  DataObjectDescriptorCollectionProperty(const DataObjectDescriptorCollectionProperty & other);

  DataObjectDescriptorCollectionProperty& operator=(DataObjectDescriptorCollectionProperty& value );

  DataObjectDescriptorCollectionProperty& operator=(const DataObjectDescriptorCollectionProperty& value );

  virtual DataObjectDescriptorCollectionProperty* clone() const;

  virtual bool load( Property& destination ) const;

  virtual bool assign( const Property& source );

  virtual std::string toString() const;

  virtual void toStream(std::ostream& out) const;

  virtual StatusCode fromString(const std::string& s);

  const DataObjectDescriptorCollection& value() const;

  bool setValue(DataObjectDescriptorCollection& value );

private:
  DataObjectDescriptorCollection * m_dataItems;
};

#endif
