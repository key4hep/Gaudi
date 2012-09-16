#ifndef GAUDIHIVE_EVENTSCHEDULINGSTATE_H
#define GAUDIHIVE_EVENTSCHEDULINGSTATE_H 1

class EventSchedulingState {
 public:
  EventSchedulingState(const unsigned int& n_algos);
  ~EventSchedulingState(){};

  void algoFinished(); 
  void algoStarts(unsigned int& index);
  bool hasStarted(unsigned int& index) const;
  bool hasFinished() const {return (m_algosFinished == m_numberOfAlgos);}; 
  const state_type& state() const {return m_eventState;};
  void update_state(unsigned int& product_index);

 private:  
  /// Number of algos in flight 
  tbb::atomic<unsigned int> m_algosInFlight;
  /// Number of finished algos 
  tbb::atomic<unsigned int> m_algosFinished;
  /// Total number of algos 
  unsigned int m_numberOfAlgos;
  /// Event state recording which products are there
  state_type m_eventState;
  /// Register of algorithms started
  tbb::concurrent_vector<bool> m_algosStarted;
};

EventSchedulingState::EventSchedulingState(const unsigned int& n_algos)
  : m_numberOfAlgos(n_algos), m_eventState(0)
{
  m_algosInFlight = 0;
  m_algosFinished = 0;
  m_algosStarted.resize(n_algos);
  std::fill(m_algosStarted.begin(),m_algosStarted.end(),false);
}

void EventSchedulingState::algoFinished() {
  --m_algosInFlight;
  ++m_algosFinished;
}

bool EventSchedulingState::hasStarted(unsigned int& index) const {
  return m_algosStarted[index];
}

void EventSchedulingState::algoStarts(unsigned int& index) {
  ++m_algosInFlight;
  m_algosStarted[index] = true; 
}

void EventSchedulingState::update_state(unsigned int& product_index){
  m_eventState[product_index] = true;
}

#endif // GAUDIHIVE_EVENTSCHEDULINGSTATE_H
