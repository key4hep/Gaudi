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
	DataObjectDescriptor(const DataObjectDescriptor & value); /* {

		//std::cout << "copy constructor for " << value.m_tag << ": " << "my handle is " <<
		//			(m_handle.isValid() ? "" : "NOT") << "valid; other handle is " << (value.m_handle.isValid() ? "" : "NOT") << " valid" << std::endl;

		//first set handle
		setHandle(value.m_handle);

		setTag(value.m_tag);
		setAddress(value.m_address);
		setAltAddress(value.m_altAddresses);
		setOptional(value.m_optional);
		setAccessType(value.m_accessType);
	}*/

	/*DataObjectDescriptor(const std::string s){
		fromString(s);
	}*/

	DataObjectDescriptor & operator=(const DataObjectDescriptor& value); /*{

		//std::cout << "operator= for " << value.m_tag << ": " << "my handle is " <<
		//			(m_handle.isValid() ? "" : "NOT") << "valid; other handle is " << (value.m_handle.isValid() ? "" : "NOT") << " valid" << std::endl;

		//first set handle
		setHandle(value.m_handle);

		setTag(value.m_tag);
		setAddress(value.m_address);
		setAltAddress(value.m_altAddresses);
		setOptional(value.m_optional);
		setAccessType(value.m_accessType);

		return *this;
	}*/

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
	void setAltAddresses(const std::vector<std::string> & addresses){ setAltAddresses(addresses, false); }

	const std::string& tag() const;
	void setTag(const std::string& tag);

	static std::string tag(const std::string & s);

	const std::string toString() const;
	void fromString(const std::string & s);

	const std::string pythonRepr() const;

	/*template<class T>
	SmartIF<DataObjectHandle<T> > createHandle(IAlgorithm* fatherAlg) {

		if (!m_handle.isValid() || m_handle->dataProductName() != address()){
			m_handle = SmartIF<DataObjectHandle<T> >(
					new DataObjectHandle<T>(*this, fatherAlg));

			//std::cout << "Created handle for " << m_tag << ": handle is " << (m_handle.isValid() ? "" : "NOT") << " valid" << std::endl;
		}

		return getHandle<T>();
	}

	template<class T>
	SmartIF<DataObjectHandle<T> > createHandle(IAlgTool* fatherTool) {

		if (!m_handle.isValid() || m_handle->dataProductName() != address()){
			m_handle = SmartIF<DataObjectHandle<T> >(
					new DataObjectHandle<T>(*this, fatherTool));

			//std::cout << "Created handle for " << m_tag << ": handle is " << (m_handle.isValid() ? "" : "NOT") << " valid" << std::endl;
		}

		return getHandle<T>();
	}
	template<class T>
	SmartIF<DataObjectHandle<T> > getHandle() const {
		return SmartIF<DataObjectHandle<T> >(dynamic_cast<DataObjectHandle<T> *>(m_handle.get()));
	}

	SmartIF<MinimalDataObjectHandle> getBaseHandle() const {
		return m_handle;
	}*/

	//SmartIF<MinimalDataObjectHandle> getHandle() const {
		//return m_handle;
	//}

	bool valid() const {
		return address() != NULL_;
	}

private:
	//void setHandle(SmartIF<MinimalDataObjectHandle> handle);
	void setAltAddresses(const std::vector<std::string> & addresses, bool skipFirst);

private:
	std::string m_tag;
	std::string m_address;
	std::vector<std::string> m_altAddresses;
	bool m_optional;
	MinimalDataObjectHandle::AccessType m_accessType;
	//SmartIF<MinimalDataObjectHandle> m_handle;

};

template <typename K, typename V>
struct get_key : std::unary_function<std::pair<K, V> const&, const K&> {
    const K& operator ()(std::pair<K, V> const& p) const { return p.first; }
};

class GAUDI_API DataObjectDescriptorCollection {

public:

	//typedef std::map<std::string, DataObjectDescriptor> tDataObjectDescriptorCollection;
    //typedef get_key<tDataObjectDescriptorCollection::key_type, tDataObjectDescriptorCollection::mapped_type> fGetTag;
    //typedef boost::transform_iterator<fGetTag, tDataObjectDescriptorCollection::iterator> tag_iterator;

	DataObjectDescriptorCollection() { }
	DataObjectDescriptorCollection(const DataObjectDescriptorCollection & other);
	DataObjectDescriptorCollection & operator=(const DataObjectDescriptorCollection& other);

	bool contains(const std::string & o) const;
	bool contains(const MinimalDataObjectHandle * o) const;

	bool insert(const std::string& tag,
				MinimalDataObjectHandle * descriptor);

	bool insert(MinimalDataObjectHandle * item);
	bool update(MinimalDataObjectHandle * item);

	//bool insert(const std::string & item);
	bool update(const std::string & item);

	//void insertOrUpdate(const std::string & item);

	boost::transform_iterator<
		get_key<std::string, MinimalDataObjectHandle *>,
		std::map<std::string, MinimalDataObjectHandle *>::iterator>
	begin() {
		return boost::transform_iterator<get_key<std::string, MinimalDataObjectHandle *>,
										 std::map<std::string, MinimalDataObjectHandle *>::iterator>(
												 m_dataItems.begin(), get_key<std::string, MinimalDataObjectHandle *>());
	}

	boost::transform_iterator<
		get_key<std::string, MinimalDataObjectHandle *>,
		std::map<std::string, MinimalDataObjectHandle *>::iterator>
	end() {
		return boost::transform_iterator<get_key<std::string, MinimalDataObjectHandle *>,
										 std::map<std::string, MinimalDataObjectHandle *>::iterator>(
												 m_dataItems.end(), get_key<std::string, MinimalDataObjectHandle *>());
	}

	boost::transform_iterator<
		get_key<std::string, MinimalDataObjectHandle *>,
		std::map<std::string, MinimalDataObjectHandle *>::const_iterator>
	begin() const {
		return boost::transform_iterator<get_key<std::string, MinimalDataObjectHandle *>,
										 std::map<std::string, MinimalDataObjectHandle *>::const_iterator>(
												 m_dataItems.begin(), get_key<std::string, MinimalDataObjectHandle *>());
	}

	boost::transform_iterator<
		get_key<std::string, MinimalDataObjectHandle *>,
		std::map<std::string, MinimalDataObjectHandle *>::const_iterator>
	end() const {
		return boost::transform_iterator<get_key<std::string, MinimalDataObjectHandle *>,
										 std::map<std::string, MinimalDataObjectHandle *>::const_iterator>(
												 m_dataItems.end(), get_key<std::string, MinimalDataObjectHandle *>());
	}

	const MinimalDataObjectHandle & operator[](const std::string & tag) const;
	MinimalDataObjectHandle & operator[](const std::string & tag);

	const std::string toString() const;
	const std::string pythonRepr() const;

private:
	std::map<std::string, MinimalDataObjectHandle *> m_dataItems;

};


class GAUDI_API DataObjectDescriptorProperty: public Property {
public:
	DataObjectDescriptorProperty(const std::string & 	name, DataObjectDescriptor & item);

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
