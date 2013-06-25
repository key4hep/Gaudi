// $Id:$
#ifndef JOBOPTIONSVC_ITERATOR_H_
#define JOBOPTIONSVC_ITERATOR_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL
// ============================================================================
#include <fstream>
// ============================================================================
// Boost:
// ============================================================================

#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
// ============================================================================
typedef std::string::const_iterator BaseIterator;
typedef boost::spirit::multi_pass<BaseIterator> ForwardIterator;
typedef boost::spirit::classic::position_iterator2<ForwardIterator> Iterator;
typedef boost::spirit::classic::file_position_base<std::string>
    IteratorPosition;
// ============================================================================
#endif
