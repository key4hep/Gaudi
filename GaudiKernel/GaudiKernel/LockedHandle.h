#ifndef GAUDIKERNEL_LOCKEDHANDLE
#define GAUDIKERNEL_LOCKEDHANDLE 1

#include <mutex>
#include <iostream>

template <class T>
class LockedHandle {
public:

  typedef std::mutex LockingHandleMutex_t;

  LockedHandle(T* ptr, LockingHandleMutex_t& mut) : m_ptr(ptr), m_mutex(&mut) {}
  LockedHandle(T* ptr, LockingHandleMutex_t* mut) : m_ptr(ptr), m_mutex(mut) {}
  LockedHandle(T* ptr) : m_ptr(ptr), m_mutex(0) {}
  LockedHandle() : m_ptr(nullptr), m_mutex(nullptr) {}

  void set(T* ptr, LockingHandleMutex_t* mut) { 
    m_ptr = ptr;
    m_mutex = mut;
  }
    
  void setMutex(LockingHandleMutex_t* mut) { m_mutex = mut; }

  T* get() const { return m_ptr; }

  class Guard  {
    Guard(const Guard &a) = delete;
  public:
    Guard(Guard &&a)
      : m_ptr(a.m_ptr),
        m_mutex(a.m_mutex)
    {
      a.m_mutex=nullptr;
    }
    
    Guard(T *ptr,LockingHandleMutex_t &mutex)
      : m_ptr(ptr),
        m_mutex(&mutex)
    {
      m_mutex->lock();
      //      std::cout << "lock" << std::endl;
    }
    ~Guard()  {
      m_mutex->unlock();
      //      std::cout << "unlock" << std::endl ;
    }
    T &operator*() { return *m_ptr; }
    T *operator->() { return m_ptr; }

    operator T&() {
      return *m_ptr;
    }

  private:
    T *m_ptr {nullptr};
    LockingHandleMutex_t *m_mutex {nullptr};
  };
  
  class ConstGuard  {
    ConstGuard(const ConstGuard &a) = delete;
  public:
    ConstGuard(ConstGuard &&a)
      : m_ptr(a.m_ptr),
        m_mutex(a.m_mutex)
    {
      a.m_mutex=nullptr;
    }
    
    ConstGuard(const T *ptr,LockingHandleMutex_t &mutex)
      : m_ptr(ptr),
        m_mutex(&mutex)
    {
      m_mutex->lock();
      //      std::cout << "lock" << std::endl;
    }
    ~ConstGuard()  {
      m_mutex->unlock();
      //      std::cout << "unlock" << std::endl;
    }
    const T &operator*() const { return *m_ptr; }
    const T *operator->() const { return m_ptr; }
    
    operator const T&() const {
      return *m_ptr;
    }
    
  private:
    const T *m_ptr;
    LockingHandleMutex_t *m_mutex;
  };


 /// Aquire and release the lock before and after the object is accessed.
  Guard operator*()  { return Guard(m_ptr,*m_mutex); }

  /// Aquire and release the lock before and after the object is accessed.
  Guard operator->() { return Guard(m_ptr,*m_mutex); }

  /// Aquire and release the lock before and after the const object is accessed.
  ConstGuard operator*()  const { return ConstGuard(m_ptr,*m_mutex); }

  /// Aquire and release the lock before and after the const object is accessed.
  ConstGuard operator->() const { return ConstGuard(m_ptr,*m_mutex); }

  operator bool() const {
    return m_ptr;
  }
private:

  T *m_ptr;
  mutable LockingHandleMutex_t *m_mutex;
};



#endif
