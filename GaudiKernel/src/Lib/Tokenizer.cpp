// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/Tokenizer.cpp,v 1.4 2008/10/27 16:41:33 marcocle Exp $
//==============================================================================
// Tokenizer.h
//------------------------------------------------------------------------------
//
// Package : Kernel
//             The LHCb definition package
//
// Author  : M.Frank
//
// Changes : M.Frank ,  01/10/00: Initiali version
//
//==============================================================================
#define KERNEL_TOKENIZER_CPP
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/Environment.h"

#include <cstring>

// Assignment operator
Tokenizer::Token& Tokenizer::Token::operator=(const Token& copy)     {
  m_length=copy.m_length;
  m_tag=copy.m_tag;
  m_value=copy.m_value;
  return *this;
}

// Resolve value from environment 
void Tokenizer::Token::resolveValue()  {
  std::string res;
  if ( System::resolveEnv(m_value, res).isSuccess() )  {
    m_value = res;
  }
}

// Create tokens from string
void Tokenizer::analyse(const std::string& s, const char* delim, const char* tagBegin, const char* tagEnd, const char* eq, const char* valBegin, const char* valEnd)    {
  m_tokens.clear();
  if ( valEnd == 0 ) valEnd = valBegin;
  long start = 0;
  Token tok;
  do {
    tok.make(s, start, delim, tagBegin, tagEnd, eq, valBegin, valEnd);
    if ( tok.length() > 0 )   {
      start += tok.length();
      m_tokens.push_back(tok);
    }
    else  { 
      start += s.length();
    }
  } while ( start < long(s.length()) );
  if ( m_resolve )  {
    for(Items::iterator i=m_tokens.begin(); i != m_tokens.end(); ++i)  {
      (*i).resolveValue();
    }
  }
}

// Create token from string
void Tokenizer::Token::make(const std::string& s, long st, const char* delim, const char* tagBeg, const char* tagEnd, const char* eq, const char* valBeg, const char* valEnd)    {
  long lenTagBeg   = ::strlen(tagBeg);
  long lenTagEnd   = ::strlen(tagEnd);
  long lenValBeg   = ::strlen(valBeg);
  long lenValEnd   = ::strlen(valEnd);
  long lenDelim    = ::strlen(delim);
  long lenEq       = ::strlen(eq);
  long slen        = s.length();
  long start       = st;
  while(::strncmp(s.c_str()+start,delim,lenDelim)==0) start += lenDelim;
  long posTag      = (lenTagBeg>0) ? s.find(tagBeg, start) + lenTagBeg : start;
  long posDelim    = s.find(delim,posTag) > 0 ? s.find(delim,posTag) : slen;
  long posEq       = s.find(eq, posTag)   > 0 ? s.find(eq, posTag) : posDelim-posTag;
  long lenTag      = (lenTagEnd>0)   ? s.find(tagEnd, posTag)-posTag : (posEq>0) ? (posEq>posTag) ? posEq-posTag : posDelim-posTag : posDelim;
  posEq            = s.find(eq, posTag+lenTag+lenTagEnd)+lenEq;
  long posVal      = (lenValBeg>0)   ? s.find(valBeg, posEq)+lenValBeg : posEq;
  posDelim         = (lenDelim>0)    ? s.find(delim, posVal+lenValBeg) : slen;
  long lenVal      = (lenValEnd>0)   ? s.find(valEnd, posVal)-posVal : ((posDelim>=0) ? posDelim : slen)-posVal;

  m_tag = m_value  = "";
  // Ooops: Valid tag found:
  if ( start  >= 0 && posTag >= 0 )   {
    m_tag    = s.substr(posTag, lenTag);
    m_length = posTag+m_tag.length()+lenTagEnd-st;
  }
  // Ooops: there is also a value:
  if ( posVal >= 0 && lenVal >= 0 )   {
    m_value  = s.substr(posVal, lenVal);
    m_length = posVal+m_value.length()+lenValEnd-st;
  }
  while(::strncmp(s.c_str()+start+m_length,delim,lenDelim)==0) m_length += lenDelim;
}

