#pragma once

#include <GaudiKernel/IDataObjectHandle.h>
#include <GaudiKernel/MinimalDataObjectHandle.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/Property.h>

#include <unordered_map>
#include <memory>
#include <functional>

template <class T>
class DataObjectHandle;

class GAUDI_API DataItemBase{

protected:

	DataItemBase(const std::string& tag,
			const std::string& productName,
			const bool optional = false,
			const IDataObjectHandle::AccessType accessType = IDataObjectHandle::READ)
	: m_tag(tag), m_productName(productName), m_optional(optional), m_accessType(accessType) {};

	DataItemBase(const DataItemBase & other)
	: m_tag(other.m_tag), m_productName(other.m_productName),
	  m_optional(other.m_optional), m_accessType(other.m_accessType) {}

	DataItemBase(const std::string s){
		fromString(s);
	}

public:

	virtual ~DataItemBase() { }

	DataItemBase & operator=(const DataItemBase& value){
		setTag(value.tag());
		setProductName(value.productName());
		setOptional(value.optional());
		setAccessType(value.accessType());

		return *this;
	}

	IDataObjectHandle::AccessType accessType() const;
	void setAccessType(IDataObjectHandle::AccessType accessType);

	bool optional() const;
	void setOptional(bool optional);

	const std::string& productName() const;
	void setProductName(const std::string& productName);

	const std::string& tag() const;
	void setTag(const std::string& tag);

	static const std::string & tag(const std::string & s);

	const std::string toString() const;
	void fromString(const std::string & s);

	const std::string pythonRepr() const;

	virtual MinimalDataObjectHandle * getHandle(IAlgorithm* fatherAlg) const = 0;

private:
	std::string m_tag;
	std::string m_productName;
	bool m_optional;
	IDataObjectHandle::AccessType m_accessType;

};

template <class T>
class GAUDI_API DataItem : public DataItemBase{

public:
	DataItem(const std::string& tag,
			const std::string& productName,
			const bool optional = false,
			const IDataObjectHandle::AccessType accessType = IDataObjectHandle::READ)
	: DataItemBase(tag, productName, optional, accessType) {};

	DataItem(const DataItem<T> & other)
	: DataItemBase(other) {}

	DataItem(const std::string s)
	: DataItemBase(s) {	}

	DataItem<T> & operator=(const DataItem<T>& value){
		setTag(value.tag());
		setProductName(value.productName());
		setOptional(value.optional());
		setAccessType(value.accessType());

		return *this;
	}

	virtual DataObjectHandle<T> * getHandle(IAlgorithm* fatherAlg) const;

};

class GAUDI_API DataItems {

public:

	DataItems() { }
	DataItems(const DataItems & other);
	DataItems & operator=(const DataItems& other);

	bool contains(const std::string & o) const;

	void insert(std::shared_ptr<DataItemBase> item);
	void update(std::shared_ptr<DataItemBase> item);

	void insert(const std::string & item);
	void update(const std::string & item);
	void insertOrUpdate(const std::string & item);

	const std::string toString() const;
	const std::string pythonRepr() const;

private:
	std::unordered_map<std::string, std::shared_ptr<DataItemBase>> m_dataItems;

};

class GAUDI_API DataItemPropertyBase: public Property {

protected:
	DataItemPropertyBase(const std::string & name);

	DataItemPropertyBase(const DataItemPropertyBase & other);

	DataItemPropertyBase& operator=(const DataItemPropertyBase& value);

public:
	virtual const DataItemBase& value() const = 0;

	virtual bool setValue( const DataItemBase& value ) = 0;

};

template <class T>
class GAUDI_API DataItemProperty: public DataItemPropertyBase {
public:
	DataItemProperty(const std::string & 	name, DataItem<T> & item);

	DataItemProperty(const DataItemProperty<T> & other);

	DataItemProperty& operator=(const DataItemProperty<T>& value);

	virtual DataItemProperty<T>* clone() const;

	virtual bool load(Property& destination) const;

	virtual bool assign(const Property& source);

	virtual std::string toString() const;

	virtual void toStream(std::ostream& out) const;

	virtual StatusCode fromString(const std::string& s);

	virtual const DataItem<T>& value() const;

	virtual bool setValue( const DataItem<T>& value );

private:
	DataItem<T> & m_item;
};

class GAUDI_API DataItemArrayProperty : public Property {
public:

	DataItemArrayProperty( const std::string& name, DataItems& items );

	DataItemArrayProperty(const DataItemArrayProperty & other);

	DataItemArrayProperty& operator=( const DataItemArrayProperty& value );

  virtual DataItemArrayProperty* clone() const;

  virtual bool load( Property& destination ) const;

  virtual bool assign( const Property& source );

  virtual std::string toString() const;

  virtual void toStream(std::ostream& out) const;

  virtual StatusCode fromString(const std::string& s);

  const DataItems& value() const;

  bool setValue( const DataItems& value );

private:
  DataItems & m_dataItems;
};
