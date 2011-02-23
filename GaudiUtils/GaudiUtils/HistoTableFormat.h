// $Id: HistoTableFormat.h,v 1.3 2007/11/16 15:29:02 hmd Exp $
// ============================================================================
#ifndef GAUDIUTILS_HISTOTABLEFORMAT_H
#define GAUDIUTILS_HISTOTABLEFORMAT_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// forward declarations
// ============================================================================
namespace AIDA
{
  class IHistogram1D   ;
  class IBaseHistogram ;
}
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    // ========================================================================
    /** @namespace Gaudi::Utils::Histos
     *  Collection of useful utilities for manipulations with
     *  AIDA hisgograms
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date   2007-08-08
     */
    namespace Histos
    {
      // ======================================================================
      /** @namespace Gaudi::Utils::Histos::Formats
       *
       *  The 15 format fields are predefined now:
       *
       *   -  1 : the histogram path in transient store  (without '/stat/')
       *   -  2 : the histogram title
       *   -  3 : number of ALL entries
       *   -  4 : number of UNDERFLOW entries
       *   -  5 : number of OVERFLOW  entries
       *   -  6 : the statistics of equivalent entries
       *   -  7 : the full integral in range
       *   -  8 : mean value
       *   -  9 : the estimated uncertainty in the mean value
       *   - 10 : rms value
       *   - 11 : the estimated uncertainty in the rms value
       *   - 12 : skewness value
       *   - 13 : the estimated uncertainty in the skewness value
       *   - 14 : kurtosis value
       *   - 15 : the estimated uncertainty in the kurtosis value
       *   - 16 : full intergal (in and out-range) 
       *   - 17 : error on the full intergal (16) 
       *   - 18 : error on the full intergal in range (7) 
       *   - 19 : fraction of overflow entries       [%]
       *   - 20 : (binomial) error on fraction of overflow entries [%]
       *   - 21 : fraction of underflow entries      [%] 
       *   - 22 : (binomial) error on fraction of underflow entries [%]
       *   - 23 : fraction of overflow integral    
       *   - 24 : error on fraction of overflow integral 
       *   - 25 : fraction of underflow  integral 
       *   - 26 : error on fraction of underflow integral 
       *
       *  @see Gauid::Utils::HistoStats
       *
       *  Unlike parameters 3-7 & 16-19, the parameters 
       *  8-15 and 19-26 are sensitive only to 
       *  the shape of the distribution.
       *
       *  Simple collection of predefiend formats for the histogram
       *  @author Vanya BELYAEV ibelyaev@physics.syed.edu
       *  @date 2007-08-07
       */
      namespace Formats
      {
        // ====================================================================
        /** the ID for predefined formats
         *  - Default   : the default format 
         *  - Old       : the old-style format (title,integra,entries,mean,rms)
         *  - Full      : FULL information (all 15 predefiend values) 
         *  - FullStat  : FULL statistical information (all 13 statistical values) 
         *  - Stat      : title, #entries,mean, rms, skewness&kurtosis 
         *  - StatOnly  : #entries,mean, rms, skewness&kurtosis 
         *  - Stat1     : #entries,mean+-, rms+-, skewness+-&kurtosis+- 
         *  - ShapeOnly : mean,rms,skewness,kurtosis,underflow&overflow fractions
         *  - Shape     : as ShapeOnly but with errors 
         *  - LineTitle : line-format title, mean,rms,skewness&kurtosis 
         *  - LineOnly  : line-format mean,rms,skewness&kurtosis 
         *  - PathTitle : path & title 
         */
        enum Format
          {
            Default = 0 , // default format
            Old         , // "OLD"-line format: title,integral,#ent,mean,rms
            Full        , // FULL format: the first 15 positions
            FullStat    , // Full statistical info (3-15)
            Stat        , // title,#ent,mean,rms,skewness&kurtosis
            StatOnly    , // #entries,mean,rms,skewness&kurtosis
            Stat1       , // #entries,mean+-,rms+-,skewness+-&kurtosis+-
            ShapeOnly   , // mean,rms,skewness,kurtosis,underflow&overflow fractions
            Shape       , // as ShapeOnly but with errors 
            LineTitle   , // line-format : title,mean,rms,skewness&kurtosis
            LineOnly    , // line-format : mean,rms,skewness&kurtosis
            PathTitle     // "PathTitle" : path & title
          } ;
        // ====================================================================
        /** get the format by enum
         *  @see Gaudi::Utils::Histos::Formats::Format
         *  @param ID the format idenfifier
         *  @return format associated with ID
         *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
         *  @date   2007-08-08
         */
        GAUDI_API std::string format ( const int ID = Default ) ;
        // ====================================================================
        /** get the recommended header by enum
         *  @see Gaudi::Utils::Histos::Formats::Format
         *  @param ID the format idenfifier
         *  @return header associated with ID
         *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
         *  @date   2007-08-08
         */
        GAUDI_API std::string header ( const int ID = Default ) ;
        // ====================================================================
      } // end of namespace Gaudi::Utils::Histos::Formats
      // ======================================================================
      /** get the path in THS for AIDA histogram
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date   2007-08-08
       */
      GAUDI_API std::string path ( const AIDA::IBaseHistogram* aida ) ;
      // ======================================================================
      /** Make the string representation of the historgam
       *  according to the specified format.
       *  The method could be used to access/print various quantities
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  const AIDA::IHistogram1D* histo = ... ;
       *
       *  // print title in a free format:
       *  std::cout << format   ( histo , "%2%" ) << std::endl ;
       *
       *  // print the path in HTS in a free format:
       *  std::cout << format   ( histo , " path in HTS: %1% " ) << std::endl ;
       *
       *  // print the formatted nEntries/Overflow/Underflow:
       *  std::cout <<
       *     format   ( histo , " #nEntries/Overflow/Underflow=%3%/%4%/%5%" )
       *            << std::endl ;
       *
       *  // print the formatted Mean+-ErrorMean:
       *  std::cout <<
       *     format   ( histo , " Mean+-Error=(%8%+-%9%)" )
       *            << std::endl ;
       *
       *  // print the skewness and kurtosis:
       *  std::cout <<
       *     format   ( histo , " Skewness/Kurtosis=%12%/%14% " )
       *            << std::endl ;
       *
       *  @endcode
       *
       *  @param histo reference to the histogram
       *  @param fmt   the printout format
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date   2007-08-08
       */
      GAUDI_API std::string format
      ( const AIDA::IHistogram1D* histo  ,
        const std::string&        fmt    ) ;
      // ======================================================================
      /** format a full row in table, including ID, label, path or any other
       *  "extra" identifier in string form
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos
       *  const AIDA::IHistogram1D* histo = ... ;
       *
       *  // define short format
       *  const std::string fmt1 = " |%1$-30.30s %|33t| %2" ;
       *
       *  // define format for the histogram
       *  const std::string fmt2 = ... ;
       *
       *  info () <<
       *        format   ( "My Histo" , histo , fmt1 , fmt2 )
       *          << endmsg ;
       *
       *  @endcode
       *
       *  @param histo pointer to the histogram
       *  @param ID historgam ID, title, label  or other extra information
       *  @param fmt1 "short" format used for the table
       *  @param fmt2 format used for the histogram printout
       *  @return formatted row
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date   2007-08-08
       */
      GAUDI_API std::string format
      ( const AIDA::IHistogram1D* histo  ,
        const std::string&        ID     ,
        const std::string&        fmt1   ,
        const std::string&        fmt2   ) ;
      // ========================================================================
      /** print the simple sequence (list-like) of histograms as table
       *
       *  @code
       *
       *  SEQUENCE histos = ... ;
       *
       *  // print a table with three colums  path, title and #entries
       *
       *  Gaudi::Utils::Histos::printList
       *    ( histos.begin () ,
       *      histos.end   () ,
       *      " | %1$|-40.40s | %2$-30.30s | %3$=7d | " ,
       *      std::cout       ,
       *      '\n'            ) ;
       *
       *  @endcode
       *
       *  @param first begin-iterator for the sequence
       *  @param last  end-iterator for the sequence
       *  @param stream the stream to be used for printout
       *  @param term   the terminmator for the stream
       *  @param fmt    the format to be used
       */
      template <class HISTO, class STREAM, class TERMINATOR>
      inline STREAM& printList
      ( HISTO              first  ,
        HISTO              last   ,
        const std::string& fmt    ,
        STREAM&            stream ,
        TERMINATOR         term   )
      {
        for ( ; first != last ; ++first )
        { stream << format   ( *first , fmt ) << term ; }  // print table rows
        return stream ;                                    // RETURN
      }
      // ======================================================================
      /** print the simple container of histograms as table
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  SEQUENCE histos = ... ;
       *
       *  // print a table with three columns:  path, title and #entries
       *
       *  printList
       *    ( histos          ,
       *      " | %1$|-40.40s | %2$-30.30s | %3$=7d | " ,
       *      std::cout       ,
       *      '\n'            ) ;
       *
       *  @endcode
       *
       *  @param histos the sequence of histograms
       *  @param stream the stream to be used for printout
       *  @param term   the terminmator for the stream
       *  @param fmt    the format to be used
       */
      template <class LIST, class STREAM, class TERMINATOR>
      inline STREAM& printList
      ( const LIST&        histos ,
        const std::string& fmt    ,
        STREAM&            stream ,
        TERMINATOR         term   )
      {
        return printList
          ( histos.begin () , histos.end  () , fmt , stream , term ) ;
      }
      // ======================================================================
      /** Print the "associative sequence"  (e.g. part of std:map) of
       *  histograms as table:
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  const std::map<std::string,AIDA::IHistogram1D*>& m = ... ;
       *
       *  printMap
       *   ( m.begin () ,
       *     m.end   () ,
       *     "| %1$-10.10s | %2% " , // short format
       *     Gaudi::Utils::Histos::Formats::histoFormatOnly ,
       *     always()   ,
       *     endmsg     ) ;
       *
       *  @endcode
       *
       *  Print only mean and rms:
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  const std::map<GaudiAlg::ID,AIDA::IHistogram1D*>& m = ... ;
       *
       *  printMap
       *    ( m.begin () ,
       *      m.end   () ,
       *      "| %1$-10.10s | %2% "    , // short format
       *      " %8$10.5g+-%10$-10.5g|", // mean+-rms
       *      always()   ,
       *      endmsg     ) ;
       *
       *  @endcode
       *
       *  @param begin 'begin'-iterator for the mapping sequence
       *  @param end   'end'-iterator for the mapping sequence
       *  @param fmt1  'short' format for the table printout
       *  @param fmt3   format for the printout of the histogram
       *  @param stream the stream for printout
       *  @param term   stream terminator
       */
      template <class HISTO, class STREAM, class TERMINATOR>
      inline STREAM& printMap
      ( HISTO              begin  ,
        HISTO              end    ,
        const std::string& fmt1   ,
        const std::string& fmt2   ,
        STREAM&            stream ,
        TERMINATOR         term   )
      {
        for ( ; begin != end ; ++begin )
        {
          stream << format
            ( begin -> second , // the histogram
              begin -> first  , // the key
              fmt1  ,  fmt2   ) << term ;
        }
        return stream ;
      }
      // ======================================================================
      /** Print the "associative sequence"  (e.g. part of std:map) of
       *  histograms as table:
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  const std::map<std::string,AIDA::IHistogram1D*>& m = ... ;
       *
       *  printMap
       *   ( m          ,
       *     "| %1$-10.10s | %2% " , // short format
       *     Gaudi::Utils::Histos::Formats::histoFormatOnly ,
       *     always()   ,
       *     endmsg     ) ;
       *
       *  @endcode
       *
       *  Print only mean and rms:
       *
       *  @code
       *
       *  using namespace Gaudi::Utils::Histos ;
       *
       *  const std::map<GaudiAlg::ID,AIDA::IHistogram1D*>& m = ... ;
       *
       *  printMap
       *    ( m          ,
       *      "| %1$-10.10s | %2% "    , // short format
       *      " %8$10.5g+-%10$-10.5g|", // mean+-rms
       *      always()   ,
       *      endmsg     ) ;
       *
       *  @endcode
       *
       *  @param begin 'begin'-iterator for the mapping sequence
       *  @param end   'end'-iterator for the mapping sequence
       *  @param fmt1  'short' format for the table printout
       *  @param fmt3   format for the printout of the histogram
       *  @param stream the stream for printout
       *  @param term   stream terminator
       */
      template <class MAP, class STREAM, class TERMINATOR>
      inline STREAM& printMap
      ( const MAP&         histos ,
        const std::string& fmt1   ,
        const std::string& fmt2   ,
        STREAM&            stream ,
        TERMINATOR         term   )
      {
        return printMap
          ( histos.begin () , histos.end() , fmt1 , fmt2 , stream , term ) ;
      }
      // ======================================================================
      /** helper method to merge the headers for short format table
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date   2007-08-07
       */
      GAUDI_API std::string format
      ( const std::string& val1 ,
        const std::string& val2 ,
        const std::string& fmt  ) ;
      // ======================================================================
      /** @class Table HistoTableFormat.h GaudiUtils/HistoTableFormat.h
       *  Simple class for the customizeble printout of the histogram tables
       *
       *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
       *  @date   2007-08-07
       */
      // ========================================================================
      class GAUDI_API Table
      {
      public:
        /// constructor from enum
        Table  ( const int ID  = 0 ) ;
        /// Constructor from the format, header and footer
        Table
        ( const std::string& format      ,
          const std::string& header = "" ,
          const std::string& footer = "" ) ;
      public:
        // ======================================================================
        /** print the simple sequence of histograms as table
         *  @param first begin-iterator for the sequence
         *  @param last  end-iterator for the sequence
         *  @param stream the stream to be used for printout
         *  @param term   the terminmator for the stream
         */
        template <class HISTO, class STREAM, class TERMINATOR>
        STREAM& printList
        ( HISTO              first  ,
          HISTO              last   ,
          STREAM&            stream ,
          TERMINATOR         term   ) const
        {
          if ( !header().empty() ) { stream << header () << term ; }
          Gaudi::Utils::Histos::printList ( first , last , format() , stream , term ) ;
          if ( !footer().empty() ) { stream << footer () << term ; }
          return stream ;
        }
        // ======================================================================
      public:
        /** print as table the content of the accociative map
         *  (key->histogram) with the key, convertible to std::string
         *
         *  The default format is used for histogram prinout
         *
         *  @param first 'begin'-iterator for the mapping sequence
         *  @param last  'end'-iterator for the mapping sequence
         *  @param fmt   'short' format for the table printout
         *  @param hval  the name of the "key"-column
         *  @param stream the stream for printout
         *  @param term   stream terminator
         */
        template <class HISTO, class STREAM, class TERMINATOR>
        STREAM& printMap
        ( HISTO              first  ,
          HISTO              last   ,
          const std::string& fmt    ,
          const std::string& hval   ,
          STREAM&            stream ,
          TERMINATOR         term   ) const
        {
          if ( !hval.empty() || !header().empty() )
          { stream << Gaudi::Utils::Histos::format ( hval , header() , fmt ) << term ; }
          return Gaudi::Utils::Histos::printMap
            ( first , last , fmt , format() , stream , term ) ;
        }
        // ======================================================================
      public:
        // ======================================================================
        /// the table header
        const std::string& header () const { return m_header ; }
        // ======================================================================
        /// the table footer
        const std::string& footer () const { return m_footer ; }
        // ======================================================================
        /// the format
        const std::string& format () const { return m_format ; }
        // ======================================================================
      public:
        // ======================================================================
        /// set new header
        void setHeader ( const std::string& v ) { m_header = v ; }
        // ======================================================================
        /// set new footer
        void setFooter ( const std::string& v ) { m_footer = v ; }
        // ======================================================================
        /// set new format
        void setFormat ( const std::string& v ) { m_format = v ; }
        // ======================================================================
      public :
        // ======================================================================
        /// make the string representation according to the default format
        std::string toString ( const AIDA::IHistogram1D* histo  ) const ;
        // ======================================================================
      public:
        // ======================================================================
        /** format a full row in table, including ID, label, path or any other
         *  "extra" identifier
         *
         *  @code
         *
         *  using namespace Gaudi::Utils::Histos ;
         *
         *  const Table& table = ... ;
         *
         *  const AIDA::IHistogram1D* histo = ... ;
         *
         *  // define short format
         *  const std::string fmt1 = " |%1$-30.30s %|33t| %2" ;
         *
         *  info () << table.toString ( "My Histo" , histo , fmt ) ;
         *
         *  @endcode
         *
         *  @param histo pointer to the histogram
         *  @param ID historgam ID, title, label  or other extra infomration
         *  @param fmt "short" format used for the table
         *  @return formatted row
         */
        std::string toString
        ( const AIDA::IHistogram1D* histo  ,
          const std::string&        ID     ,
          const std::string&        fmt    ) const ;
        // ======================================================================
      public:
        // the table header
        std::string m_header ; ///< the table header
        // the table footer
        std::string m_footer ; ///< the table footer
        // the default format
        std::string m_format ; ///< the defautl format
        //
      } ;
    } // end of namespace Gaudi::Utils::Histos
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIUTILS_HISTOTABLEFORMAT_H
// ============================================================================
