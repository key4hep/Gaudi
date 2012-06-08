//==============================================================================
// Tokenizer.h
//------------------------------------------------------------------------------
//
// Package : Kernel
//             The LHCb definition package
//
// Author  : M.Frank
//
// Changes : M.Frank ,  01/10/00: Initialil version
//
//==============================================================================
#ifndef KERNEL_TOKENIZER_H
#define KERNEL_TOKENIZER_H

/// STL Include files
#include "GaudiKernel/Kernel.h"
#include <string>
#include <vector>


/** definition of class Tokenizer
*/
class GAUDI_API Tokenizer   {
public:
  /** Definition of the sub-class Token.
  */
  class Token   {
    /// Toke length
    long        m_length;
    /// Tag variable
    std::string m_tag;
    /// Value variable
    std::string m_value;
  public:
    /// Copy constructor
    Token(const Token& copy)  : m_length(copy.m_length), m_tag(copy.m_tag), m_value(copy.m_value)  {
    }
    /// Standard constructor
    Token()  : m_length(0)  {
    }
    /// Create token from string
    void make(const std::string& s, long st, const char* delim, const char* tagBegin, const char* tagEnd, const char* eq, const char* valBegin, const char* valEnd);
    /// Standard Destructor
    virtual ~Token()    {
    }
    /// Assignment operator
    Token& operator=(const Token& copy);
    // Equality operator
    bool operator==(const Token& copy)    const   {
      return m_tag == copy.m_tag && m_value == copy.m_value && m_length == copy.m_length;
    }
    /// Total token length
    long length()   const   {
      return m_length;
    }
    /// Acces tag value
    const std::string& tag()   const    {
      return m_tag;
    }
    /// Access value
    const std::string& value()  const   {
      return m_value;
    }
    /// Resolve value from environment
    void resolveValue();
  };
  typedef Token Item;
  typedef std::vector<Token> Items;
protected:
  /// Assigned tokens within string
  Items m_tokens;
  /// Flag to resolve environment
  bool  m_resolve;
public:
  /// Standard (dummy) constructor
  Tokenizer() : m_resolve(false)  {}
  /// Initializing constructor
  Tokenizer(bool resolve) : m_resolve(resolve)  {}
  /// Standard destructor
  virtual ~Tokenizer()    {
    m_tokens.erase(m_tokens.begin(), m_tokens.end());
  }
  /// Analyse tokens from string
  void analyse(const std::string& s, const char* delim, const char* tagBegin, const char* tagEnd, const char* eq, const char* valBegin, const char* valEnd);
  /// Analyse tokens from string
  void analyse(const std::string& s, const char* delim, const char* tag, const char* eq, const char* val)    {
    analyse(s, delim, tag, tag, eq, val, val);
  }
  /// Analyse tokens from string
  void analyse(const std::string& s, const char* delim, const char* eq, const char* val)    {
    analyse(s, delim, "", "", eq, val, val);
  }
  /// Analyse tokens from string
  void analyse(const std::string& s, const char* delim=" ", const char* eq="=")    {
    analyse(s, delim, "", "", eq, "", "");
  }
  /// Access token collection
  Items& items()    {
    return m_tokens;
  }
  /// Access token collection (CONST)
  const Items& items()    const   {
    return m_tokens;
  }
};
#endif  // KERNEL_TOKENIZER_H
