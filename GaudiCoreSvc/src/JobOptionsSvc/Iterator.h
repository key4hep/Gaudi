/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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

#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
// ============================================================================
typedef std::string::const_iterator                                 BaseIterator;
typedef boost::spirit::multi_pass<BaseIterator>                     ForwardIterator;
typedef boost::spirit::classic::position_iterator2<ForwardIterator> Iterator;
typedef boost::spirit::classic::file_position_base<std::string>     IteratorPosition;
// ============================================================================
#endif
