#ifndef GAUDIHIVE_ALGSEXECUTIONSTATES_H
#define GAUDIHIVE_ALGSEXECUTIONSTATES_H

// Framework include files
#include "GaudiKernel/Service.h" 

// C++ include files
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <iterator>

#include <boost/dynamic_bitset.hpp>

//---------------------------------------------------------------------------

/**@class AlgsExecutionStates AlgsExecutionStates.h GaudiKernel/AlgsExecutionStates.h
 *
 *  The AlgsExecutionStates encodes the state machine for the execution of 
 *  algorithms within a single event. It is used by the concurrent schedulers 
 * 
    @author  Benedikt Hegner 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class AlgsExecutionStates{
public:

	/// Execution states of the algorithms
	enum State : unsigned short {
		INITIAL = 0,
		CONTROLREADY = 1,
		DATAREADY = 2,
		SCHEDULED = 3,
		EVTACCEPTED = 4,
		EVTREJECTED = 5,
		ERROR = 6
	};

	static std::map<State,std::string> stateNames;

	AlgsExecutionStates(unsigned int algsNumber, SmartIF<IMessageSvc> MS):
		m_MS(MS) {

		for(uint i = INITIAL; i <= ERROR; ++i)
			m_states[i] = boost::dynamic_bitset<>(algsNumber); //everything initialized to zero

		m_states[INITIAL].set();
	};

	~AlgsExecutionStates(){};

	StatusCode updateState(unsigned int iAlgo,State newState);

	void reset(){
		m_states[INITIAL].set();

		for(uint i = (INITIAL+1); i <= ERROR; ++i)
			m_states[i].reset();
	};

	bool algsPresent(State state) const {
		return m_states.at(state).any();
	};

	bool allAlgsExecuted() const {
		return m_states.at(EVTACCEPTED).count() + m_states.at(EVTREJECTED).count() == m_states.at(INITIAL).size();
	};

	State operator[](unsigned int i) const {

		for(uint k = INITIAL; k <= ERROR; ++k) //early access, because often in lower state
			if(m_states.at(k).test(i))
				return State(k);

		return ERROR;

	};

	size_t size() const {
		return m_states.at(INITIAL).size();
	}

private:
	std::unordered_map<uint, boost::dynamic_bitset<> > m_states;
	SmartIF<IMessageSvc> m_MS;

public:
	class Iterator : public std::iterator<std::forward_iterator_tag, uint> {

	public:

		enum POS { BEGIN, END};

		Iterator(POS pos, const boost::dynamic_bitset<> & bs) : bs_(&bs) {
			if(pos == POS::BEGIN)
				pos_ = bs_->find_first();
			if(pos == POS::END)
				pos_ = boost::dynamic_bitset<>::npos;
			//std::cout << "initialized iterator at " << pos_ << std::endl;
		}

		~Iterator() {}

		Iterator& operator=(const Iterator& other){
			pos_ = other.pos_;
			bs_ = other.bs_;
			return(*this);
		}

		bool operator==(const Iterator& other){
			return bs_ == other.bs_ && pos_ == other.pos_;
		}

		bool operator!=(const Iterator& other){
			return bs_ != other.bs_ || pos_ != other.pos_;
		}

		Iterator& operator++(){
			if (pos_ != boost::dynamic_bitset<>::npos){
				pos_ = bs_->find_next(pos_);
				//std::cout << "advanced iterator to " << pos_ << std::endl;
			}
			return(*this);
		}

		Iterator& operator++(int){
			return(++(*this));
		}

		uint operator*(){
			return pos_;
		}

	private:
		boost::dynamic_bitset<>::size_type pos_;
		const boost::dynamic_bitset<> * bs_;
	};

	Iterator begin(State kind)
	{
		return(Iterator(Iterator::POS::BEGIN, m_states.at(kind)));
	}

	Iterator end(State kind)
	{
		return(Iterator(Iterator::POS::END, m_states.at(kind)));
	}

};

#endif // GAUDIHIVE_ALGSEXECUTIONSTATES_H
