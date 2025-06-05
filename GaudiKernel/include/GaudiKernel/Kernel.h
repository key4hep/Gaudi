/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_KERNEL_H
#define GAUDIKERNEL_KERNEL_H

// Some pragmas to avoid warnings in VisualC
#ifdef _WIN32
// Disable warning C4786: identifier was truncated to '255' characters in the debug information
#  pragma warning( disable : 4786 )
// Disable warning C4291: no matching operator delete found; memory will not be freed if initialization throws an
// exception
#  pragma warning( disable : 4291 )
// Disable warning C4250: inheritance via dominance
#  pragma warning( disable : 4250 )
#endif

// Large integer definition depends of the platform
#ifndef NO_LONGLONG_TYPEDEF
typedef long long int          longlong;
typedef unsigned long long int ulonglong;
#endif

#ifndef LONGLONG_MAX
#  define LONGLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef LONGLONG_MIN
#  define LONGLONG_MIN 0x8000000000000000LL
#endif

#ifndef ULONGLONG_MAX
#  define ULONGLONG_MAX 0xfFFFFFFFFFFFFFFFLL
#endif
#ifndef ULONGLONG_MIN
#  define ULONGLONG_MIN 0x0000000000000000LL
#endif

// ---------------------------------- Symbol visibility macros (begin)
#if defined( G21_HIDE_SYMBOLS )
// These macros will allow selection on exported symbols
// taken from http://gcc.gnu.org/wiki/Visibility
#  if __GNUC__ >= 4 && !defined( __CINT__ )
#    define GAUDI_HASCLASSVISIBILITY
#  endif

#  ifdef _WIN32
/*
#  define GAUDI_IMPORT __declspec(dllimport)
#  define GAUDI_EXPORT __declspec(dllexport)
#  define GAUDI_LOCAL
*/
// The symbol visibility is disabled on Win32 because it is not possible to
// make coexists the gcc and VC ways.
#    define GAUDI_IMPORT
#    define GAUDI_EXPORT
#    define GAUDI_LOCAL
#  else
#    if defined( GAUDI_HASCLASSVISIBILITY )
#      define GAUDI_IMPORT __attribute__( ( visibility( "default" ) ) )
#      define GAUDI_EXPORT __attribute__( ( visibility( "default" ) ) )
#      define GAUDI_LOCAL __attribute__( ( visibility( "hidden" ) ) )
#    else
#      define GAUDI_IMPORT
#      define GAUDI_EXPORT
#      define GAUDI_LOCAL
#    endif
#  endif

// Define GAUDI_API for DLL builds
#  ifdef GAUDI_LINKER_LIBRARY
#    define GAUDI_API GAUDI_EXPORT
#  else
#    define GAUDI_API GAUDI_IMPORT
#  endif
#else
// Dummy definitions for the backward compatibility mode.
#  define GAUDI_API
#  define GAUDI_IMPORT
#  define GAUDI_EXPORT
#  define GAUDI_LOCAL
#endif
// ---------------------------------- Symbol visibility macros (end)

// -----------------------------------------------------------------------------
// Sanitizer suppressions
// Gcc
#if defined( __GNUC__ )
#  if defined( __SANITIZE_ADDRESS__ )
#    define GAUDI_NO_SANITIZE_ADDRESS __attribute__( ( no_sanitize_address ) )
#  endif
// Note there is no __SANITIZE_MEMORY__ to test for
#  define GAUDI_NO_SANITIZE_MEMORY __attribute__( ( no_sanitize_memory ) )
// Note there is no __SANITIZE_UNDEFINED__ to test for
#  define GAUDI_NO_SANITIZE_UNDEFINED __attribute__( ( no_sanitize_undefined ) )
// Note there is no __SANITIZE_THREAD__ to test for
#  define GAUDI_NO_SANITIZE_THREAD __attribute__( ( no_sanitize_thread ) )
#endif
// clang
#if defined( __clang__ )
#  if __has_feature( address_sanitizer )
#    define GAUDI_NO_SANITIZE_ADDRESS __attribute__( ( no_sanitize( "address" ) ) )
#  endif
#  if __has_feature( memory_sanitizer )
#    define GAUDI_NO_SANITIZE_MEMORY __attribute__( ( no_sanitize( "memory" ) ) )
#  endif
#  if __has_feature( undefined_sanitizer )
#    define GAUDI_NO_SANITIZE_UNDEFINED __attribute__( ( no_sanitize( "undefined" ) ) )
#  endif
#  if __has_feature( thread_sanitizer )
#    define GAUDI_NO_SANITIZE_THREAD __attribute__( ( no_sanitize( "thread" ) ) )
#  endif
#endif
// defaults
#ifndef GAUDI_NO_SANITIZE_ADDRESS
#  define GAUDI_NO_SANITIZE_ADDRESS
#endif
#ifndef GAUDI_NO_SANITIZE_MEMORY
#  define GAUDI_NO_SANITIZE_MEMORY
#endif
#ifndef GAUDI_NO_SANITIZE_UNDEFINED
#  define GAUDI_NO_SANITIZE_UNDEFINED
#endif
#ifndef GAUDI_NO_SANITIZE_THREAD
#  define GAUDI_NO_SANITIZE_THREAD
#endif
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Adds compiler specific hints for loop unrolling.
// To use place the macro directly before the loop you wish to unroll. e.g.
//
// GAUDI_LOOP_UNROLL(N)
// for ( std::size_t i = 0; i < N; ++i ) {
// // do stuff
// }
//
// Constraints on N are it needs to be something known at compile time.
// Gains are most obvious with small fixed size (compile time) loops,
// but in principle can be used with any loop.
#define GAUDI_DO_PRAGMA( x ) _Pragma( #x )
#if defined( __clang__ )
#  define GAUDI_LOOP_UNROLL( x ) GAUDI_DO_PRAGMA( clang loop unroll_count( x ) )
#elif defined( __GNUC__ )
#  define GAUDI_LOOP_UNROLL( x ) GAUDI_DO_PRAGMA( GCC unroll x )
#else
#  define GAUDI_LOOP_UNROLL( x )
#endif
// -----------------------------------------------------------------------------

#endif // GAUDIKERNEL_KERNEL_H
