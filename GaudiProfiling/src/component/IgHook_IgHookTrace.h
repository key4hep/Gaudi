#ifndef IG_HOOK_IG_HOOK_TRACE_H
# define IG_HOOK_IG_HOOK_TRACE_H

//<<<<<< INCLUDES                                                       >>>>>>

# include <new>
# include <cstddef>

//<<<<<< PUBLIC DEFINES                                                 >>>>>>
//<<<<<< PUBLIC CONSTANTS                                               >>>>>>
//<<<<<< PUBLIC TYPES                                                   >>>>>>
//<<<<<< PUBLIC VARIABLES                                               >>>>>>
//<<<<<< PUBLIC FUNCTIONS                                               >>>>>>
//<<<<<< CLASS DECLARATIONS                                             >>>>>>

class IgHookTraceAlloc
{
public:
    IgHookTraceAlloc (void);

    void *	allocate (size_t bytes);

private:
    void *	m_pool;
    size_t	m_left;
};

class IgHookTrace
{
public:
    /** Nearly dummy object type to identify a counter. */
    struct Counter { const char *m_name; };

    /** Value for a counter chained from a trace.  */
    class CounterValue
    {
    public:
	void *			operator new (size_t n, IgHookTraceAlloc *alloc = 0);

	CounterValue (Counter *counter,
		      CounterValue *next = 0,
		      unsigned long long value = 0);
	// implicit copy constructor
	// implicit assignment operator
	// implicit destructor

	Counter *		counter (void);
	CounterValue *		next (void);
	unsigned long long	count (void);

	unsigned long long	value (void);
	unsigned long long	tick (void);
	unsigned long long	untick (void);
	unsigned long long	add (unsigned long long value);
	unsigned long long	add (CounterValue &x);
	unsigned long long	sub (unsigned long long value);
	unsigned long long	sub (CounterValue &x);
	unsigned long long	max (unsigned long long value);
	unsigned long long	max (CounterValue &x);

    private:
	Counter			*m_counter;
	CounterValue		*m_next;
	unsigned long long	m_value;
	unsigned long long	m_count;
    };

    // General utilities
    static int		stacktrace (void **addresses, int nmax);
    static void *	tosymbol (void *address);
    static bool		symbol (void *address, const char *&sym,
		    		const char *&lib, int &offset,
				int &liboffset);

    void *		operator new (size_t n, IgHookTraceAlloc *alloc = 0);

    // Class methods
    IgHookTrace (IgHookTrace *parent = 0, void *address = 0);

    IgHookTrace *	parent (void);
    IgHookTrace *	next (void);

    void *		address (void);
    bool		symbol (const char *&sym, const char *&lib,
		    		int &offset, int &liboffset);

    IgHookTrace *	children (void);
    IgHookTrace *	child (void *address);

    CounterValue *	counters (void);
    CounterValue *	counter (Counter *id);

    void		merge (IgHookTrace *other);

private:
    IgHookTraceAlloc	*m_alloc;
    IgHookTrace		*m_parent;
    IgHookTrace		*m_next;
    IgHookTrace		*m_children;
    void		*m_address;
    CounterValue	*m_counters;
};

//<<<<<< INLINE PUBLIC FUNCTIONS                                        >>>>>>
//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // IG_HOOK_IG_HOOK_TRACE_H
