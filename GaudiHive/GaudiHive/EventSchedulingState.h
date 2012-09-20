#ifndef GAUDIHIVE_EVENTSCHEDULINGSTATE_H
#define GAUDIHIVE_EVENTSCHEDULINGSTATE_H


#include "tbb/concurrent_vector.h"

#include <atomic>
#include <bitset>


// typedef for the event and algo state
typedef std::bitset<1000> state_type;


class EventSchedulingState {
 public:
  EventSchedulingState(const unsigned int& n_algos);
  ~EventSchedulingState(){};

  void algoFinished(); 
  void algoStarts(unsigned int& index);
  bool hasStarted(unsigned int& index) const;
  bool hasFinished() const {return (m_algosFinished == m_numberOfAlgos);}
  const state_type& state() const {return m_eventState;}
  void update_state(unsigned int& product_index);

 private:  
  /// Number of algos in flight 
  std::atomic_uint m_algosInFlight;
  /// Number of finished algos 
  std::atomic_uint m_algosFinished;
  /// Total number of algos 
  unsigned int m_numberOfAlgos;
  /// Event state recording which products are there
  state_type m_eventState;
  /// Register of algorithms started
  tbb::concurrent_vector<bool> m_algosStarted;
};


#endif // GAUDIHIVE_EVENTSCHEDULINGSTATE_H
