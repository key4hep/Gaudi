//<<<<<< INCLUDES                                                       >>>>>>

#include "IgHook_IgHookTrace.h"
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#if __linux
# include <execinfo.h>
# include <ucontext.h>
# include <sys/syscall.h>
# if __x86_64__
#  define UNW_LOCAL_ONLY
#  include <libunwind.h>
# endif
#endif
#if __APPLE__
extern "C" void _sigtramp (void);
#endif

//<<<<<< PRIVATE DEFINES                                                >>>>>>

#if !defined MAP_ANONYMOUS && defined MAP_ANON
# define MAP_ANONYMOUS MAP_ANON
#endif

//<<<<<< PRIVATE CONSTANTS                                              >>>>>>
//<<<<<< PRIVATE TYPES                                                  >>>>>>
//<<<<<< PRIVATE VARIABLE DEFINITIONS                                   >>>>>>
//<<<<<< PUBLIC VARIABLE DEFINITIONS                                    >>>>>>
//<<<<<< CLASS STRUCTURE INITIALIZATION                                 >>>>>>
//<<<<<< PRIVATE FUNCTION DEFINITIONS                                   >>>>>>

#if 0 && __x86_64__ && __linux
// Linux x86-64 does not use regular call frames, like IA-32 does for
// example, and it would be a very difficult job to decipher the call
// stack.  In order to walk the call stack correctly, we have to use
// the DWARF-2 unwind data.  This alone is incredibly, uselessly slow
// for our purposes.
//
// We avoid using the unwind data by caching frame structures for
// recently seen functions.  This is slow to start with, but very
// quickly gets fast enough for our purposes.  Fortunately the x86-64
// unwind library appears to be robust enough to be called in signal
// handlers (unlike at least some IA-32 versions).
//
// The cache consists of two arrays arranged as an open-addressed
// unprobed hash table.  Hash collisions overwrite the entry with the
// latest data.  We try to avoid making this a problem by using a
// high-quality hash function and pure brute force in the form of a
// large hash table.  A couple of megabytes goes a long way to help!
//
// The first of the cache arrays, of "void *", tracks program counter
// addresses.  A parallel array of "int" tracks the size of the call
// frame at that address.  Given a program counter and the canonical
// frame address (CFA) of the previous (= above) call frame, the new
// frame address is the previous plus the delta.  We find the address
// of the caller just above this new frame address.
//
// We use the cache as long as we can find the addresses there.  When
// we fall off the cache, we resort to the language run time unwinder.

struct IgHookTraceArgs
{
  struct
  {
    void **pc;
    int **frame;
  } cache;
  struct
  {
    void **addresses;
    int top;
    int size;
  } stack;
  void **prevframe;
};

static _Unwind_Reason_Code
GCCBackTrace (_Unwind_Context *context, void *arg)
{
  IgHookTraceArgs *args = (IgHookTraceArgs *) arg;
  if (args->stack.top < 0 || args->stack.top >= args->stack.size)
    return _URC_END_OF_STACK;

  args->stack.addresses [args->stack.top++] = (void *) _Unwind_GetIP (context);
  args->prevframe = (void **) _Unwind_GetCFA (context);
  return _URC_NO_REASON;
}
#endif

//<<<<<< PUBLIC FUNCTION DEFINITIONS                                    >>>>>>
//<<<<<< MEMBER FUNCTION DEFINITIONS                                    >>>>>>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
IgHookTraceAlloc::IgHookTraceAlloc (void)
    : m_pool (0),
      m_left (0)
{}

void *
IgHookTraceAlloc::allocate (size_t bytes)
{
    // The reason for the existence of this class is to allocate
    // memory directly using mmap() so we don't create calls to
    // malloc() and friends.  This is for two reasons: it must be
    // possible to use this in asynchronous signal handlers, and
    // calling malloc() in those is a really bad idea; and this is
    // meant to be used by profiling code and it's nicer to not
    // allocate memory in ways tracked by the profiler.
    if (m_left < bytes)
    {
	size_t psize = getpagesize ();
	size_t hunk = psize * 20;
	if (hunk < bytes) hunk = (hunk + psize - 1) & ~(psize-1);
	void *addr = mmap (0, hunk, PROT_READ | PROT_WRITE,
			   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (addr == MAP_FAILED)
	    return 0;

	m_pool = addr;
	m_left = hunk;
    }

    void *ptr = m_pool;
    m_pool = (char *) m_pool + bytes;
    m_left -= bytes;
    return ptr;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void *
IgHookTrace::CounterValue::operator new (size_t n, IgHookTraceAlloc *alloc /* = 0 */)
{ return alloc ? alloc->allocate (n) : ::operator new (n); }

IgHookTrace::CounterValue::CounterValue (Counter *counter,
					 CounterValue *next /* = 0 */,
					 unsigned long long value /* = 0 */)
    : m_counter (counter),
      m_next (next),
      m_value (value),
      m_count (0)
{}

IgHookTrace::Counter *
IgHookTrace::CounterValue::counter (void)
{ return m_counter; }

IgHookTrace::CounterValue *
IgHookTrace::CounterValue::next (void)
{ return m_next; }

unsigned long long
IgHookTrace::CounterValue::value (void)
{ return m_value; }
    
unsigned long long
IgHookTrace::CounterValue::count (void)
{ return m_count; }
    
unsigned long long
IgHookTrace::CounterValue::tick (void)
{ ++m_count; return ++m_value; }

unsigned long long
IgHookTrace::CounterValue::untick (void)
{ --m_count; return --m_value; }

unsigned long long
IgHookTrace::CounterValue::add (unsigned long long value)
{ ++m_count; return m_value += value; }

unsigned long long
IgHookTrace::CounterValue::sub (unsigned long long value)
{ --m_count; return m_value -= value; }

unsigned long long
IgHookTrace::CounterValue::max (unsigned long long value)
{ ++m_count; if (m_value < value) m_value = value; return m_value; }

unsigned long long
IgHookTrace::CounterValue::add (CounterValue &x)
{ m_count += x.m_count; m_value += x.m_value; return m_value; }

unsigned long long
IgHookTrace::CounterValue::sub (CounterValue &x)
{ m_count -= x.m_count; m_value -= x.m_value; return m_value; }

unsigned long long
IgHookTrace::CounterValue::max (CounterValue &x)
{ m_count += x.m_count; if (m_value < x.m_value) m_value = x.m_value; return m_value; }

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void *
IgHookTrace::operator new (size_t n, IgHookTraceAlloc *alloc /* = 0 */)
{ return alloc ? alloc->allocate (n) : ::operator new (n); }

IgHookTrace::IgHookTrace (IgHookTrace *parent /* = 0 */, void *address /* = 0 */)
    : m_alloc (parent ? parent->m_alloc : new IgHookTraceAlloc),
      m_parent (parent),
      m_next (parent ? parent->m_children : 0),
      m_children (0),
      m_address (address),
      m_counters (0)
{ if (m_parent) m_parent->m_children = this; }

IgHookTrace *
IgHookTrace::parent (void)
{ return m_parent; }

IgHookTrace *
IgHookTrace::next (void)
{ return m_next; }

void *
IgHookTrace::address (void)
{ return m_address; }

bool
IgHookTrace::symbol (void *address,
		     const char *&sym,
		     const char *&lib,
		     int &offset,
		     int &liboffset)
{
    sym = lib = 0;
    offset = 0;
    liboffset = (unsigned long) address;

    Dl_info info;
    if (dladdr (address, &info))
    {
	if (info.dli_fname && info.dli_fname [0])
	    lib = info.dli_fname;

	if (info.dli_fbase)
	    liboffset = (unsigned long) address - (unsigned long) info.dli_fbase;

	if (info.dli_saddr)
	    offset = (unsigned long) address - (unsigned long) info.dli_saddr;

	if (info.dli_sname && info.dli_sname [0])
	    sym = info.dli_sname;

	return true;
    }

    return false;
}

bool
IgHookTrace::symbol (const char *&sym, const char *&lib, int &offset, int &liboffset)
{ return symbol (m_address, sym, lib, offset, liboffset); }

void *
IgHookTrace::tosymbol (void *address)
{
    Dl_info info;
    return (dladdr (address, &info)
	    && info.dli_fname
	    && info.dli_fname [0]
	    && info.dli_saddr)
	? info.dli_saddr : address;
}
