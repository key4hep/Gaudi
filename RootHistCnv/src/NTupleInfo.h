#ifdef __cplusplus

#ifndef HBOOKCNV_NTUPLEINFO_H
#define HBOOKCNV_NTUPLEINFO_H 1

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/NTuple.h"

class NTUPLEINFO   {
public:
  long    id;
  char    title[128];
  char    rzdir[256];
  long    numEnt;
  long    numVar;
  char    block[32];

  //  struct _var   {
    char     name[512][64];
    long     dim[512];
    long     theDim[512][5];
    long     hasIndex[512];
    char     index[512][64];
    char     type[512];
    long     size[512];
    long     hasRange[512];
    long     irange[512][2];
    double   frange[512][2];
    template <class TYPE>
    void getBounds(long i, TYPE& min, TYPE& max, TYPE& null)  const  {
      if ( hasRange )   {
        min = (type[i]=='R') ? TYPE(frange[i][0]) : TYPE(irange[i][0]);
        max = (type[i]=='R') ? TYPE(frange[i][1]) : TYPE(irange[i][1]);
      }
      else  {
        min = NTuple::Range<TYPE>::min();
        max = NTuple::Range<TYPE>::max();
      }
      null = TYPE(0);
    }
    TEMPLATE_SPECIALIZATION
    void getBounds(long /*i*/, bool& min, bool& max, bool& null )  const  {
      min = null = false;
      max = true;
    }
    template <class TYPE>
    bool isRangeWithin(long i, TYPE min, TYPE max)  const  {
      if ( hasRange[i] )   {
        return min <= (TYPE)irange[i][0] && (TYPE)irange[i][1] <= max;
      }
      else if ( type[i] == 'I' || type[i] == 'U' )   {
        return NTuple::Range<TYPE>::min() <= min && NTuple::Range<TYPE>::max() >= max;
      }
      return false;
    }
    TEMPLATE_SPECIALIZATION
      bool isRangeWithin(long i, float /* min */, float /* max*/ )  const  {
      if ( size[i] == 32 ) return true;
      return false;
    }
    TEMPLATE_SPECIALIZATION
      bool isRangeWithin(long i, double /* min */, double /* max */ )  const  {
      if ( size[i] == 64 ) return true;
      return false;
    }
   TEMPLATE_SPECIALIZATION
    bool isRangeWithin(long /* i */ , bool min, bool max)  const  {
      return min == false && max == true;
    }
    long idist(long i)   const   {
        return irange[i][1] - irange[i][0];
    }
    double fdist(long i)  const    {
        return frange[i][1] - frange[i][0];
    }
  //  } var[512];

  long getIndex(const char* nam)  const  {
    if ( nam[0] != 0 )   {
      for ( int i = 0; i < numVar; i++ )    {
        if ( strncmp(name[i], nam, sizeof(name[i])) == 0 )   {
          return i;
        }
      }
    }
    return -1;
  }
};
#endif // HBOOKCNV_NTUPLEINFO_H

#else
#ifdef _WIN32
        STRUCTURE /NTUPLEVAR/
          CHARACTER*64 NAME
          INTEGER      NDIM
           INTEGER      THEDIM(5)
          INTEGER      HASINDEX
           CHARACTER*64 INDEX
          CHARACTER*1  TYPE
           INTEGER      SIZE
          INTEGER      HASRANGE
          UNION
            MAP
              INTEGER IMIN, IMAX
            END MAP
            MAP
              REAL*8  FMIN, FMAX
            END MAP
          END UNION
        END STRUCTURE

        STRUCTURE /NTUPLEINFO/
          INTEGER       ID
          CHARACTER*128 TITLE
          INTEGER       NOENT
          INTEGER       NDIM
           CHARACTER*8   BLOCK 
           RECORD /NTUPLEVAR/ VAR(512)
        END STRUCTURE
#else
c        TYPE NTUPLEVAR
          CHARACTER*64 VNAME(512)
          INTEGER      VNDIM(512)
          INTEGER      VTHEDIM(512,5)
          INTEGER      VHASINDEX(512)
          CHARACTER*64 VINDEX(512)
          CHARACTER*1  VTYPE(512)
          INTEGER      VSIZE(512)
          INTEGER      VHASRANGE(512)
c          UNION
c            MAP
  INTEGER VIMIN(512), VIMAX(512)
c            END MAP
c            MAP
  REAL*8  VFMIN(512), VFMAX(512)
c            END MAP
c          END UNION
c        END TYPE NTUPLEVAR

c        TYPE NTUPLEINFO
          INTEGER       NTID
          CHARACTER*128 NTTITLE
          INTEGER       NTNOENT
          INTEGER       NTNDIM
          CHARACTER*8   NTBLOCK 
c          TYPE(NTUPLEVAR) VAR(512)
c        END TYPE NTUPLEINFO
       COMMON/NTUPLEINFO/VNAME,VNDIM,VTHEDIM,
     &  VHASINDEX,VINDEX,VTYPE,VSIZE,
     &  VHASRANGE, VIMIN, VIMAX, VFMIN, VFMAX,
     &  NTID, NTTITLE, NTNDIM, NTBLOCK
#endif
#endif
