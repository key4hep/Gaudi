/*
 * DataObjectHandle.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: dfunke
 */

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <csignal>

#include <GaudiKernel/DataItemProperty.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/DataObjectHandle.h>

const char FIELD_SEP = '|';
const char ITEM_SEP = '#';

std::ostream& operator<<( std::ostream&   stream ,
                          const DataItemBase& di   )
{ return stream << di.toString() ; }

const std::string DataItemBase::toString() const {
	return  m_tag + FIELD_SEP
			+ m_productName + FIELD_SEP
			+ boost::lexical_cast<std::string>(m_optional) + FIELD_SEP
			+ boost::lexical_cast<std::string>(m_accessType);
}

const std::string DataItemBase::pythonRepr() const {
	return "DataItem(\"" + toString() + "\")";
}

void DataItemBase::fromString(const std::string& s) {
	std::vector<std::string> items;
	boost::split(items, s, boost::is_any_of(boost::lexical_cast<std::string>(FIELD_SEP)), boost::token_compress_on);

	setTag(items[0]);
	setProductName(items[1]); //productName
	setOptional(boost::lexical_cast<bool>( items[2] ));
	setAccessType(IDataObjectHandle::AccessType(boost::lexical_cast<int>( items[3] )));

}

IDataObjectHandle::AccessType DataItemBase::accessType() const {
	return m_accessType;
}

void DataItemBase::setAccessType(
		IDataObjectHandle::AccessType accessType) {
	m_accessType = accessType;
}

bool DataItemBase::optional() const {
	return m_optional;
}

void DataItemBase::setOptional(bool optional) {
	m_optional = optional;
}

const std::string& DataItemBase::tag() const {
	return m_tag;
}

void DataItemBase::setTag(const std::string& tag) {
	m_tag = tag;
}

const std::string & DataItemBase::tag(const std::string & s) {
	std::vector<std::string> items;
	boost::split(items, s,
			boost::is_any_of(boost::lexical_cast<std::string>(FIELD_SEP)),
			boost::token_compress_on);

	return items[0];

}

const std::string& DataItemBase::productName() const {
	return m_productName;
}

void DataItemBase::setProductName(const std::string& productName) {
	m_productName = productName;
}

const std::string DataItems::toString() const {

	std::stringstream out;

	for(auto it = m_dataItems.begin(); it != m_dataItems.end(); ++it){
		out << it->second->toString() << ITEM_SEP;
	}

	std::string sOut = out.str();

	return sOut.substr(0, sOut.length()-1);

}

DataItems::DataItems(const DataItems & other){
	m_dataItems.insert(other.m_dataItems.begin(), other.m_dataItems.end());
}

DataItems & DataItems::operator=(const DataItems& other){
	m_dataItems.clear();
	m_dataItems.insert(other.m_dataItems.begin(), other.m_dataItems.end());

	return *this;
}

template <class T>
DataObjectHandle<T> * DataItem<T>::getHandle(IAlgorithm* fatherAlg) const {

	return new DataObjectHandle<T>(productName(), fatherAlg, accessType(), optional());
}

const std::string DataItems::pythonRepr() const {
	return "DataItems(\"" + toString() + "\")";
}

bool DataItems::contains(const std::string & s) const {
	std::cout << "trying to find: " << s << std::endl;

	//if(s != "tracks" && s != "hits" && s != "trackSelection")
		//raise(SIGINT);

	return s != "" && m_dataItems.find(s) != m_dataItems.end();
}


void DataItems::insert(std::shared_ptr<DataItemBase> item){

	if(!contains(item->tag()))
		m_dataItems[item->tag()] = item;

}

void DataItems::update(std::shared_ptr<DataItemBase> item){

	if(contains(item->tag()))
		(*m_dataItems[item->tag()]) = *item;

}

void DataItems::update(const std::string & item){

	std::string tag = DataItemBase::tag(item);

	if(tag != "" && contains(tag))
		m_dataItems[tag]->fromString(item);

}

void DataItems::insert(const std::string & item){

	std::string tag = DataItemBase::tag(item);

	if(tag != "" && !contains(tag)){
		m_dataItems[tag] = std::shared_ptr<DataItemBase>(
												new DataItem<DataObject>(item));
	}

}

void DataItems::insertOrUpdate(const std::string & item){

	std::string tag = DataItemBase::tag(item);

	if(tag == "")
		return;

	if(contains(tag))
		update(item);
	else
		insert(item);
}

DataItemPropertyBase::DataItemPropertyBase(const std::string & name)
	: Property(name, typeid(DataItemBase)) {
}

DataItemPropertyBase::DataItemPropertyBase(const DataItemPropertyBase & other)
	: Property(other.name(), typeid(DataItemBase)) {
}

DataItemPropertyBase & DataItemPropertyBase::operator=(const DataItemPropertyBase & other){
	setName(other.name());

	return *this;
}

template <class T>
DataItemProperty<T>::DataItemProperty(const std::string & name,
		DataItem<T> & item)
	: DataItemPropertyBase(name),
	  m_item(item) {
}

template <class T>
DataItemProperty<T>::DataItemProperty(const DataItemProperty<T> & other)
	: DataItemPropertyBase(other.name()), m_item(other.m_item) {
}

template <class T>
inline DataItemProperty<T>& DataItemProperty<T>::operator =(
		const DataItemProperty<T>& value) {

	setName(value.name());
	m_item = value.m_item;

	return *this;

}

template <class T>
inline DataItemProperty<T>* DataItemProperty<T>::clone() const {
	return new DataItemProperty<T>(*this);
}

template <class T>
inline bool DataItemProperty<T>::load(Property& destination) const {
	return destination.assign(*this);
}

template <class T>
inline bool DataItemProperty<T>::assign(const Property& source) {
	return fromString(source.toString()).isSuccess();
}

template <class T>
void DataItemProperty<T>::toStream(std::ostream& out) const {
	out << toString();
}

template <class T>
std::string DataItemProperty<T>::toString() const {
	useReadHandler();
	return m_item.toString();
}

template <class T>
StatusCode DataItemProperty<T>::fromString(const std::string& s) {
	m_item.fromString(s);

	return useUpdateHandler();
}

template <class T>
const DataItem<T>& DataItemProperty<T>::value() const {
	useReadHandler();
	return m_item;
}

template <class T>
bool DataItemProperty<T>::setValue(const DataItem<T>& value) {
	m_item = value;

	return useUpdateHandler();
}

DataItemArrayProperty::DataItemArrayProperty(const std::string& name,
		DataItems& items)
	: Property(name, typeid(DataItems)), m_dataItems(items){
}

DataItemArrayProperty::DataItemArrayProperty(
		const DataItemArrayProperty& other)
	: Property(other.name(), typeid(DataItems)), m_dataItems(other.m_dataItems){
}

DataItemArrayProperty& DataItemArrayProperty::operator =(
		const DataItemArrayProperty& value) {

	m_dataItems = value.m_dataItems;
	return *this;
}

DataItemArrayProperty* DataItemArrayProperty::clone() const {
	return new DataItemArrayProperty(*this);
}

bool DataItemArrayProperty::load(Property& destination) const {
	return destination.assign(*this);
}

bool DataItemArrayProperty::assign(const Property& source) {
	return fromString(source.toString());
}

std::string DataItemArrayProperty::toString() const {

	useReadHandler();

	return m_dataItems.toString();

}

void DataItemArrayProperty::toStream(std::ostream& out) const {
	useReadHandler();
	out << m_dataItems.toString();
}

StatusCode DataItemArrayProperty::fromString(const std::string& s) {

	std::vector<std::string> items;
	boost::split(items, s, boost::is_any_of(boost::lexical_cast<std::string>(ITEM_SEP)), boost::token_compress_on);

	for(uint i = 0; i < items.size(); ++i)
		m_dataItems.insertOrUpdate(items[i]);

	return useUpdateHandler();
}

const DataItems& DataItemArrayProperty::value() const {
	useReadHandler();
	return m_dataItems;
}

bool DataItemArrayProperty::setValue(const DataItems& value) {
	m_dataItems = value;

	return useUpdateHandler();
}
