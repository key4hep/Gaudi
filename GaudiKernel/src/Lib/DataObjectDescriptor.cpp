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

#include <GaudiKernel/DataObjectDescriptor.h>

const char ADDR_SEP = '&';
const char FIELD_SEP = '|';
const char ITEM_SEP = '#';

const std::string DataObjectDescriptor::NULL_ = "NULL";

std::ostream& operator<<( std::ostream&   stream ,
                          const DataObjectDescriptor& di   )
{ return stream << di.toString() ; }

const std::string DataObjectDescriptor::toString() const {
	std::stringstream str;

	str << m_tag << FIELD_SEP;
	str << m_address;

	if(UNLIKELY(!m_altAddresses.empty())){
		for(auto & s : m_altAddresses)
			str << ADDR_SEP << s;
	}

	str << FIELD_SEP;

	str << boost::lexical_cast<std::string>(m_optional) << FIELD_SEP;
	str << boost::lexical_cast<std::string>(m_accessType);

	return str.str();
}

const std::string DataObjectDescriptor::pythonRepr() const {
	return "DataObjectDescriptor(\"" + toString() + "\")";
}

void DataObjectDescriptor::fromString(const std::string& s) {
	std::vector<std::string> items;

	boost::split(items, s, boost::is_any_of(boost::lexical_cast<std::string>(FIELD_SEP)), boost::token_compress_on);

	setTag(items[0]);

	if(LIKELY(items[1].find(ADDR_SEP) == std::string::npos))
		setAddress(items[1]); //address
	else { //alternative addresses provided
		std::vector<std::string> addr;
		boost::split(addr, items[1], boost::is_any_of(boost::lexical_cast<std::string>(ADDR_SEP)), boost::token_compress_on);

		setAddress(addr[0]); //main address
		setAltAddress(addr, true); //set alternatives, skipping first
	}

	setOptional(boost::lexical_cast<bool>( items[2] ));
	setAccessType(IDataObjectHandle::AccessType(boost::lexical_cast<int>( items[3] )));

}

IDataObjectHandle::AccessType DataObjectDescriptor::accessType() const {
	return m_accessType;
}

void DataObjectDescriptor::setAccessType(
		IDataObjectHandle::AccessType accessType) {
	m_accessType = accessType;
}

bool DataObjectDescriptor::optional() const {
	return m_optional;
}

void DataObjectDescriptor::setOptional(bool optional) {
	m_optional = optional;
}

const std::string& DataObjectDescriptor::tag() const {
	return m_tag;
}

void DataObjectDescriptor::setTag(const std::string& tag) {
	m_tag = tag;
}

std::string DataObjectDescriptor::tag(const std::string & s) {

	//std::cout << "Extracting tag from: " << s << std::endl;

	std::vector<std::string> items;

	boost::split(items, s,
			boost::is_any_of(boost::lexical_cast<std::string>(FIELD_SEP)),
			boost::token_compress_on);

	/*std::cout << "Got: ";
	for(uint i = 0; i < items.size(); ++i)
		std::cout << items[i] << " ";
	std::cout << std::endl;*/

	return items[0];

}

const std::string& DataObjectDescriptor::address() const {
	return m_address;
}

const std::vector<std::string>& DataObjectDescriptor::alternativeAddresses() const {
	return m_altAddresses;
}

void DataObjectDescriptor::setAddress(const std::string& address) {

	//std::cout << "setting address for " << m_tag << " from " << m_address << " to " << address
	//		<< " with " << (m_handle.isValid() ? "valid" : "NOT valid") << " handle" << std::endl;

	if(!m_handle || !m_handle->initialized())
		m_address = address;
}

void DataObjectDescriptor::setAddresses(const std::vector<std::string>& addresses) {

	//std::cout << "setting address for " << m_tag << " from " << m_address << " to " << address
	//		<< " with " << (m_handle.isValid() ? "valid" : "NOT valid") << " handle" << std::endl;

	if(!m_handle || !m_handle->initialized()){
		setAddress(addresses[0]);
		setAltAddress(addresses, true);
	}
}

void DataObjectDescriptor::setAltAddress(const std::vector<std::string> & addresses, bool skipFirst){
	if(!m_handle || !m_handle->initialized())
		m_altAddresses.assign(addresses.begin() + skipFirst, addresses.end());
}

const std::string DataObjectDescriptorCollection::toString() const {

	std::stringstream out;

	for(auto it = m_dataItems.begin(); it != m_dataItems.end(); ++it){
		out << it->second.toString() << ITEM_SEP;
	}

	std::string sOut = out.str();

	return sOut.substr(0, sOut.length()-1);

}

void DataObjectDescriptor::setHandle(SmartIF<MinimalDataObjectHandle> handle){

	//std::cout << "setting handle for " << m_tag << ": " << "my handle is " <<
	//		(m_handle.isValid() ? "" : "NOT") << "valid; other handle is " << (handle.isValid() ? "" : "NOT") << " valid" << std::endl;

	if(!m_handle.isValid() && handle.isValid()) //don't override an existing handle with NULL handle
		m_handle = handle;

}

DataObjectDescriptorCollection::DataObjectDescriptorCollection(const DataObjectDescriptorCollection & other){
	m_dataItems.insert(other.m_dataItems.begin(), other.m_dataItems.end());
}

DataObjectDescriptorCollection & DataObjectDescriptorCollection::operator=(const DataObjectDescriptorCollection& other){
	m_dataItems.clear();
	m_dataItems.insert(other.m_dataItems.begin(), other.m_dataItems.end());

	return *this;
}

const std::string DataObjectDescriptorCollection::pythonRepr() const {
	return "DataObjectDescriptorCollection(\"" + toString() + "\")";
}

bool DataObjectDescriptorCollection::contains(const std::string & s) const {
	//std::cout << "trying to find: " << s << std::endl;

	//if(s != "tracks" && s != "hits" && s != "trackSelection")
		//raise(SIGINT);

	return s != "" && m_dataItems.find(s) != m_dataItems.end();
}

void DataObjectDescriptorCollection::insert(const std::string& tag,
		const std::string& address,
		const bool optional,
		const IDataObjectHandle::AccessType accessType){

	if(!contains(tag)){
		m_dataItems.emplace(std::piecewise_construct,
		          std::make_tuple(tag),
		          std::make_tuple(tag, address, optional, accessType));
	}

}

void DataObjectDescriptorCollection::insert(const std::string& tag,
		const std::vector<std::string>& addresses,
		const bool optional,
		const IDataObjectHandle::AccessType accessType){

	if(!contains(tag)){
		m_dataItems.emplace(std::piecewise_construct,
		          std::make_tuple(tag),
		          std::make_tuple(tag, addresses, optional, accessType));
	}

}

void DataObjectDescriptorCollection::insert(const DataObjectDescriptor & item){

	if(!contains(item.tag())){
		m_dataItems.emplace(item.tag(), item);
	}

}

void DataObjectDescriptorCollection::update(const DataObjectDescriptor & item){

	if(contains(item.tag()))
		m_dataItems.at(item.tag()) = item;

}

void DataObjectDescriptorCollection::update(const std::string & item){

	std::string tag = DataObjectDescriptor::tag(item);

	if(tag != "" && contains(tag))
		m_dataItems.at(tag).fromString(item);

}

void DataObjectDescriptorCollection::insert(const std::string & item){

	std::string tag = DataObjectDescriptor::tag(item);

	if(tag != "" && !contains(tag)){
		m_dataItems.emplace(tag, item);
	}

}

const DataObjectDescriptor & DataObjectDescriptorCollection::operator [](const std::string & tag) const {
	return m_dataItems.at(tag);
}

DataObjectDescriptor & DataObjectDescriptorCollection::operator [](const std::string & tag) {
	return m_dataItems.at(tag);
}

void DataObjectDescriptorCollection::insertOrUpdate(const std::string & item){

	//std::cout << "insertOrUpdate: " << item << std::endl;

	std::string tag = DataObjectDescriptor::tag(item);

	//std::cout << "tag: " << tag << std::endl;

	if(tag == "")
		return;

	if(contains(tag))
		update(item);
	else
		insert(item);
}


DataObjectDescriptorProperty::DataObjectDescriptorProperty(const std::string & name,
		DataObjectDescriptor & item)
	: Property(name, typeid(DataObjectDescriptor)),
	  m_item(&item) {
}

DataObjectDescriptorProperty::DataObjectDescriptorProperty(DataObjectDescriptorProperty & other)
	: Property(other.name(), typeid(DataObjectDescriptor)), m_item(other.m_item) {
}

DataObjectDescriptorProperty::DataObjectDescriptorProperty(const DataObjectDescriptorProperty & other)
	: Property(other.name(), typeid(DataObjectDescriptor)) {

	m_item = new DataObjectDescriptor(other.value());

}

DataObjectDescriptorProperty& DataObjectDescriptorProperty::operator =(
		DataObjectDescriptorProperty& value) {

	setName(value.name());
	m_item = value.m_item;

	return *this;

}

DataObjectDescriptorProperty& DataObjectDescriptorProperty::operator =(
		const DataObjectDescriptorProperty& value) {

	setName(value.name());

	delete m_item;
	m_item = new DataObjectDescriptor(value.value());

	return *this;

}

DataObjectDescriptorProperty* DataObjectDescriptorProperty::clone() const {
	return new DataObjectDescriptorProperty(*this);
}

 bool DataObjectDescriptorProperty::load(Property& destination) const {
	return destination.assign(*this);
}

 bool DataObjectDescriptorProperty::assign(const Property& source) {
	return fromString(source.toString()).isSuccess();
}

void DataObjectDescriptorProperty::toStream(std::ostream& out) const {
	out << toString();
}

std::string DataObjectDescriptorProperty::toString() const {
	useReadHandler();
	return m_item->toString();
}

StatusCode DataObjectDescriptorProperty::fromString(const std::string& s) {
	m_item->fromString(s);

	return useUpdateHandler();
}

const DataObjectDescriptor& DataObjectDescriptorProperty::value() const {
	useReadHandler();
	return *m_item;
}

bool DataObjectDescriptorProperty::setValue(DataObjectDescriptor& value) {
	m_item = &value;

	return useUpdateHandler();
}

DataObjectDescriptorCollectionProperty::DataObjectDescriptorCollectionProperty(const std::string& name,
		DataObjectDescriptorCollection& items)
	: Property(name, typeid(DataObjectDescriptorCollection)), m_dataItems(&items){
}

DataObjectDescriptorCollectionProperty::DataObjectDescriptorCollectionProperty(
		DataObjectDescriptorCollectionProperty& other)
	: Property(other.name(), typeid(DataObjectDescriptorCollection)), m_dataItems(other.m_dataItems){
}

DataObjectDescriptorCollectionProperty::DataObjectDescriptorCollectionProperty(
		const DataObjectDescriptorCollectionProperty& other)
	: Property(other.name(), typeid(DataObjectDescriptorCollection)){

	m_dataItems = new DataObjectDescriptorCollection(other.value());
}

DataObjectDescriptorCollectionProperty& DataObjectDescriptorCollectionProperty::operator =(
		DataObjectDescriptorCollectionProperty& value) {

	m_dataItems = value.m_dataItems;
	return *this;
}

DataObjectDescriptorCollectionProperty& DataObjectDescriptorCollectionProperty::operator =(
		const DataObjectDescriptorCollectionProperty& value) {

	delete m_dataItems;
	m_dataItems = new DataObjectDescriptorCollection(value.value());
	return *this;
}

DataObjectDescriptorCollectionProperty* DataObjectDescriptorCollectionProperty::clone() const {
	return new DataObjectDescriptorCollectionProperty(*this);
}

bool DataObjectDescriptorCollectionProperty::load(Property& destination) const {
	return destination.assign(*this);
}

bool DataObjectDescriptorCollectionProperty::assign(const Property& source) {
	//std::cout << "Loading DODs from " << source.toString() << std::endl;
	return fromString(source.toString());
}

std::string DataObjectDescriptorCollectionProperty::toString() const {

	useReadHandler();

	return m_dataItems->toString();

}

void DataObjectDescriptorCollectionProperty::toStream(std::ostream& out) const {
	useReadHandler();
	out << m_dataItems->toString();
}

StatusCode DataObjectDescriptorCollectionProperty::fromString(const std::string& s) {

	//std::cout << "from string: " << s << std::endl;

	std::vector<std::string> items;
	boost::split(items, s, boost::is_any_of(boost::lexical_cast<std::string>(ITEM_SEP)), boost::token_compress_on);

	for(uint i = 0; i < items.size(); ++i){
		//std::cout << "item " << i << ": " << items[i] << std::endl;
		m_dataItems->insertOrUpdate(items[i]);
	}

	return useUpdateHandler();
}

const DataObjectDescriptorCollection& DataObjectDescriptorCollectionProperty::value() const {
	useReadHandler();
	return *m_dataItems;
}

bool DataObjectDescriptorCollectionProperty::setValue(DataObjectDescriptorCollection& value) {
	m_dataItems = &value;

	return useUpdateHandler();
}